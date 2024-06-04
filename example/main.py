import live2d

from PySide6.QtCore import QTimerEvent, Qt
from PySide6.QtWidgets import QApplication
from PySide6.QtOpenGLWidgets import QOpenGLWidget
from OpenGL.GL import *


class Win(QOpenGLWidget):
    timer: int = -1
    model: live2d.LAppModel

    def __init__(self) -> None:
        super().__init__()
        # self.setWindowFlags(Qt.WindowType.FramelessWindowHint | Qt.WindowType.Tool)
        self.setAttribute(Qt.WidgetAttribute.WA_TranslucentBackground, True)

    def initializeGL(self) -> None:

        self.makeCurrent()


        live2d.InitializeGlew()

        # テクスチャサンプリング設定
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);

        # 透過設定
        glEnable(GL_BLEND);
        glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

        glViewport(0, 0, self.width(), self.height());

        self.model = live2d.LAppModel()
        self.model.LoadAssets("../Resources/Hiyori/", "Hiyori.model3.json")

        self.timer = self.startTimer(int(1000 / 30))

    def resizeGL(self, w: int, h: int) -> None:
        return super().resizeGL(w, h)
    
    def paintGL(self) -> None:
        
        glClearColor(0.0, 0.0, 0.0, 0.0)
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT)
        glClearDepth(1.0)

        self.model.Update(self.width(), self.height())
    
    def timerEvent(self, a0: QTimerEvent | None) -> None:
        self.update() 


if __name__ == "__main__":
    import sys
    live2d.InitializeCubism()

    app = QApplication(sys.argv)
    win = Win()
    win.show()
    app.exec()

    live2d.ReleaseCubism()