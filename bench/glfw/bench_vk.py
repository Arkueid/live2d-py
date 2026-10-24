"""Vulkan 压测程序（与 bench_gl.py 对等，供 tests/tracy_bench.py 调用）。

帧流程全部由 Python 驱动：vkBeginFrame -> Draw -> vkEndFrame，无交互、无音频。
"""

import argparse
import ctypes
import math
import os
import sys
import time

ROOT = os.path.dirname(os.path.dirname(os.path.dirname(os.path.abspath(__file__))))
sys.path.insert(0, os.path.join(ROOT, "package"))

# 默认 MAILBOX 会按显示器垂直同步节流（本机 165Hz -> ~330 FPS），压测需要真实吞吐，
# 因此与 bench_gl.py 的 swap_interval(0) 对齐，关闭呈现节流
os.environ.setdefault("LIVE2D_VK_PRESENT_MODE", "immediate")

import glfw
from glfw.GLFW import _glfw as _glfw_lib

import live2d


def parse_size(text):
    w, h = text.lower().split("x")
    return int(w), int(h)


def get_native_window_handle(window):
    if sys.platform != "win32":
        raise NotImplementedError(f"native window handle not implemented for {sys.platform}")
    _glfw_lib.glfwGetWin32Window.restype = ctypes.c_void_p
    _glfw_lib.glfwGetWin32Window.argtypes = [ctypes.c_void_p]
    hwnd = _glfw_lib.glfwGetWin32Window(ctypes.cast(window, ctypes.c_void_p).value)
    return hwnd, 0


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
    glfw.window_hint(glfw.CLIENT_API, glfw.NO_API)
    glfw.window_hint(glfw.RESIZABLE, False)
    window = glfw.create_window(width, height, "live2d-py VK bench", None, None)
    if not window:
        glfw.terminate()
        raise RuntimeError("glfw.create_window failed")

    live2d.init()
    window_handle, display_handle = get_native_window_handle(window)
    live2d.vkInit(window_handle, display_handle)

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

        frame = live2d.vkBeginFrame()
        if frame is not None:
            model.Draw()
            live2d.vkEndFrame()

        if args.seconds and now - start >= args.seconds:
            break

    model.DestroyRenderer()
    live2d.vkDestroy()
    live2d.dispose()
    glfw.terminate()


if __name__ == "__main__":
    main()
