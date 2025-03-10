# 简易面捕示例

import os
import threading as td

import pygame
import live2d.v3 as live2d
# import live2d.v2 as live2d
import resources
from facial_params import Params
if live2d.LIVE2D_VERSION == 3:
    from live2d.v3.params import StandardParams
elif live2d.LIVE2D_VERSION == 2:
    from live2d.v2.params import StandardParams

# from open_see_face.capture_task import open_see_face_task


live2d.setLogEnable(False)


def s_call(group, no):
    print(group, no)


def f_call():
    print("end")


def main():
    pygame.init()
    live2d.init()

    display = (450, 700)
    pygame.display.set_mode(display, pygame.DOUBLEBUF | pygame.OPENGL)

    live2d.glewInit()

    model = live2d.LAppModel()

    if live2d.LIVE2D_VERSION == 3:
        model.LoadModelJson(os.path.join(resources.RESOURCES_DIRECTORY, "v3/llny/llny.model3.json"))
    elif live2d.LIVE2D_VERSION == 2:
        model.LoadModelJson(os.path.join(resources.RESOURCES_DIRECTORY, "v2/kasumi2/kasumi2.model.json"))
    model.Resize(*display)

    param_list = model.GetParamIds()

    index_to_param = [
        param_list.index(StandardParams.ParamEyeLOpen),
        param_list.index(StandardParams.ParamEyeLOpen),
        param_list.index(StandardParams.ParamMouthOpenY),
        param_list.index(StandardParams.ParamAngleX),
        param_list.index(StandardParams.ParamAngleY),
        param_list.index(StandardParams.ParamAngleZ),
        param_list.index(StandardParams.ParamBodyAngleX),
        param_list.index("Param14")
    ]

    running = True

    # 提前导入有概率绘制不出 live2d
    from mediapipe_capture.capture_task import mediapipe_capture_task
    params = Params()
    td.Thread(None, mediapipe_capture_task, "Capture Task", (params,), daemon=True).start()

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

        if params:
            # 较大程度的解決抖动问题，Params类中的smooth_factor控制平滑度
            params.update_params(params)
            # 面捕贴合程度取决于面部特征识别和参数计算算法
            model.SetIndexParamValue(index_to_param[0], params.EyeLOpen, 1)
            model.SetIndexParamValue(index_to_param[1], params.EyeROpen, 1)
            model.SetIndexParamValue(index_to_param[2], params.MouthOpenY, 1)
            model.SetIndexParamValue(index_to_param[3], params.AngleX, 1)
            model.SetIndexParamValue(index_to_param[4], params.AngleY, 1)
            model.SetIndexParamValue(index_to_param[5], params.AngleZ, 1)
            model.SetIndexParamValue(index_to_param[6], params.BodyAngleX, 1)

        # 去除水印
        model.SetIndexParamValue(index_to_param[7], 1, 1)

        live2d.clearBuffer()
        model.Update()
        model.Draw()
        pygame.display.flip()
        pygame.time.wait(int(1000 / 60))

    live2d.dispose()
    pygame.quit()
    quit()


if __name__ == "__main__":
    main()
