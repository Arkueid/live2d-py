import live2d

from PySide6.QtCore import QTimerEvent, Qt
from PySide6.QtWidgets import QApplication
from PySide6.QtOpenGLWidgets import QOpenGLWidget
from OpenGL.GL import *

def callback():
    print("motion end")


class Win(QOpenGLWidget):
    timer: int = -1
    model: live2d.LAppModel

    def __init__(self) -> None:
        super().__init__()
        # self.setWindowFlags(Qt.WindowType.FramelessWindowHint | Qt.WindowType.Tool)
        self.setAttribute(Qt.WidgetAttribute.WA_TranslucentBackground, True)
        self.a = 0

    def initializeGL(self) -> None:

        self.makeCurrent()


        live2d.InitializeGlew()
        live2d.SetGLProperties()

        self.model = live2d.LAppModel()
        # 测试模型文件是否被修改过
        print("moc consistency: ", self.model.HasMocConsistencyFromFile('./Resources/Hiyori/Hiyori.moc3'));
        # 加载模型参数
        self.model.LoadAssets("./Resources/Hiyori/", "Hiyori.model3.json")

        self.timer = self.startTimer(int(1000 / 30))

    def resizeGL(self, w: int, h: int) -> None:
        return super().resizeGL(w, h)
    
    def paintGL(self) -> None:
        
        live2d.ClearBuffer()

        self.model.Update(self.width(), self.height())
    
    def timerEvent(self, a0: QTimerEvent | None) -> None:
        self.update() 
        if self.a == 0:
            self.model.StartMotion("TapBody", 0, 3, callback)

            self.a += 1



if __name__ == "__main__":
    import sys
    live2d.InitializeCubism()

    app = QApplication(sys.argv)
    win = Win()
    win.show()
    app.exec()

    live2d.ReleaseCubism()