
import pygame
from pygame.locals import *

import live2d.v2 as live2d

live2d.setLogEnable(True)

def draw():
    pygame.display.flip()
    pygame.time.wait(10)

def s_call(group, no):
    print(group, no)

def f_call():
    print("end")

def main():
    pygame.init()
    live2d.init()

    display = (400,300)
    pygame.display.set_mode(display, OPENGL | DOUBLEBUF)

    model = live2d.LAppModel()

    model2 = live2d.LAppModel()

    del model

    model = live2d.LAppModel()
    model.LoadModelJson("./Resources/v2/kasumi2/kasumi2.model.json")

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
            if event.type == pygame.MOUSEMOTION:
                x, y = pygame.mouse.get_pos()
                model.Drag(x, y)
            if event.type == pygame.MOUSEBUTTONDOWN:
                x, y = pygame.mouse.get_pos()
                try:
                    model.Touch(x, y, s_call, f_call)
                except Exception as e:
                    print(e)

            if event.type == pygame.KEYDOWN:
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
            model.StartMotion("x", 0, live2d.MotionPriority.IDLE.value, s_call, f_call)

        model.SetOffset(dx, dy)
        model.SetScale(scale)
        live2d.clearBuffer()
        model.Update()
        draw()

    # del model
    live2d.dispose()

    del model2

    pygame.quit()
    quit()

if __name__ == "__main__":
    main()