import os

import pygame
from pygame.locals import *

import live2d.v3 as live2d
import resouces
from wav_handler import WavHandler

# import live2d.v2 as live2d

live2d.setLogEnable(True)

model: live2d.LAppModel


def draw():
    pygame.display.flip()
    pygame.time.wait(10)


def main():
    global model

    pygame.init()
    pygame.mixer.init()
    live2d.init()

    wavHandler = WavHandler()

    display = (700, 500)
    pygame.display.set_mode(display, DOUBLEBUF | OPENGL)

    if live2d.LIVE2D_VERSION == 3:
        live2d.glewInit()
        live2d.setGLProperties()

    model = live2d.LAppModel()

    model = live2d.LAppModel()
    if live2d.LIVE2D_VERSION == 3:
        model.LoadModelJson(os.path.join(resouces.RESOURCES_DIRECTORY, "v3/Haru/Haru.model3.json"))
    else:
        model.LoadModelJson(os.path.join(resouces.RESOURCES_DIRECTORY, "v2/kasumi2/kasumi2.model.json"))

    model.Resize(*display)

    running = True

    dx: float = 0.0
    dy: float = 0.0
    scale: float = 1.0

    cnt = 0

    # 关闭内置口型同步
    model.SetLipSyncEnable(False)

    # 关闭自动眨眼
    # model.SetAutoBlinkEnable(False)
    # 关闭自动呼吸
    # model.SetAutoBreathEnable(False)

    def start_callback(group, no):
        print("start lipsync")
        pygame.mixer.music.load("audio1.wav")
        pygame.mixer.music.play()
        wavHandler.Start("audio1.wav")

    lipSyncN = 3

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

            if event.type == pygame.MOUSEMOTION:
                model.Drag(*pygame.mouse.get_pos())

        if not running:
            break

        model.CalcParameters()
        if cnt < 1:
            cnt += 1
            model.StartMotion("Speak", 0, live2d.MotionPriority.FORCE.value, start_callback)

        if wavHandler.Update():  # 当前音频仍在播放
            model.SetParameterValue("ParamMouthOpenY", wavHandler.GetRms() * lipSyncN, 1)

        model.SetOffset(dx, dy)
        model.SetScale(scale)
        live2d.clearBuffer()
        model.Update()
        draw()

    live2d.dispose()

    pygame.quit()
    quit()


if __name__ == "__main__":
    main()
