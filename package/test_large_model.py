import os
import pygame
from pygame.locals import *

import live2d.v3 as live2d

import resources

live2d.setLogEnable(True)


def main():
    pygame.init()
    pygame.mixer.init()
    live2d.init()

    display = (300, 400)
    pygame.display.set_mode(display, DOUBLEBUF | OPENGL, vsync=1)
    pygame.display.set_caption("pygame window")

    live2d.glewInit()

    model = live2d.LAppModel()

    model.LoadModelJson(
        os.path.join(resources.RESOURCES_DIRECTORY, "v3/magic/magic.model3.json")
    )

    model.Resize(*display)

    running = True

    dx: float = 0.0
    dy: float = 0.0
    scale: float = 1.0

    model.SetRandomExpression()

    while True:
        for event in pygame.event.get():
            if event.type == pygame.QUIT:
                running = False
                break
            if event.type == pygame.MOUSEBUTTONDOWN:
                model.StartRandomMotion(onStartMotionHandler=lambda g, n: print(g, n))

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
                
                elif event.key == pygame.K_r:
                    model.StopAllMotions()
                    model.ResetPose()
                
                elif event.key == pygame.K_e:
                    model.ResetExpression()

            if event.type == pygame.MOUSEMOTION:
                # 实现拖拽
                model.Drag(*pygame.mouse.get_pos())

        if not running:
            break

        model.Update()
        # 去除水印
        model.SetParameterValue("ParamEyeHeart3", 1, 1)

        model.SetOffset(dx, dy)
        model.SetScale(scale)
        live2d.clearBuffer(0.0, 0.0, 0.0, 0.0)
        model.Draw()
        pygame.display.flip()
        pygame.time.wait(10)

    live2d.dispose()

    pygame.quit()
    quit()


if __name__ == "__main__":
    currentTopClickedPartId = None
    main()
