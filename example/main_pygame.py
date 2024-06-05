import pygame
from pygame.locals import *
from OpenGL.GL import *
from OpenGL.GLUT import *
from OpenGL.GLU import *

import live2d

def draw():
    pygame.display.flip()
    pygame.time.wait(10)

def main():
    pygame.init()
    live2d.InitializeCubism()

    display = (800,600)
    pygame.display.set_mode(display, DOUBLEBUF|OPENGL)

    live2d.InitializeGlew()
    live2d.SetGLProperties()

    model = live2d.LAppModel()
    model.LoadAssets("./Resources/Haru/", "Haru.model3.json")

    model.Resize(*display)

    running = True

    while True:
        for event in pygame.event.get():
            if event.type == pygame.QUIT:
                running = False
                break
            if event.type == pygame.MOUSEBUTTONDOWN:
                x, y = pygame.mouse.get_pos()
                model.Touch(x, y)

        if not running: break

        live2d.ClearBuffer()
        model.Update(*display)
        draw()

    live2d.ReleaseCubism()
    pygame.quit()
    quit()

if __name__ == "__main__":
    main()
