"""Motion callback lifecycle tests (headless, no OpenGL context).

Covers the per-motion callback contract shared by V2/V3:
  - mount:            holder refcount +1
  - remount:          old callback released, total stays +1 (single-slot semantics)
  - preempt remount:  finish still fires after preemption (regression for the
                      V2 slot-stealing bug and the wrapper cross-motion bug)
  - burst re-entry:   100 StartMotion calls from inside the finish callback
                      (vector reallocations mid-iteration) must not crash
  - dealloc:          uncalled callbacks released on model destruction
"""
import gc
import os
import sys

import live2d

RESOURCES = os.path.abspath(
    os.path.join(os.path.dirname(os.path.abspath(__file__)), "..", "Resources")
)


def _drive_until_finish(model, events, max_frames=4000):
    for i in range(1, max_frames + 1):
        model.Update(1.0 / 60.0)
        if any(e[0] == "finish" for e in events):
            return i
    raise AssertionError(f"motion not finished in {max_frames} frames; events={events[:4]}")


def _run_case(label, model_json):
    print(f"---- {label}: {os.path.basename(model_json)} ----")
    events = []
    model = live2d.Model()
    model.LoadModelJson(model_json, create_renderer=False)

    group = next(iter(model.GetMotionGroups()))

    def on_start(g, n):
        events.append(("start", g, n))

    def on_finish(g, n):
        events.append(("finish", g, n))

    # 1) mount: refcount +1 each
    rc_s0, rc_f0 = sys.getrefcount(on_start), sys.getrefcount(on_finish)
    model.StartMotion(group, 0, 3, onStart=on_start, onFinish=on_finish)
    assert sys.getrefcount(on_start) == rc_s0 + 1, "onStart not held"
    assert sys.getrefcount(on_finish) == rc_f0 + 1, "onFinish not held"

    # 2) start fires (V2: synchronously on mount; V3: on first Update)
    model.Update(1.0 / 60.0)
    assert any(e[0] == "start" for e in events), events
    print("  start OK")

    # 3) remount same motion: old callback released, total stays +1
    model.StartMotion(group, 0, 3, onStart=on_start, onFinish=on_finish)
    assert sys.getrefcount(on_start) == rc_s0 + 1, "remount onStart leak"
    assert sys.getrefcount(on_finish) == rc_f0 + 1, "remount onFinish leak"
    print("  remount refcount OK")

    # 4) preempt remount then drive to natural finish (same callback object on
    #    both mounts -- regression: preempted entry must not steal the slot)
    model.StartMotion(group, 0, 3, onStart=on_start, onFinish=on_finish)
    frame = _drive_until_finish(model, events)
    finishes = [e for e in events if e[0] == "finish"]
    assert finishes, events[:6]
    assert all(e[1] == group and e[2] == 0 for e in finishes), finishes
    print(f"  finish OK at frame {frame} ({len(finishes)} fired)")

    # 5) burst re-entry stress: 100 StartMotion calls issued from inside the
    #    finish callback -- several vector reallocations happen mid-iteration
    #    (v3 csmVector 10->20->40->80->160). Must not crash; the 99 preempted
    #    entries stay silent and only the last-mounted entry fires its finish.
    burst_events = []
    burst_left = [100]

    def burst_finish(g, n):
        burst_events.append(("finish", g, n))
        left, burst_left[0] = burst_left[0], 0  # first round only
        for b in range(left):
            # strictly increasing priority 4..103 bypasses the <= reserve
            # check on v3 and the < check on v2
            model.StartMotion(
                group, 0, 4 + b,
                onFinish=lambda g2, n2: burst_events.append(("finish2", g2, n2)))

    model.StartMotion(group, 0, 3, onFinish=burst_finish)
    for i in range(1, 4001):
        model.Update(1.0 / 60.0)
        if any(e[0] == "finish2" for e in burst_events):
            break
    else:
        raise AssertionError(f"burst finish not fired; events={burst_events[:4]}")
    firsts = [e for e in burst_events if e[0] == "finish"]
    seconds = [e for e in burst_events if e[0] == "finish2"]
    assert len(firsts) == 1 and firsts[0][1] == group, firsts
    assert len(seconds) == 1 and seconds[0][1] == group, seconds
    # all burst mounts are overwritten; no leak beyond the live slot
    assert burst_left[0] == 0
    print(f"  burst re-entry OK ({len(burst_events)} events)")

    # 6) uncalled callbacks released on model destruction
    model.StartMotion(group, 0, 3, onStart=on_start, onFinish=on_finish)
    assert sys.getrefcount(on_start) == rc_s0 + 1
    del model
    gc.collect()
    assert sys.getrefcount(on_start) == rc_s0, "dealloc: onStart leaked"
    assert sys.getrefcount(on_finish) == rc_f0, "dealloc: onFinish leaked"
    print("  dealloc OK")


def main():
    live2d.init()
    _run_case("V2", os.path.join(RESOURCES, "v2", "shizuku/shizuku.model.json"))
    _run_case("V3", os.path.join(RESOURCES, "v3", "Haru/Haru.model3.json"))
    print("ALL OK")


if __name__ == "__main__":
    main()
