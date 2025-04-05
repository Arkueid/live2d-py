import os
import pygame
import live2d.v3 as live2d
import resources
from live2d.utils.lipsync import WavHandler
from live2d.v3.params import StandardParams


def main():
    pygame.init()
    live2d.init()

    display = (500, 700)
    pygame.display.set_mode(display, pygame.DOUBLEBUF | pygame.OPENGL)
    pygame.display.set_caption("pygame window")

    live2d.glewInit()

    model = live2d.LAppModel()

 
    model.LoadModelJson(
        os.path.join(resources.RESOURCES_DIRECTORY, "v3/nn/nn.model3.json")
    )


    model.Resize(*display)

    modelSoundDir = os.path.join(resources.RESOURCES_DIRECTORY, "v3/nn")

    wavHandler = WavHandler()
    def startMotionHandler(group: str, no: int):
        print("start motion: [%s_%d]" % (group, no))
        sp = os.path.join(modelSoundDir, model.GetSoundPath(group, no))
        if os.path.exists(sp):
            pygame.mixer.music.load(sp)
            pygame.mixer.music.play()
            wavHandler.Start(sp)
            print("start sound %s" % sp)


    running = True
    while True:
        for event in pygame.event.get():
            if event.type == pygame.QUIT:
                running = False
                break
            elif event.type == pygame.MOUSEMOTION:
                model.Drag(*pygame.mouse.get_pos())
            elif event.type == pygame.MOUSEBUTTONUP:
                # model.SetRandomExpression()
                model.StartRandomMotion(onStartMotionHandler=startMotionHandler)
        
        if not running:
            break

        live2d.clearBuffer()
        model.Update()
        if wavHandler.Update():
            model.SetParameterValue(StandardParams.ParamMouthOpenY, wavHandler.GetRms() * 9)
        model.Draw()

        pygame.display.flip()

    live2d.dispose()

    pygame.quit()

if __name__ == "__main__":
    main()
