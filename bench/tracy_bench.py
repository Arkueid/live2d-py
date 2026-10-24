"""Tracy 压测：自动跑「模型 × 后端」，抓 trace 并生成 CSV + 汇总报告。

用法:
    python bench/tracy_bench.py
    python bench/tracy_bench.py --name baseline --models llny,Haru --backends gl,vk
    python bench/tracy_bench.py --models v2/kasumi2/kasumi2.model.json --backends gl
    python bench/tracy_bench.py --models Mao --duration 8 --warmup 5
    python bench/tracy_bench.py --name all_models --report-only   # 只重建报告

产物（默认 bench/traces/<name>/）:
    <backend>_<model>.tracy     原始 trace，可用 Tracy GUI 打开
    <backend>_<model>.csv       tracy-csvexport 的 zone 统计
    summary.csv                 汇总表
    summary.md                  汇总报告（含说明）
    report.html                 可视化报告（自动分析 + 表格）
    run.log                     每个用例的启动/抓取日志

注意:
    - 需要启用 Tracy 的构建（cmake/Config.local.cmake 里 LIVE2D_TRACY on）。
    - GL 与 VK 应跑在同一张显卡上；GPU 归属由程序日志的 `Device:` 行记录。
"""

import argparse
import csv
import os
import re
import shutil
import subprocess
import sys
import time
from datetime import datetime
from pathlib import Path

ROOT = Path(__file__).resolve().parent.parent
RESOURCES = ROOT / "Resources"
EXAMPLES = {
    "gl": ROOT / "bench" / "glfw" / "bench_gl.py",
    "vk": ROOT / "bench" / "glfw" / "bench_vk.py",
}
PYTHON = ROOT / ".venv" / "Scripts" / "python.exe"

# 汇总报告中关注的 zone
ZONES = [
    "py/Draw",
    "py/Update",
    "py/vkBeginFrame",
    "py/vkEndFrame",
    "v3/ModelUpdate",
    "v2/ModelUpdate",
    "v3/DrawModel",
    "v2/DrawModel",
    "vk/RecordObjects",
    "vk/ClipSetupDrawable",
    "vk/Sort",
    "vk/acquire",
    "vk/waitIdle",
    "vk/present",
    "vk/submit",
]


def find_tracy_tools():
    """定位 tracy-capture / tracy-csvexport（tools/windows-*）。"""
    for d in sorted((ROOT / "tools").glob("windows-*")):
        cap = d / "tracy-capture.exe"
        exp = d / "tracy-csvexport.exe"
        if cap.exists() and exp.exists():
            return cap, exp
    raise SystemExit("找不到 tracy-capture.exe / tracy-csvexport.exe（应在 tools/windows-*/ 下）")


def resolve_model(spec):
    """把 llny / Haru / v3/llny/llny.model3.json 解析成绝对 json 路径。"""
    p = Path(spec)
    if p.is_absolute():
        if not p.exists():
            raise SystemExit(f"模型不存在: {spec}")
        return p
    rel = RESOURCES / spec
    if rel.exists() and rel.is_file():
        return rel

    # 按目录名模糊匹配（大小写不敏感）
    name = spec.lower()
    for ver in ("v3", "v2"):
        base = RESOURCES / ver
        if not base.is_dir():
            continue
        for d in sorted(base.iterdir()):
            if d.is_dir() and name in d.name.lower():
                cands = list(d.glob("*.model3.json")) + list(d.glob("*.model.json")) + list(d.glob("model0.json"))
                if cands:
                    return sorted(cands, key=lambda f: len(f.name))[0]
    raise SystemExit(f"找不到模型: {spec}")


def model_label(path):
    """v3/llny/llny.model3.json -> llny；v2/托尔/model0.json -> 托尔"""
    return path.parent.name


def discover_models():
    """枚举 Resources 下所有含模型 json 的目录（每个目录取文件名最短的一个，避开 *_min 变体）。"""
    found = []
    for ver in ("v3", "v2"):
        base = RESOURCES / ver
        if not base.is_dir():
            continue
        for d in sorted(base.iterdir()):
            if not d.is_dir():
                continue
            cands = list(d.glob("*.model3.json")) + list(d.glob("*.model.json")) + list(d.glob("model0.json"))
            if cands:
                found.append(sorted(cands, key=lambda f: len(f.name))[0])
    return found


def slug(text):
    """保留中文，仅替换文件系统非法字符。"""
    return re.sub(r'[\\/:*?"<>|\s]+', "_", text)


def version_of(path):
    return 2 if "v2" in path.parts else 3


def parse_zones(csv_path):
    """读取 csvexport 结果，返回 {zone: {mean_ms, max_ms, counts, perc}} 与 trace 时长。"""
    stats = {}
    duration_s = None
    with open(csv_path, encoding="utf-8") as f:
        for row in csv.DictReader(f):
            name = (row.get("name") or "").strip()
            if not name:
                continue
            try:
                total_ns = float(row["total_ns"])
                perc = float(row["total_perc"] or 0)
            except (KeyError, ValueError):
                continue
            stats[name] = {
                "mean_ms": float(row["mean_ns"]) / 1e6,
                "max_ms": float(row["max_ns"]) / 1e6,
                "counts": int(float(row["counts"])),
            }
            if duration_s is None and perc > 0:
                duration_s = total_ns / (perc / 100.0) / 1e9
    return stats, duration_s


def parse_device(log_path, retries=5):
    """从应用日志里取 `[pyi][I] Device: ...` 行（GL/VK 初始化时都会打印）。"""
    for _ in range(retries):
        try:
            text = log_path.read_text(encoding="utf-8", errors="replace")
        except OSError:
            text = ""
        for line in reversed(text.splitlines()):
            if "Device:" in line:
                return line.split("Device:", 1)[1].strip()
        time.sleep(0.2)
    return None


def normalize_gpu(name):
    """统一 GL 的 `.../PCIe/SSE2` 后缀，便于判定是否同一张卡。"""
    return name.split("/")[0].strip() if name else name


def kill_tree(pid):
    if pid is None:
        return
    subprocess.run(
        ["taskkill", "/PID", str(pid), "/T", "/F"],
        stdout=subprocess.DEVNULL,
        stderr=subprocess.DEVNULL,
    )


def run_case(backend, model_path, out_dir, duration, warmup, size, capture, csvexport, log):
    """跑单个用例，返回汇总字典。"""
    label = model_label(model_path)
    version = version_of(model_path)
    stem = f"{backend}_v{version}_{slug(label)}"
    tracy_path = out_dir / f"{stem}.tracy"
    csv_path = out_dir / f"{stem}.csv"
    result = {"model": label, "version": version, "backend": backend, "status": "ok"}

    env = os.environ.copy()
    env["PYTHONPATH"] = str(ROOT / "package")

    log.write(f"\n=== {stem} ===\n")
    app_log = out_dir / f"{stem}.log"
    with open(app_log, "w", encoding="utf-8", errors="replace") as fh:
        proc = subprocess.Popen(
            [str(PYTHON), str(EXAMPLES[backend]),
             "--model", str(model_path), "--size", size],
            cwd=str(ROOT),
            env=env,
            stdout=fh,
            stderr=subprocess.STDOUT,
        )
    try:
        time.sleep(warmup)
        if proc.poll() is not None:
            result["status"] = f"启动失败(exit={proc.returncode})"
            log.write(result["status"] + "\n")
            return result

        # GPU 归属：直接取应用日志里的 Device 行（GL: glGetString(GL_RENDERER)，VK: VkPhysicalDeviceProperties）
        result["gpu"] = normalize_gpu(parse_device(app_log))

        t0 = time.monotonic()
        cap = subprocess.run(
            [str(capture), "-f", "-s", str(duration), "-o", str(tracy_path)],
            capture_output=True,
            text=True,
            encoding="utf-8",
            errors="replace",
            timeout=duration + 60,
        )
        elapsed = time.monotonic() - t0
        log.write((cap.stdout or "") + (cap.stderr or "") + "\n")
        if cap.returncode != 0 or not tracy_path.exists():
            result["status"] = f"抓取失败(exit={cap.returncode})"
            return result
    except subprocess.TimeoutExpired:
        result["status"] = "抓取超时（客户端未连接 8086）"
        log.write(result["status"] + "\n")
        return result
    finally:
        kill_tree(proc.pid)
        time.sleep(1.0)

    exp = subprocess.run(
        [str(csvexport), str(tracy_path)],
        capture_output=True,
        text=True,
        encoding="utf-8",
        errors="replace",
    )
    if exp.returncode != 0 or not exp.stdout.strip():
        result["status"] = f"导出失败(exit={exp.returncode})"
        return result
    csv_path.write_text(exp.stdout, encoding="utf-8")

    stats, duration_s = parse_zones(csv_path)
    frames = stats.get("py/Draw", {}).get("counts", 0)
    if frames == 0 or not duration_s:
        result["status"] = "无帧数据"
        return result

    result.update(
        frames=frames,
        trace_s=elapsed,
        fps=frames / elapsed,
        draw_ms=stats.get("py/Draw", {}).get("mean_ms"),
        draw_max_ms=stats.get("py/Draw", {}).get("max_ms"),
        update_ms=(stats.get("v3/ModelUpdate") or stats.get("v2/ModelUpdate") or {}).get("mean_ms"),
        draw_model_ms=(stats.get("v3/DrawModel") or stats.get("v2/DrawModel") or {}).get("mean_ms"),
        record_ms=stats.get("vk/RecordObjects", {}).get("mean_ms"),
        acquire_ms=stats.get("vk/acquire", {}).get("mean_ms"),
        wait_idle_ms=stats.get("vk/waitIdle", {}).get("mean_ms"),
        present_ms=stats.get("vk/present", {}).get("mean_ms"),
        submit_ms=stats.get("vk/submit", {}).get("mean_ms"),
    )

    # 帧时间：VK 由 Python 驱动三段时间；GL 只有 Draw + Update（无 FrameMark）
    begin = stats.get("py/vkBeginFrame", {}).get("mean_ms", 0.0) or 0.0
    end = stats.get("py/vkEndFrame", {}).get("mean_ms", 0.0) or 0.0
    upd = stats.get("py/Update", {}).get("mean_ms", 0.0) or 0.0
    result["frame_ms"] = result["draw_ms"] + (begin + end if backend == "vk" else upd)

    if backend == "vk":
        result["gpu_stall_ms"] = (result["acquire_ms"] or 0.0) + (result["wait_idle_ms"] or 0.0)
    else:
        result["gpu_stall_ms"] = None

    log.write(f"frames={frames} fps={result['fps']:.1f} frame={result['frame_ms']:.3f}ms\n")
    return result


def fmt(v, digits=3):
    return "—" if v is None else f"{v:.{digits}f}"


def build_findings(results):
    """从结果里提炼几条可读的结论。"""
    ok = [r for r in results if r.get("status") == "ok"]
    out = []
    if not ok:
        return [("没有可用数据", "所有用例都失败，请查看 run.log。", "warn")]

    gpus = sorted({normalize_gpu(r.get("gpu")) for r in ok if r.get("gpu")})
    if len(gpus) > 1:
        detail = "、".join(f"{r['backend']}/{r['model']}={r['gpu']}" for r in ok if r.get("gpu"))
        out.append(("GPU 不一致",
                    f"用例分布在 {', '.join(gpus)} 上，跨后端对比不成立。明细：{detail}", "warn"))
    elif gpus:
        out.append(("GPU 一致",
                    f"全部 {len(ok)} 个用例都运行在 {gpus[0]} 上（取自日志 Device 行）。", "ok"))

    for backend, title in (("vk", "Vulkan"), ("gl", "OpenGL")):
        rows = [r for r in ok if r["backend"] == backend]
        if not rows:
            continue
        frames = [r["frame_ms"] for r in rows]
        draws = [r["draw_ms"] for r in rows]
        if backend == "vk":
            stalls = [r["gpu_stall_ms"] or 0.0 for r in rows]
            fps = [r["fps"] for r in rows]
            spread = (max(fps) - min(fps)) / max(fps) * 100 if max(fps) else 0
            stall_avg = sum(stalls) / len(stalls)
            frame_avg = sum(frames) / len(frames)
            draw_ratio = max(draws) / max(min(draws), 1e-6)
            if spread < 5 and draw_ratio > 2:
                out.append((
                    "Vulkan 被固定开销钳制",
                    f"{len(rows)} 个模型 FPS 只差 {spread:.1f}%（{min(fps):.0f}–{max(fps):.0f}），"
                    f"但 Draw 相差 {draw_ratio:.1f} 倍（{min(draws):.2f}–{max(draws):.2f}ms）。"
                    f"GPU 阻塞平均 {stall_avg:.2f}ms，占帧时间 {100 * stall_avg / frame_avg:.0f}%。"
                    f"瓶颈在每帧 acquire/waitIdle 的串行等待，而不是绘制本身。",
                    "warn"))
            else:
                out.append(("Vulkan 帧时间",
                            f"帧时间 {min(frames):.2f}–{max(frames):.2f}ms，"
                            f"GPU 阻塞平均 {stall_avg:.2f}ms。", "info"))
        else:
            out.append((
                "OpenGL 跟随模型负载",
                f"{len(rows)} 个模型帧时间 {min(frames):.2f}–{max(frames):.2f}ms"
                f"（{max(frames) / max(min(frames), 1e-6):.1f} 倍差），"
                f"Draw {min(draws):.2f}–{max(draws):.2f}ms。GL 无显式等待，帧率直接反映 CPU 提交成本。",
                "info"))

    pairs = []
    for vk in [r for r in ok if r["backend"] == "vk"]:
        gl = next((g for g in ok if g["backend"] == "gl"
                   and g["model"] == vk["model"] and g["version"] == vk["version"]), None)
        if gl:
            pairs.append((vk, gl))
    if pairs:
        ratio = sum(v["frame_ms"] / g["frame_ms"] for v, g in pairs) / len(pairs)
        stall = sum((v["gpu_stall_ms"] or 0) for v, _ in pairs) / len(pairs)
        out.append(("GL vs VK",
                    f"{len(pairs)} 个同模型配对中，VK 帧时间平均是 GL 的 {ratio:.1f} 倍；"
                    f"VK 平均 GPU 阻塞 {stall:.2f}ms，与该差距同量级。", "info"))

    heavy = sorted(ok, key=lambda r: -(r["draw_ms"] or 0))[:3]
    out.append(("CPU 最重的模型",
                "、".join(f"{r['model']} v{r['version']}/{r['backend']} Draw {r['draw_ms']:.2f}ms"
                          for r in heavy), "info"))
    return out


CSS = """
:root{--bg:#f4f5f7;--card:#fff;--ink:#181c22;--muted:#6b7280;--line:#e6e8ec;
--gl:#2563eb;--vk:#7c3aed;--warn:#c2410c;--ok:#047857;--info:#334155}
*{box-sizing:border-box}
body{margin:0;background:var(--bg);color:var(--ink);
font:14px/1.6 -apple-system,"Segoe UI","Microsoft YaHei",sans-serif}
.wrap{max-width:1180px;margin:0 auto;padding:32px 24px 64px}
h1{font-size:26px;margin:0 0 6px}
h2{font-size:17px;margin:36px 0 12px;padding-bottom:8px;border-bottom:1px solid var(--line)}
.meta{color:var(--muted);font-size:13px;display:flex;gap:18px;flex-wrap:wrap}
.meta b{color:var(--ink);font-weight:600}
.cards{display:grid;grid-template-columns:repeat(auto-fit,minmax(320px,1fr));gap:14px;margin-top:20px}
.finding{background:var(--card);border-radius:12px;padding:16px 18px;
border-left:4px solid var(--info);box-shadow:0 1px 2px rgba(16,24,40,.06)}
.finding.warn{border-left-color:var(--warn)}
.finding.ok{border-left-color:var(--ok)}
.finding h3{margin:0 0 6px;font-size:14px}
.finding p{margin:0;color:var(--muted);font-size:13px}
table{width:100%;border-collapse:collapse;background:var(--card);border-radius:12px;
overflow:hidden;box-shadow:0 1px 2px rgba(16,24,40,.06);font-size:13px}
th,td{padding:9px 10px;text-align:right;border-bottom:1px solid var(--line);white-space:nowrap}
th{background:#fafbfc;color:var(--muted);font-weight:600;font-size:12px}
td:first-child,th:first-child,td:nth-child(2),th:nth-child(2),
td:nth-child(3),th:nth-child(3),td:nth-child(4),th:nth-child(4){text-align:left}
tbody tr:hover{background:#fafbff}
tbody tr:last-child td{border-bottom:none}
.badge{display:inline-block;padding:1px 8px;border-radius:999px;font-size:11px;font-weight:600;color:#fff}
.badge.gl{background:var(--gl)}.badge.vk{background:var(--vk)}
.badge.ok{background:var(--ok)}.badge.bad{background:var(--warn);font-weight:500}
.badge.small{background:#94a3b8}
.bar{display:flex;height:8px;border-radius:4px;overflow:hidden;background:#eef0f4;min-width:90px}
.bar i{display:block;height:100%}
.bar .b1{background:#3b82f6}.bar .b2{background:#f59e0b}.bar .b3{background:#cbd5e1}
.legend{display:flex;gap:16px;color:var(--muted);font-size:12px;margin:10px 0 0}
.legend span::before{content:"";display:inline-block;width:10px;height:10px;border-radius:3px;
margin-right:6px;vertical-align:-1px;background:var(--c)}
.notes{background:var(--card);border-radius:12px;padding:16px 20px;color:var(--muted);font-size:13px}
.notes li{margin:4px 0}
code{background:#f1f3f6;border-radius:4px;padding:1px 5px;font-size:12px}
"""


def write_report(out_dir, results, name, args, models, backends):
    from html import escape

    cols = [
        ("model", "模型"),
        ("version", "版本"),
        ("backend", "后端"),
        ("gpu", "GPU"),
        ("status", "状态"),
        ("frames", "帧数"),
        ("fps", "FPS"),
        ("frame_ms", "帧时间(ms)"),
        ("draw_ms", "Draw(ms)"),
        ("update_ms", "ModelUpdate(ms)"),
        ("draw_model_ms", "DrawModel(ms)"),
        ("record_ms", "RecordObjects(ms)"),
        ("acquire_ms", "acquire(ms)"),
        ("wait_idle_ms", "waitIdle(ms)"),
        ("gpu_stall_ms", "GPU阻塞(ms)"),
        ("draw_max_ms", "Draw最大(ms)"),
    ]

    with open(out_dir / "summary.csv", "w", encoding="utf-8", newline="") as f:
        w = csv.writer(f)
        w.writerow([c[0] for c in cols])
        for r in results:
            w.writerow(["" if r.get(k) is None else r.get(k) for k, _ in cols])

    findings = build_findings(results)

    # ---- summary.md ----
    lines = [
        f"# Tracy 压测报告：{name}",
        "",
        f"- 生成时间：{datetime.now().strftime('%Y-%m-%d %H:%M:%S')}",
        f"- 模型：{', '.join(models)}",
        f"- 后端：{', '.join(backends)}",
        f"- 抓取时长 / 预热：{args.duration}s / {args.warmup}s",
        f"- 窗口尺寸：{args.size}",
        "",
        "## 自动分析",
        "",
    ]
    for title, body, _ in findings:
        lines.append(f"- **{title}**：{body}")
    lines += ["", "## 结果", "",
              "| " + " | ".join(c[1] for c in cols) + " |",
              "|" + "---|" * len(cols)]
    for r in results:
        cells = []
        for key, _ in cols:
            v = r.get(key)
            if key == "status":
                cells.append(str(v))
            elif isinstance(v, float):
                cells.append(fmt(v, 1 if key in ("fps", "frames") else 3))
            elif isinstance(v, int):
                cells.append(str(v))
            else:
                cells.append("—" if v is None else str(v))
        lines.append("| " + " | ".join(cells) + " |")

    lines += [
        "", "## 说明", "",
        "- **FPS** = `py/Draw` 次数 / 抓取窗口墙钟时长（`tracy-capture -s` 的实际耗时）。",
        "- **帧时间**：Vulkan = `py/vkBeginFrame + py/Draw + py/vkEndFrame`；OpenGL 无 FrameMark，取 `py/Draw + py/Update`，不含 `swap_buffers` 与 Python 循环开销。",
        "- **GPU阻塞** = `vk/acquire + vk/waitIdle`，只对 Vulkan 有意义。",
        "- **GPU** 归属取自应用启动日志的 `Device:` 行（GL 用 `glGetString(GL_RENDERER)`，VK 用 `VkPhysicalDeviceProperties`）。",
        "- **ModelUpdate** 是 `CubismModel::Update()`（顶点形变），**DrawModel** 是渲染器录制阶段。",
        "- 用例失败只看「状态」列；trace 仍保留，可用 Tracy GUI 打开排查。",
    ]
    (out_dir / "summary.md").write_text("\n".join(lines) + "\n", encoding="utf-8")

    # ---- report.html ----
    findings_html = "".join(
        f'<div class="finding {lvl}"><h3>{escape(t)}</h3><p>{escape(b)}</p></div>'
        for t, b, lvl in findings
    )

    rows_html = []
    for r in results:
        backend = r["backend"]
        status = r.get("status", "")
        status_html = ('<span class="badge ok">ok</span>' if status == "ok"
                       else f'<span class="badge bad">{escape(status)}</span>')
        gpu = r.get("gpu")
        if not gpu:
            gpu_html = "—"
        else:
            low = gpu.lower()
            vendor = ("NVIDIA" if "nvidia" in low else
                      "Intel" if "intel" in low else
                      "AMD" if "amd" in low else gpu[:14])
            gpu_html = f'<span class="badge small" title="{escape(gpu)}">{escape(vendor)}</span>'
        frame = r.get("frame_ms") or 0.0
        draw = r.get("draw_ms") or 0.0
        stall = r.get("gpu_stall_ms") or 0.0
        other = max(0.0, frame - draw - stall)
        bar = ""
        if frame > 0:
            bar = (f'<div class="bar"><i class="b1" style="width:{draw / frame * 100:.1f}%"></i>'
                   f'<i class="b2" style="width:{stall / frame * 100:.1f}%"></i>'
                   f'<i class="b3" style="width:{other / frame * 100:.1f}%"></i></div>')
        rows_html.append(
            "<tr>"
            f"<td>{escape(str(r.get('model', '')))}</td>"
            f"<td>v{r.get('version', '')}</td>"
            f"<td><span class=\"badge {backend}\">{backend}</span></td>"
            f"<td>{gpu_html}</td>"
            f"<td>{status_html}</td>"
            f"<td>{r.get('frames') or '—'}</td>"
            f"<td><b>{fmt(r.get('fps'), 1)}</b></td>"
            f"<td>{fmt(r.get('frame_ms'))}</td>"
            f"<td>{bar}</td>"
            f"<td>{fmt(r.get('update_ms'))}</td>"
            f"<td>{fmt(r.get('draw_model_ms'))}</td>"
            f"<td>{fmt(r.get('acquire_ms'))}</td>"
            f"<td>{fmt(r.get('wait_idle_ms'))}</td>"
            f"<td>{fmt(r.get('gpu_stall_ms'))}</td>"
            f"<td>{fmt(r.get('draw_max_ms'))}</td>"
            "</tr>"
        )

    # 同模型 GL/VK 对比
    pair_rows = []
    ok_rows = [r for r in results if r.get("status") == "ok"]
    models_seen = []
    for r in ok_rows:
        key = (r["model"], r["version"])
        if key in models_seen:
            continue
        models_seen.append(key)
        gl = next((g for g in ok_rows if (g["model"], g["version"]) == key and g["backend"] == "gl"), None)
        vk = next((v for v in ok_rows if (v["model"], v["version"]) == key and v["backend"] == "vk"), None)
        if not gl and not vk:
            continue
        ratio = "—"
        if gl and vk and gl.get("frame_ms"):
            ratio = f"{vk['frame_ms'] / gl['frame_ms']:.1f}×"
        pair_rows.append(
            "<tr>"
            f"<td>{escape(str(key[0]))}</td><td>v{key[1]}</td>"
            f"<td>{fmt(gl.get('fps') if gl else None, 0)}</td>"
            f"<td>{fmt(gl.get('frame_ms') if gl else None)}</td>"
            f"<td>{fmt(vk.get('fps') if vk else None, 0)}</td>"
            f"<td>{fmt(vk.get('frame_ms') if vk else None)}</td>"
            f"<td>{ratio}</td>"
            "</tr>"
        )

    ok_count = len(ok_rows)
    html_doc = f"""<!DOCTYPE html>
<html lang="zh-CN"><head><meta charset="utf-8">
<meta name="viewport" content="width=device-width,initial-scale=1">
<title>Tracy 压测报告 · {escape(name)}</title>
<style>{CSS}</style></head>
<body><div class="wrap">
<h1>Tracy 压测报告 · {escape(name)}</h1>
<div class="meta">
<span>生成时间 <b>{datetime.now().strftime('%Y-%m-%d %H:%M:%S')}</b></span>
<span>模型 <b>{len(models)}</b></span>
<span>后端 <b>{', '.join(backends)}</b></span>
<span>成功用例 <b>{ok_count}/{len(results)}</b></span>
<span>抓取/预热 <b>{args.duration}s / {args.warmup}s</b></span>
<span>窗口 <b>{escape(args.size)}</b></span>
</div>
<div class="cards">{findings_html}</div>

<h2>总览</h2>
<div class="legend">
<span style="--c:#3b82f6">Draw（CPU 录制/提交）</span>
<span style="--c:#f59e0b">GPU 阻塞（acquire+waitIdle）</span>
<span style="--c:#cbd5e1">其他</span>
</div>
<table>
<thead><tr>
<th>模型</th><th>版本</th><th>后端</th><th>GPU</th><th>状态</th><th>帧数</th>
<th>FPS</th><th>帧时间(ms)</th><th>构成</th>
<th>ModelUpdate</th><th>DrawModel</th><th>acquire</th><th>waitIdle</th>
<th>GPU阻塞</th><th>Draw最大</th>
</tr></thead><tbody>{''.join(rows_html)}</tbody></table>

<h2>同模型 GL vs VK</h2>
<table>
<thead><tr><th>模型</th><th>版本</th><th>GL FPS</th><th>GL 帧时间(ms)</th>
<th>VK FPS</th><th>VK 帧时间(ms)</th><th>VK/GL</th></tr></thead>
<tbody>{''.join(pair_rows) or '<tr><td colspan="7">无配对数据</td></tr>'}</tbody></table>

<h2>口径与说明</h2>
<div class="notes"><ul>
<li><b>FPS</b> = <code>py/Draw</code> 次数 / 抓取窗口墙钟时长（<code>tracy-capture -s</code> 实际耗时）。</li>
<li><b>帧时间</b>：Vulkan = <code>py/vkBeginFrame + py/Draw + py/vkEndFrame</code>；OpenGL 无 FrameMark，取 <code>py/Draw + py/Update</code>，不含 <code>swap_buffers</code> 与 Python 循环开销。</li>
<li><b>GPU 阻塞</b> = <code>vk/acquire + vk/waitIdle</code>，仅 Vulkan 有意义。</li>
<li><b>GPU 归属</b>取自应用启动日志的 <code>Device:</code> 行（GL 用 <code>glGetString(GL_RENDERER)</code>，VK 用 <code>VkPhysicalDeviceProperties</code>）。</li>
<li><b>ModelUpdate</b> 为 <code>CubismModel::Update()</code>（顶点形变），<b>DrawModel</b> 为渲染器录制阶段。</li>
<li>原始 trace 与逐用例 CSV 都在同目录，可用 Tracy GUI 复现。</li>
</ul></div>
</div></body></html>"""
    (out_dir / "report.html").write_text(html_doc, encoding="utf-8")


def main():
    ap = argparse.ArgumentParser(description="Tracy 压测：模型 × 后端 → trace + CSV + 报告")
    ap.add_argument("--name", default=None, help="输出目录名（默认 run_<时间戳>）")
    ap.add_argument("--models", default="llny", help="逗号分隔：模型目录名或 json 路径；all=Resources 下全部模型")
    ap.add_argument("--all", action="store_true", help="等价于 --models all")
    ap.add_argument("--backends", default="gl,vk", help="逗号分隔：gl,vk")
    ap.add_argument("--duration", type=int, default=6, help="抓取秒数，默认 6")
    ap.add_argument("--warmup", type=int, default=4, help="启动后预热秒数，默认 4")
    ap.add_argument("--size", default="800x800", help="窗口尺寸（GL/VK 统一），默认 800x800")
    ap.add_argument("--out", default=str(ROOT / "bench" / "traces"), help="输出根目录，默认 bench/traces/")
    ap.add_argument("--report-only", action="store_true",
                    help="不重新采样，用目录里已有的 summary.csv 重新生成 summary.md / report.html")
    args = ap.parse_args()

    name = args.name or f"run_{datetime.now().strftime('%Y%m%d_%H%M%S')}"
    out_dir = Path(args.out) / name
    out_dir.mkdir(parents=True, exist_ok=True)

    if args.report_only:
        csv_file = out_dir / "summary.csv"
        if not csv_file.exists():
            raise SystemExit(f"缺少 {csv_file}，无法只重建报告")
        num_keys = ("fps", "frame_ms", "draw_ms", "update_ms", "draw_model_ms",
                    "record_ms", "acquire_ms", "wait_idle_ms", "gpu_stall_ms", "draw_max_ms")
        with open(csv_file, encoding="utf-8") as f:
            rows = list(csv.DictReader(f))
        results = []
        for row in rows:
            r = {k: (v if v not in ("", None) else None) for k, v in row.items()}
            for k in num_keys:
                r[k] = float(r[k]) if r.get(k) else None
            r["frames"] = int(r["frames"]) if r.get("frames") else None
            results.append(r)
        models = sorted({f"v{r['version']}/{r['model']}" for r in results})
        backends = sorted({r["backend"] for r in results})
        write_report(out_dir, results, name, args, models, backends)
        print(f"[tracy_bench] 报告已重建: {out_dir / 'report.html'}")
        return

    if args.all or args.models.strip().lower() == "all":
        models = discover_models()
        if not models:
            raise SystemExit("Resources 下没有找到任何模型")
    else:
        models = [resolve_model(m.strip()) for m in args.models.split(",") if m.strip()]
    backends = [b.strip().lower() for b in args.backends.split(",") if b.strip()]
    for b in backends:
        if b not in EXAMPLES:
            raise SystemExit(f"未知后端: {b}（可选 gl,vk）")

    capture, csvexport = find_tracy_tools()
    print(f"[tracy_bench] 输出目录: {out_dir}")
    print(f"[tracy_bench] 工具: {capture}")
    print(f"[tracy_bench] 用例: {len(models)} 个模型 × {len(backends)} 个后端")

    results = []
    with open(out_dir / "run.log", "w", encoding="utf-8") as log:
        log.write(f"models={[str(m) for m in models]}\nbackends={backends}\n")
        for model in models:
            for backend in backends:
                print(f"[run] {model_label(model)} / {backend} ...", flush=True)
                results.append(run_case(
                    backend, model, out_dir, args.duration, args.warmup,
                    args.size, capture, csvexport, log,
                ))

    write_report(out_dir, results, name, args,
                 [f"v{version_of(m)}/{model_label(m)}" for m in models], backends)
    print(f"[tracy_bench] 完成: {out_dir / 'summary.md'}")


if __name__ == "__main__":
    main()
