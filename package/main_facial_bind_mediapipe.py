import pygame
from pygame.locals import *
import live2d.v3 as live2d
from mediapipe_capture.capture_task import mediapipe_capture_task, Params
import threading as td

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

    model.LoadModelJson("../Resources/v3/Haru/Haru.model3.json")

    model.Resize(*display)

    running = True

    params = None

    while True:
        for event in pygame.event.get():
            if event.type == pygame.QUIT:
                running = False
                break
            if event.type == pygame.MOUSEBUTTONDOWN:
                x, y = pygame.mouse.get_pos()
                try:
                    model.Touch(x, y, s_call, f_call)
                except Exception as e:
                    print(e)

        if not running:
            break

        if not params:
            params = Params()
            td.Thread(None, mediapipe_capture_task, "Capture Task", (params,), daemon=True).start()

        model.CalcParameters()
        if params:
            # 面捕贴合程度取决于面部特征识别和参数计算算法
            model.SetParameterValue("ParamEyeLOpen", params.EyeLOpen, 1)
            model.SetParameterValue("ParamEyeROpen", params.EyeROpen, 1)
            model.SetParameterValue("ParamMouthOpenY", params.MouthOpenY, 1)
            model.SetParameterValue("ParamAngleX", params.AngleX, 1)
            model.SetParameterValue("ParamAngleY", params.AngleY, 1)
            model.SetParameterValue("ParamAngleZ", params.AngleZ, 1)
            model.SetParameterValue("ParamBodyAngleX", params.BodyAngleX, 1)

        live2d.clearBuffer()
        model.Update()
        draw()

    live2d.dispose()
    pygame.quit()
    quit()


if __name__ == "__main__":
    main()
