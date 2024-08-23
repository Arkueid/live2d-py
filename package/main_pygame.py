import os

import pygame
from pygame.locals import *

import live2d.v3 as live2d
import live2d.utils.log as log
from live2d.utils.lipsync import WavHandler
from live2d.v3.params import StandardParams, Parameter
import resources


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

    # 加载中文路径模型
    model.LoadModelJson(
        os.path.join(
            resources.RESOURCES_DIRECTORY, "v3/波奇酱2.0/波奇酱2.0.model3.json"
        )
    )

    model.Resize(*display)

    running = True

    dx: float = 0.0
    dy: float = 0.0
    scale: float = 1.0

    # 关闭自动眨眼
    # model.SetAutoBlinkEnable(False)
    # 关闭自动呼吸
    # model.SetAutoBreathEnable(False)

    wavHandler = WavHandler()
    lipSyncN = 2.5

    audioPlayed = False

    def on_start_motion_callback(group: str, no: int):
        log.Info("start motion: [%s_%d]" % (group, no))
        audioPath = os.path.join(resources.CURRENT_DIRECTORY, "audio2.wav")
        pygame.mixer.music.load(audioPath)
        pygame.mixer.music.play()
        log.Info("start lipSync")
        wavHandler.Start(audioPath)


    def on_finish_motion_callback():
        log.Info("motion finished")


    # 获取全部可用参数
    for i in range(model.GetParameterCount()):
        param: Parameter = model.GetParameter(i)
        log.Debug(param.id, param.type, param.value, param.max, param.min, param.default)

    while True:
        for event in pygame.event.get():
            if event.type == pygame.QUIT:
                running = False
                break
            if event.type == pygame.MOUSEBUTTONDOWN:
                x, y = pygame.mouse.get_pos()
                model.Touch(x, y, on_start_motion_callback, on_finish_motion_callback)

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

            if event.type == pygame.MOUSEMOTION:
                # 实现拖拽
                model.Drag(*pygame.mouse.get_pos())

        if not running:
            break

        model.Update()
        if wavHandler.Update():
            # 利用 wav 响度更新 嘴部张合
            model.AddParameterValue(StandardParams.ParamMouthOpenY, wavHandler.GetRms() * lipSyncN)

        if not audioPlayed:
            # 播放一个不存在的动作
            model.StartMotion("", 0, live2d.MotionPriority.FORCE.value, on_start_motion_callback, on_finish_motion_callback)
            audioPlayed = True
        
        # 一般通过设置 param 去除水印
        # model.SetParameterValue("Param261", 1, 1)

        model.SetOffset(dx, dy)
        model.SetScale(scale)
        live2d.clearBuffer()
        model.Draw()
        draw()

    live2d.dispose()

    pygame.quit()
    quit()




if __name__ == "__main__":
    main()
