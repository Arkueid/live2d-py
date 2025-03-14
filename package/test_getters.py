# test for v3
# GetExpressionIds
# GetMotionGroups

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
    pygame.display.set_mode(display, DOUBLEBUF | OPENGL)
    pygame.display.set_caption("pygame window")

    live2d.glewInit()

    model = live2d.LAppModel()

    model.LoadModelJson(
        os.path.join(resources.RESOURCES_DIRECTORY, "v3/Mao/Mao.model3.json")
    )

    model.Resize(*display)

    running = True

    model.SetRandomExpression()

    expIds = model.GetExpressionIds()
    motionGroups = model.GetMotionGroups()
    paramIds = model.GetParamIds()

    print(expIds)

    print("===")

    print(motionGroups)

    print("===")

    print(paramIds)

    while True:
        for event in pygame.event.get():
            if event.type == pygame.QUIT:
                running = False
                break

        if not running:
            break

        model.Update()
        paramIds = model.GetParamIds()
        expIds = model.GetExpressionIds()
        motionGroups = model.GetMotionGroups()

        live2d.clearBuffer()
        model.Draw()
        pygame.display.flip()
        pygame.time.wait(int(1000 / 60))
    # f.close()

    live2d.dispose()

    pygame.quit()
    quit()

if __name__ == "__main__":
    main()
