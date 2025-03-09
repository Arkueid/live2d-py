# 测试 v3 SetParameterValue 是否正确
# 测试 v3 SetParameterValue 是否能在 Update 和 Draw 之外的地方调用后起作用

import os
import pygame
import live2d.v3 as live2d
import resources
from live2d.v3.params import StandardParams


def main():
    pygame.init()
    live2d.init()

    display = (300, 400)
    pygame.display.set_mode(display, pygame.DOUBLEBUF | pygame.OPENGL)
    pygame.display.set_caption("pygame window")

    live2d.glewInit()

    model = live2d.LAppModel()

    model.LoadModelJson(
        os.path.join(resources.RESOURCES_DIRECTORY, "v3/Haru/Haru.model3.json")
    )

    model.Resize(*display)

    model.SetAutoBlinkEnable(False) # 防止影响测试

    # 左眼闭上
    model.SetParameterValue(StandardParams.ParamEyeLOpen, 0)
    # 右眼睁开
    model.SetParameterValue(StandardParams.ParamEyeROpen, 1)
    # 张嘴
    model.SetParameterValue(StandardParams.ParamMouthOpenY, 1)

    running = True
    while True:
        for event in pygame.event.get():
            if event.type == pygame.QUIT:
                running = False
                break
            if event.type == pygame.MOUSEMOTION:
                model.Drag(*pygame.mouse.get_pos())
        
        if not running:
            break

        live2d.clearBuffer()
        model.Update()
        model.Draw()

        pygame.display.flip()

    live2d.dispose()

    pygame.quit()

if __name__ == "__main__":
    main()
