import pygame
from pygame.locals import *
import live2d.v3 as live2d
from facial_capture.capture_task import facial_capture_task, FacialParams, OnCapturedListener
import threading as td

live2d.setLogEnable(False)


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
    capture_task_running = False

    class MyListener(OnCapturedListener):

        def __init__(self, params: FacialParams):
            self.params = params

        def onCaptured(self, params: FacialParams):
            self.params = params

    listener = None

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

        if not capture_task_running:
            listener = MyListener(FacialParams())
            td.Thread(None, facial_capture_task, "Capture Task", (listener, ), daemon=True).start()
            capture_task_running = True

        model.CalcParameters()
        if listener:
            model.SetParamValue("ParamEyeLOpen", listener.params.paramEyeLOpen, 1)
            model.SetParamValue("ParamEyeROpen", listener.params.paramEyeROpen, 1)
        live2d.clearBuffer()
        model.Update()
        draw()

    live2d.dispose()
    pygame.quit()
    quit()


if __name__ == "__main__":
    main()
