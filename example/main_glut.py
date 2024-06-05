from OpenGL.GL import *
from OpenGL.GLUT import *
from OpenGL.GLU import *
import sys

import live2d

model: live2d.LAppModel = None
width = 800
height = 600

# OpenGL initialization function
def init_gl():
    global model

    live2d.InitializeGlew()
    live2d.SetGLProperties()

    if model is None:
        model = live2d.LAppModel()
        model.LoadAssets("./Resources/Haru", "Haru.model3.json")

# Display function
def display():
    live2d.ClearBuffer()

    model.Update(width, height)

    glutSwapBuffers()  # Swap the front and back buffers

# Window reshape function
def reshape(w, h):
    glViewport(0, 0, w, h)
    model.Resize(w, h)


# Idle function
def idle():
    glutPostRedisplay()


def mouse_click_callback(button, state, x, y):
    if button == 0: # left button
        model.Touch(x, y)
    elif button == 1: # right button
        pass


def mouse_motion_callback(x, y):
    model.Drag(x, y)


if __name__ == "__main__":
    glutInit(sys.argv)
    glutInitDisplayMode(GLUT_RGBA | GLUT_DOUBLE | GLUT_DEPTH)
    glutInitWindowSize(width, height)
    glutInitWindowPosition(100, 100)
    glutCreateWindow(b"OpenGL Window")

    live2d.InitializeCubism()

    init_gl()

    glutDisplayFunc(display)
    glutReshapeFunc(reshape)
    glutIdleFunc(idle)
    glutMouseFunc(mouse_click_callback)
    glutMotionFunc(mouse_motion_callback)
    glutCloseFunc(live2d.ReleaseCubism)

    glutMainLoop()

