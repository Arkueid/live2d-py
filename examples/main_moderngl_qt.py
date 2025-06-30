import sys
import numpy as np
import moderngl
from PyQt5 import QtWidgets, QtOpenGL
# import live2d.v3 as live2d
import live2d.v2 as live2d
import os

import resources


class GLWidget(QtOpenGL.QGLWidget):
    def __init__(self, parent=None):
        # 设置OpenGL版本为3.3
        fmt = QtOpenGL.QGLFormat()
        # fmt.setVersion(3, 3)
        # fmt.setProfile(QtOpenGL.QGLFormat.CoreProfile)
        # fmt.setSampleBuffers(True)
        super().__init__(fmt, parent)
        
        # 初始化变量
        self.ctx = None
        self.model = None
        
        # 设置窗口尺寸
        self.setMinimumSize(800, 600)
    
    def initializeGL(self):
        # 创建ModernGL上下文
        self.ctx = moderngl.create_context()

        live2d.glInit()
        
        # 设置清屏颜色
        self.ctx.clear_color = (0.2, 0.2, 0.2, 1.0)

        self.model = live2d.LAppModel()
        self.model.LoadModelJson(
            os.path.join(
                resources.RESOURCES_DIRECTORY, 
                "v3/llny/llny.model3.json" if live2d.LIVE2D_VERSION == 3
                         else "v2/kasumi2/kasumi2.model.json"
                         ))
    
    def paintGL(self):
        # 清除屏幕
        self.ctx.clear()

        self.model.Update()
        self.model.Draw()
        
        # 请求刷新
        self.update()
    
    def resizeGL(self, w, h):
        self.ctx.viewport = (0, 0, w, h)
        self.model.Resize(w, h)


if __name__ == '__main__':
    live2d.init()
    app = QtWidgets.QApplication(sys.argv)
    window = GLWidget()
    window.setWindowTitle('ModernGL + PyQt5 + live2d')
    window.show()
    app.exec_()
    live2d.dispose()