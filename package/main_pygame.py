import pygame
from pygame.locals import *
import live2d.v3 as live2d
# import live2d.v2 as live2d

live2d.setLogEnable(True)

model: live2d.LAppModel

def draw():
    pygame.display.flip()
    pygame.time.wait(10)


def s_call(group, no):
    print(group, no)


def f_call():
    global model

    print("end")
    
    model.SetParameterValue("ParamAngleX", 30, 1.)


def main():
    global model

    pygame.init()
    live2d.init()

    display = (700,500)
    pygame.display.set_mode(display, DOUBLEBUF|OPENGL)

    if live2d.LIVE2D_VERSION == 3:
        live2d.glewInit()
        live2d.setGLProperties()

    model = live2d.LAppModel()

    model = live2d.LAppModel()
    if live2d.LIVE2D_VERSION == 3:
        model.LoadModelJson("../Resources/v3/Haru/Haru.model3.json")
    else:
        model.LoadModelJson("../Resources/v2/kasumi2/kasumi2.model.json")

    model.Resize(*display)

    running = True

    dx: float = 0.0
    dy: float = 0.0
    scale: float = 1.0

    cnt = 0
    
    model.SetAutoBlinkEnable(False)
    model.SetAutoBreathEnable(False)

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

        if not running: break

        model.CalcParameters()
        if cnt < 1000 / 30 * 5:
            cnt += 1
            # model.StartMotion(live2d.MotionGroup.IDLE.value, 0, live2d.MotionPriority.IDLE.value, None, f_call)
            model.AddParameterValue("ParamAngleX", 30)
        

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
