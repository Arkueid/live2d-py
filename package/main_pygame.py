import os

import pygame
from pygame.locals import *

import live2d.v2 as live2d
import live2d.utils.log as log
from live2d.v2.params import Parameter
import resouces

live2d.setLogEnable(True)


def on_start_motion_callback(group: str, no: int):
    log.Info("start motion: [%s_%d]" % (group, no))

def on_finish_motion_callback():
    log.Info("motion finished")


def draw():
    pygame.display.flip()
    pygame.time.wait(10)


def main():
    pygame.init()
    pygame.mixer.init()
    live2d.init()

    display = (300, 300)
    pygame.display.set_mode(display, DOUBLEBUF | OPENGL)

    model = live2d.LAppModel()

    # 中文路径测试
    model.LoadModelJson(os.path.join(resouces.RESOURCES_DIRECTORY, "v2/户山香澄/kasumi2.model.json"))

    model.Resize(*display)

    running = True

    dx: float = 0.0
    dy: float = 0.0
    scale: float = 1.0

    # 关闭自动眨眼
    # model.SetAutoBlinkEnable(False)
    # 关闭自动呼吸
    # model.SetAutoBreathEnable(False)

    model.StartMotion("cry02", 0, live2d.MotionPriority.FORCE, onStartMotionHandler=None, onFinishMotionHandler=on_finish_motion_callback)

    # 打印参数
    for i in range(model.GetParameterCount()):
        param: Parameter = model.GetParameter(i)
        log.Debug(param.id, param.value, param.max, param.min, param.default)

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
                model.Drag(*pygame.mouse.get_pos())

        if not running:
            break

        model.Update()
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
