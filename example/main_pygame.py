import pygame
from pygame.locals import *

import live2d.debug as live2d
# import live2d

def draw():
    pygame.display.flip()
    pygame.time.wait(10)

def main():
    pygame.init()
    live2d.InitializeCubism()

    display = (400,300)
    pygame.display.set_mode(display, DOUBLEBUF|OPENGL)

    live2d.InitializeGlew()
    live2d.SetGLProperties()

    model = live2d.LAppModel()

    model2 = live2d.LAppModel()

    del model

    model = live2d.LAppModel()
    model.LoadAssets("./live2d-desktop/Resources/Haru/", "Haru.model3.json")

    model.Resize(*display)

    running = True

    dx: float = 0.0
    dy: float = 0.0
    scale: float = 1.0

    cnt = 0

    while True:
        for event in pygame.event.get():
            if event.type == pygame.QUIT:
                running = False
                break
            if event.type == pygame.MOUSEBUTTONDOWN:
                x, y = pygame.mouse.get_pos()
                model.Touch(x, y)
            if event.type == pygame.KEYDOWN:
                print(event.key)
                if event.key == pygame.K_LEFT:
                    dx -= 0.1
                elif event.key == pygame.K_RIGHT:
                    dx += 0.1

                elif event.key == pygame.K_UP:
                    dy += 0.1

                elif event.key == pygame.K_DOWN:
                    dy -= 0.1

                elif event.key == pygame.K_i:
                    scale += 0.01
                
                elif event.key == pygame.K_u:
                    scale -= 0.01

        if not running: break

        if cnt == 0:
            cnt += 1
            model.StartMotion(live2d.MotionGroup.IDLE.value, 0, live2d.MotionPriority.IDLE.value)

        model.SetOffset(dx, dy)
        model.SetScale(scale)
        live2d.ClearBuffer()
        model.Update(*display)
        draw()

    # del model
    live2d.ReleaseCubism()

    del model2

    pygame.quit()
    quit()

if __name__ == "__main__":
    main()
