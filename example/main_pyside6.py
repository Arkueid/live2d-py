from PySide6.QtGui import QMouseEvent
import live2d

from PySide6.QtCore import QTimerEvent, Qt
from PySide6.QtWidgets import QApplication
from PySide6.QtOpenGLWidgets import QOpenGLWidget
from OpenGL.GL import *

def callback():
    print("motion end")


class Win(QOpenGLWidget):
    model: live2d.LAppModel

    def __init__(self) -> None:
        super().__init__()
        # self.setWindowFlags(Qt.WindowType.FramelessWindowHint | Qt.WindowType.Tool)
        self.setAttribute(Qt.WidgetAttribute.WA_TranslucentBackground, True)
        self.a = 0

    def initializeGL(self) -> None:
        # 将当前窗口作为 OpenGL 的上下文
        # 图形会被绘制到当前窗口
        self.makeCurrent()

        # 初始化Glew
        live2d.InitializeGlew()
        # 设置 OpenGL 绘图参数
        live2d.SetGLProperties()
        # 创建模型
        self.model = live2d.LAppModel()
        # 测试模型文件是否被修改过，目前来说没什么用
        print("moc consistency: ", self.model.HasMocConsistencyFromFile('./Resources/Hiyori/Hiyori.moc3'));
        # 加载模型参数
        self.model.LoadAssets("./Resources/Haru/", "Haru.model3.json")

        # 以 fps = 30 的频率进行绘图
        self.startTimer(int(1000 / 30))

    def resizeGL(self, w: int, h: int) -> None:
        # 使模型的参数按窗口大小进行更新
        self.model.Resize(w, h)
    
    def paintGL(self) -> None:
        
        live2d.ClearBuffer()

        self.model.Update(self.width(), self.height())
    
    def timerEvent(self, a0: QTimerEvent | None) -> None:
        self.update() 

        if self.a == 0: # 测试一次播放动作和回调函数
            self.model.StartMotion("TapBody", 0, 3, callback)
            self.a += 1

    def mousePressEvent(self, event: QMouseEvent) -> None:
        # 传入鼠标点击位置的窗口坐标
        self.model.Touch(event.pos().x(), event.pos().y());

    def mouseMoveEvent(self, event: QMouseEvent) -> None:
        self.model.Drag(event.pos().x(), event.pos().y())


if __name__ == "__main__":
    import sys
    live2d.InitializeCubism()

    app = QApplication(sys.argv)
    win = Win()
    win.show()
    app.exec()

    live2d.ReleaseCubism()