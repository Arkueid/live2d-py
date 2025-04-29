from PySide6.QtOpenGLWidgets import QOpenGLWidget
from PySide6.QtWidgets import QApplication
from PySide6.QtCore import QCoreApplication, Qt
import sys
import os 
import time

from live2d import v3 as live2d
import resources

live2d.init()

model = live2d.Model()
model.LoadModelJson(os.path.join(resources.RESOURCES_DIRECTORY, "v3/Haru/Haru.model3.json"))

class Live2DWidget(QOpenGLWidget):

    def __init__(self, model: live2d.Model, parent=None):
        super().__init__(parent)
        self.model = model
        self.lastCt = 0

    def closeEvent(self, event):
        print("release opengl resources")
        self.makeCurrent()
        self.model.DestroyRenderer() # release buffers and textures
        live2d.glRelease() # release shaders
        self.doneCurrent()
        return super().closeEvent(event)
    
    def timerEvent(self, event):
        self.update()

    def initializeGL(self):
        live2d.glInit()
        self.model.CreateRenderer(2)

        self.lastCt = time.time()
        self.startTimer(int(1000 / 60))

    def paintGL(self):
        live2d.clearBuffer()

        ct = time.time()
        self.model.Update(ct - self.lastCt)
        self.lastCt = ct

        self.model.Draw()

    def resizeGL(self, width, height):
        self.model.Resize(width, height)


QCoreApplication.setAttribute(Qt.AA_ShareOpenGLContexts, False)
app = QApplication(sys.argv)

win = Live2DWidget(model)
win.show()

app.exec()

win = Live2DWidget(model)
win.show()
app.exec()

live2d.dispose()
