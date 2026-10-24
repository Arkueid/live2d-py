"""OpenGL 压测程序（与 bench_vk.py 对等，供 tests/tracy_bench.py 调用）。

只做固定窗口尺寸的裸渲染循环：Rotate -> Update -> Draw -> swap，无交互、无音频。
"""

import argparse
import math
import os
import sys
import time

ROOT = os.path.dirname(os.path.dirname(os.path.dirname(os.path.abspath(__file__))))
sys.path.insert(0, os.path.join(ROOT, "package"))

if sys.platform.startswith("linux") and not os.environ.get("PYOPENGL_PLATFORM"):
    os.environ["PYOPENGL_PLATFORM"] = "glx"

import glfw
import OpenGL.GL as GL

import live2d


def parse_size(text):
    w, h = text.lower().split("x")
    return int(w), int(h)


def main():
    ap = argparse.ArgumentParser()
    ap.add_argument("--model", required=True, help="模型 json 路径")
    ap.add_argument("--size", default="800x800", help="窗口尺寸，默认 800x800")
    ap.add_argument("--seconds", type=float, default=0, help="运行秒数，0=一直跑")
    args = ap.parse_args()
    width, height = parse_size(args.size)

    live2d.enableLog(True)
    live2d.setLogLevel(live2d.LogLevels.LV_INFO)  # 输出 Device 行，供压测脚本记录 GPU

    if not glfw.init():
        raise RuntimeError("glfw.init failed")
    glfw.window_hint(glfw.RESIZABLE, False)
    window = glfw.create_window(width, height, "live2d-py GL bench", None, None)
    if not window:
        glfw.terminate()
        raise RuntimeError("glfw.create_window failed")
    glfw.make_context_current(window)
    glfw.swap_interval(0)

    live2d.init()
    live2d.glInit()

    model = live2d.Model()
    model.LoadModelJson(args.model, create_renderer=False)
    model.CreateRenderer()
    model.Resize(width, height)
    model.SetAutoBlink(True)
    model.SetAutoBreath(True)
    model.StartMotion("", 0, live2d.MotionPriority.FORCE)

    progress = 0.0
    last = time.time()
    start = last
    while not glfw.window_should_close(window):
        glfw.poll_events()

        now = time.time()
        progress += math.pi * 2 / 3 * (now - last)
        last = now

        model.Rotate(math.sin(progress) * 5)
        model.Update()

        live2d.clearBuffer(0.0, 0.0, 0.0, 0.0)
        model.Draw()
        glfw.swap_buffers(window)

        if args.seconds and now - start >= args.seconds:
            break

    glfw.terminate()


if __name__ == "__main__":
    main()
