# 口型同步示例

import os

import pygame
from pygame.locals import *

import live2d.v3 as live2d
import resources
from live2d.utils.lipsync import WavHandler
from live2d.utils.log import Info
from live2d.v3.params import StandardParams


live2d.setLogEnable(True)


def draw():
    pygame.display.flip()
    pygame.time.wait(10)


def main():
    pygame.init()
    pygame.mixer.init()
    live2d.init()

    display = (700, 500)
    pygame.display.set_mode(display, DOUBLEBUF | OPENGL)

    live2d.glewInit()
    live2d.setGLProperties()

    model = live2d.LAppModel()
    wavHandler = WavHandler()

    model.LoadModelJson(
        os.path.join(resouces.RESOURCES_DIRECTORY, "v3/Haru/Haru.model3.json")
    )

    model.Resize(*display)

    running = True

    cnt = 0

    # 可以通过 group 和 no 获取 model3.json 中定义的音频路径
    # 其他外部生成的音频路径直接传入即可
    def start_callback(group, no):
        pygame.mixer.music.load(os.path.join(resouces.CURRENT_DIRECTORY, "audio2.wav"))
        pygame.mixer.music.play()
        Info("start lipsync")
        wavHandler.Start(os.path.join(resouces.CURRENT_DIRECTORY, "audio2.wav"))

    lipSyncN = 2

    while running:
        for event in pygame.event.get():
            if event.type == pygame.QUIT:
                running = False
                break

        model.Update()
        if cnt < 1:
            cnt += 1
            model.StartMotion(
                "Speak", 0, live2d.MotionPriority.FORCE.value, start_callback
            )

        if wavHandler.Update():  # 当前音频仍在播放
            model.AddParameterValue(StandardParams.ParamMouthOpenY, wavHandler.GetRms() * lipSyncN)

        live2d.clearBuffer()
        model.Draw()
        draw()

    live2d.dispose()

    pygame.quit()
    quit()


if __name__ == "__main__":
    main()
