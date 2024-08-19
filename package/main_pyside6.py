from PySide6.QtGui import QMouseEvent
# import live2d.v2 as live2d
import live2d.v3 as live2d
import os
import resouces
from PySide6.QtCore import QTimerEvent
from PySide6.QtWidgets import QApplication
from PySide6.QtOpenGLWidgets import QOpenGLWidget


def callback():
    print("motion end")


class Win(QOpenGLWidget):
    model: live2d.LAppModel

    def __init__(self) -> None:
        super().__init__()
        # self.setWindowFlags(Qt.WindowType.FramelessWindowHint | Qt.WindowType.Tool)
        # self.setAttribute(Qt.WidgetAttribute.WA_TranslucentBackground, True)
        self.a = 0
        self.resize(270, 200)

    def initializeGL(self) -> None:
        # 将当前窗口作为 OpenGL 的上下文
        # 图形会被绘制到当前窗口
        self.makeCurrent()

        if live2d.LIVE2D_VERSION == 3:
            live2d.glewInit()
            live2d.setGLProperties()

        # 创建模型
        self.model = live2d.LAppModel()

        # 加载模型参数
        if live2d.LIVE2D_VERSION == 2:
            # 适用于 2 的模型
            self.model.LoadModelJson(os.path.join(resouces.RESOURCES_DIRECTORY, "v2/kasumi2/kasumi2.model.json"))
        elif live2d.LIVE2D_VERSION == 3:
            # 适用于 3 的模型
            self.model.LoadModelJson(os.path.join(resouces.RESOURCES_DIRECTORY, "v3/Haru/Haru.model3.json"))

        # 以 fps = 30 的频率进行绘图
        self.startTimer(int(1000 / 30))

    def resizeGL(self, w: int, h: int) -> None:
        if self.model:
            # 使模型的参数按窗口大小进行更新
            self.model.Resize(w, h)
    
    def paintGL(self) -> None:
        
        live2d.clearBuffer()

        self.model.Update()

        self.model.Draw()
    
    def timerEvent(self, a0: QTimerEvent | None) -> None:

        if self.a == 0: # 测试一次播放动作和回调函数
            self.model.StartMotion("TapBody", 0, live2d.MotionPriority.FORCE.value, onFinishMotionHandler=callback)
            self.a += 1
        
        self.update() 

    def mousePressEvent(self, event: QMouseEvent) -> None:
        # 传入鼠标点击位置的窗口坐标
        self.model.Touch(event.pos().x(), event.pos().y());

    def mouseMoveEvent(self, event: QMouseEvent) -> None:
        self.model.Drag(event.pos().x(), event.pos().y())


if __name__ == "__main__":
    import sys
    live2d.init()

    app = QApplication(sys.argv)
    win = Win()
    win.show()
    app.exec()

    live2d.dispose()