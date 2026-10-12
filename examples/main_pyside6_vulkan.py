"""PySide6 + Vulkan backend example (experimental).

与 OpenGL 版 (main_pyside6_simple.py) 的区别：
1. 使用纯 QWindow 而非 QOpenGLWidget（不让 Qt 管理 Vulkan）。
2. 不调用 live2d.glInit / clearBuffer ——
   Vulkan 的 acquire → clear → draw → present 全部在 model.Draw() 内部完成。
3. 向 live2d.vkInit 传入 QWindow 的原生窗口句柄 (Windows: HWND)。
   surface 创建由 VulkanManager 内部的 vkCreateWin32SurfaceKHR 完成。

仅验证过 v3 模型。v2 的 Vulkan shader 从 CWD 的 ./FrameworkShaders 读取。
"""

import os
import platform
import sys

from PySide6.QtCore import QTimerEvent, Qt
from PySide6.QtGui import QWindow

import resources

import live2d

live2d.enableLog(True)
live2d.setLogLevel(live2d.LogLevels.LV_WARN)


LIVE2D_VERSION = 2


def get_native_window_handle(window: QWindow):
    """从 QWindow 获取平台原生窗口句柄。

    Windows 返回 HWND，其他平台未实现 (TODO)。
    """
    sysname = platform.system()
    if sysname == "Windows":
        # QWindow.winId() 返回 sip.voidptr 类型的 HWND，转成 int
        hwnd = int(window.winId())
        return hwnd, 0  # (window_handle, display_handle)
    else:
        # TODO: 支持 Linux(X11/Wayland) / macOS
        raise NotImplementedError(f"native window handle not implemented for {sysname}")


class VulkanWindow(QWindow):
    def __init__(self) -> None:
        super().__init__()
        self.setSurfaceType(QWindow.RasterSurface)
        self.resize(800, 800)
        self.model: live2d.Model | None = None
        self._timer_id: int | None = None

    def exposeEvent(self, event) -> None:
        # 原生窗口创建后再初始化 Vulkan
        if self.model is None:
            self._init_vulkan()

    def _init_vulkan(self) -> None:
        window_handle, display_handle = get_native_window_handle(self)
        # VulkanManager 会自动追加平台 surface 扩展 (VK_KHR_surface /
        # VK_KHR_win32_surface)，所以这里传空列表即可。
        live2d.vkInit(window_handle, [], display_handle)

        self.model = live2d.Model()
        model_path = os.path.abspath(
            os.path.join(resources.RESOURCES_DIRECTORY, "v3", "Haru", "Haru.model3.json") if LIVE2D_VERSION == 3 else os.path.join(resources.RESOURCES_DIRECTORY, "v2", "kasumi2", "kasumi2.model.json")
        )
        self.model.LoadModelJson(model_path, create_renderer=False)
        self.model.CreateRenderer()
        self.model.Resize(self.width(), self.height())
        self.model.StartRandomMotion(priority=3)

        # 120 fps 绘制
        self._timer_id = self.startTimer(int(1000 / 120))

    def resizeEvent(self, event) -> None:
        if self.model is not None:
            self.model.Resize(self.width(), self.height())

    def mouseMoveEvent(self, event) -> None:
        if self.model is not None:
            self.model.Drag(event.position().x(), event.position().y())

    def mousePressEvent(self, event) -> None:
        if self.model is not None and event.button() == Qt.MouseButton.LeftButton:
            self.model.SetRandomExpression()
            self.model.StartRandomMotion(priority=3)

    def timerEvent(self, event: QTimerEvent | None) -> None:
        if event.timerId() == self._timer_id and self.model is not None:
            self.model.Update()
            # Vulkan 帧 (acquire → clear → draw → present) 全部在 Draw() 内部完成
            self.model.Draw()
            if self.model.IsMotionFinished():
                self.model.StartRandomMotion(priority=3)

    def cleanup(self) -> None:
        if self._timer_id is not None:
            self.killTimer(self._timer_id)
            self._timer_id = None
        if self.model is not None:
            self.model.DestroyRenderer()
            self.model = None
        live2d.vkDestroy()


if __name__ == "__main__":
    from PySide6.QtCore import QTimer
    from PySide6.QtWidgets import QApplication

    # CubismFramework 初始化（shader dir 使用包目录）
    live2d.init()

    app = QApplication(sys.argv)

    win = VulkanWindow()
    win.setTitle("live2d-py Vulkan (PySide6)")
    win.show()

    # 确保窗口句柄已创建，处理事件
    app.processEvents()

    # LIVE2D_VK_DURATION 设置后，运行指定秒数自动关闭（无头验证用）
    auto_close_after = float(os.environ.get("LIVE2D_VK_DURATION", "0")) or None
    if auto_close_after is not None:
        QTimer.singleShot(int(auto_close_after * 1000), app.quit)

    app.exec()

    win.cleanup()
    live2d.dispose()
