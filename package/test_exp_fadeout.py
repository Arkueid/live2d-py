import os

import pygame
from pygame.locals import *

import live2d.v3 as live2d
# import live2d.v2 as live2d
from live2d.utils import log


import resources
import random


def main():
    pygame.init()
    live2d.init()

    display = (300, 400)
    pygame.display.set_mode(display, DOUBLEBUF | OPENGL, vsync=0)
    pygame.display.set_caption("pygame window")

    live2d.glewInit()

    model = live2d.LAppModel()

    if live2d.LIVE2D_VERSION == 3:
        model.LoadModelJson(
            os.path.join(resources.RESOURCES_DIRECTORY, "v3/Mao/Mao.model3.json")
        )
    else:
        model.LoadModelJson(
            os.path.join(resources.RESOURCES_DIRECTORY, "v2/kasumi2/kasumi2.model.json")
        )

    model.Resize(*display)

    running = True

    expIds = model.GetExpressionIds()

    activeExpIds = set()

    model.SetRandomExpression() # 默认表情设置为 exp_03
    while True:
        for event in pygame.event.get():
            if event.type == pygame.QUIT:
                running = False
                break
            if event.type == pygame.MOUSEBUTTONDOWN:
                # model.SetExpression("exp_04", fadeout=5000)
                # expId = model.SetRandomExpression(fadeout=5000)
                expId = model.SetRandomExpression(fadeout=5000)
                print("random exp:", expId)
            elif event.type == pygame.KEYDOWN:
                if event.key == pygame.K_r:
                    model.ResetExpression()
                elif event.key == pygame.K_a:
                    e = random.choice(expIds)
                    model.AddExpression(e)
                    activeExpIds.add(e)
                elif event.key == pygame.K_s:
                    model.ResetExpressions()
                elif event.key == pygame.K_d:
                    if len(activeExpIds) > 0:
                        e = random.choice(list(activeExpIds))
                        model.RemoveExpression(e)
                        activeExpIds.remove(e)
        
        if not running:
            break

        model.Update()
        live2d.clearBuffer(1.0, 0.0, 0.0, 0.0)
        model.Draw()
        pygame.display.flip()
        pygame.time.wait(10)

    live2d.dispose()

    pygame.quit()
    quit()


if __name__ == "__main__":
    main()
