import os.path
import tkinter

import pyautogui
from OpenGL import GL
from pyopengltk import OpenGLFrame
import resources


# import live2d.v2 as live2d
import live2d.v3 as live2d

class AppOgl(OpenGLFrame):
    def __init__(self, master, **kw):
        super().__init__(master, **kw)
        self.model = None

    def initgl(self):
        """Initalize gl states when the frame is created"""
        live2d.init()
        live2d.glewInit()

        GL.glViewport(0, 0, self.width, self.height)
        GL.glClearColor(0.0, 1.0, 0.0, 0.0)
        self.model = live2d.LAppModel()
        if live2d.LIVE2D_VERSION == 2:
            self.model.LoadModelJson(os.path.join(resources.RESOURCES_DIRECTORY, "v2/kasumi2/kasumi2.model.json"))
        else:
            self.model.LoadModelJson(os.path.join(resources.RESOURCES_DIRECTORY, "v3/haru/haru.model3.json"))
        self.model.Resize(self.width, self.height)

    def redraw(self):
        """Render a single frame"""
        GL.glClear(GL.GL_COLOR_BUFFER_BIT)

        screen_x, screen_y = pyautogui.position()
        x = screen_x - self.winfo_rootx()
        y = screen_y - self.winfo_rooty()

        self.model.Update()
        self.model.Drag(x, y)
        self.model.Draw()



if __name__ == '__main__':
    root = tkinter.Tk()
    app = AppOgl(root, width=320, height=200)
    app.pack(fill=tkinter.BOTH, expand=tkinter.YES)
    app.animate = 1
    app.after(100, app.printContext)
    app.mainloop()

    live2d.dispose()