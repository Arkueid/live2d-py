# 简易面捕示例

import pygame
from pygame.locals import *
import live2d.v3 as live2d
from mediapipe_capture.capture_task import mediapipe_capture_task
from facial_params import Params
import threading as td
import os
import resources
from live2d.v3.params import StandardParams

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

    display = (450, 700)
    pygame.display.set_mode(display, DOUBLEBUF | OPENGL)

    live2d.glewInit()
    live2d.setGLProperties()

    model = live2d.LAppModel()

    model.LoadModelJson(os.path.join(resources.RESOURCES_DIRECTORY, "v3/mianfeimox/llny.model3.json"))

    model.Resize(*display)

    running = True

    params = None

    while True:
        for event in pygame.event.get():
            if event.type == pygame.QUIT:
                running = False
                break
            if event.type == pygame.MOUSEBUTTONDOWN:
                print("set random expression")
                model.SetRandomExpression()

        if not running:
            break

        if not params:
            params = Params()
            td.Thread(None, mediapipe_capture_task, "Capture Task", (params,), daemon=True).start()

        model.Update()
        if params:
            # 较大程度的解決抖动问题，Params类中的smooth_factor控制平滑度
            params.update_params(params)
            # 面捕贴合程度取决于面部特征识别和参数计算算法
            model.SetParameterValue(StandardParams.ParamEyeLOpen, params.EyeLOpen, 1)
            model.SetParameterValue(StandardParams.ParamEyeROpen, params.EyeROpen, 1)
            model.SetParameterValue(StandardParams.ParamMouthOpenY, params.MouthOpenY, 1)
            model.SetParameterValue(StandardParams.ParamAngleX, params.AngleX, 1)
            model.SetParameterValue(StandardParams.ParamAngleY, params.AngleY, 1)
            model.SetParameterValue(StandardParams.ParamAngleZ, params.AngleZ, 1)
            model.SetParameterValue(StandardParams.ParamBodyAngleX, params.BodyAngleX, 1)
        
        # 去除水印
        model.SetParameterValue("Param14", 1, 1)

        live2d.clearBuffer()
        model.Draw()
        draw()

    live2d.dispose()
    pygame.quit()
    quit()


if __name__ == "__main__":
    main()
