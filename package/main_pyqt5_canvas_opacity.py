import sys

from live2d.utils.canvas import Canvas

"""
设置模型整体透明度示例
"""

# import live2d.v2 as live2d


import live2d.v3 as live2d
import math
from PyQt5.QtCore import Qt
from PyQt5.QtWidgets import QOpenGLWidget



class Live2DCanvas(QOpenGLWidget):
    def __init__(self):
        super().__init__()
        self.model: None | live2d.LAppModel = None

        # tool for controlling model opacity
        self.canvas: None | Canvas = None

        self.setWindowTitle("Live2DCanvas")
        self.setAttribute(Qt.WidgetAttribute.WA_TranslucentBackground)
        self.radius_per_frame = math.pi * 0.5 / 120
        self.total_radius = 0

    def initializeGL(self):
        live2d.glewInit()
        self.model = live2d.LAppModel()
        if live2d.LIVE2D_VERSION == 3:
            self.model.LoadModelJson("resources/v3/llny/llny.model3.json")
        else:
            self.model.LoadModelJson("resources/v2/kasumi2/kasumi2.model.json")
        
        # must be created after opengl context is configured
        self.canvas = Canvas()

        self.startTimer(int(1000 / 120))
    
    def timerEvent(self, a0):
        self.total_radius += self.radius_per_frame
        v = abs(math.cos(self.total_radius))
        self.canvas.setOutputOpacity(v)
        self.update()

    def on_draw(self):
        live2d.clearBuffer()
        self.model.Draw()

    def paintGL(self):
        self.model.Update()
        
        self.canvas.draw(self.on_draw)

    def resizeGL(self, width: int, height: int):
        self.model.Resize(width, height)

        self.canvas.setSize(width, height)


if __name__ == '__main__':
    from PyQt5.QtWidgets import QApplication
    live2d.init()
    app = QApplication(sys.argv)
    win = Live2DCanvas()
    win.show()
    app.exec()
    live2d.dispose()
