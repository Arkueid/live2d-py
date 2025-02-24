import os

import pygame
from pygame.locals import *

import live2d.v3 as live2d
# import live2d.v2 as live2d
from live2d.utils import log


import resources


def main():
    pygame.init()
    live2d.init()

    display = (300, 400)
    pygame.display.set_mode(display, DOUBLEBUF | OPENGL)
    pygame.display.set_caption("pygame window")

    live2d.glewInit()

    model = live2d.LAppModel()

    if live2d.LIVE2D_VERSION == 3:
        model.LoadModelJson(
            os.path.join(resources.RESOURCES_DIRECTORY, "v3/llny/llny.model3.json")
        )
    else:
        model.LoadModelJson(
            os.path.join(resources.RESOURCES_DIRECTORY, "v2/kasumi2/kasumi2.model.json")
        )

    model.Resize(*display)

    running = True

    def on_start_motion_callback(group: str, no: int):
        log.Info("start motion: [%s_%d]" % (group, no))

    def on_finish_motion_callback():
        log.Info("motion finished")


    while True:
        for event in pygame.event.get():
            if event.type == pygame.QUIT:
                running = False
                break
            if event.type == pygame.MOUSEBUTTONDOWN:
                model.StartRandomMotion(
                    onFinishMotionHandler=on_finish_motion_callback,
                    onStartMotionHandler=on_start_motion_callback
                )
        
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
