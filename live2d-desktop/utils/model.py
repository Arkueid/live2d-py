import os
import time

from app import live2d, define
from config.configuration import Configuration
from ui.view.scene import Scene


def find_model_dir(path: str) -> list[str]:
    ls: list[str] = list()
    dirs = os.listdir(path)
    for i in dirs:

        if i == '.' or i == '..':
            continue

        dirName = os.path.join(path, i)
        if not os.path.isdir(dirName):
            continue

        modelJson = os.path.join(dirName, i + define.MODEL_JSON_SUFFIX)
        if os.path.exists(modelJson):
            ls.append(i)
    return ls


class Model(Scene.CallBackSet):

    def onInitialize(self):
        self.initialize = True
        self.load_model()

    def onUpdate(self, ww: int, wh: int):
        self.model.SetScale(self.config.scale.value)
        self.model.SetOffset(self.config.drawX.value, self.config.drawY.value)
        live2d.ClearBuffer()
        self.model.Update(ww, wh)

    def onResize(self, ww: int, wh: int):
        self.model.Resize(ww, wh)

    def onTouch(self, rx: int, ry: int):
        self.model.Touch(rx, ry)

    def onMouseMoved(self, mx: int, my: int):
        self.model.Drag(mx, my)

    def onIntervalReached(self):
        self.finished = False
        self.model.StartRandomMotion(live2d.MotionGroup.IDLE.value, live2d.MotionPriority.IDLE.value, self.set_finished)

    def IsFinished(self):
        return self.finished

    config: Configuration
    model: live2d.LAppModel | None
    finished: bool
    initialize: bool

    def __init__(self):
        self.model = None
        self.finished = True
        self.initialize = False

    def setup(self, config: Configuration):
        self.config = config

    def load_model(self):
        if not self.initialize:
            return

        if self.model is not None:
            del self.model
        self.model = live2d.LAppModel()
        self.model.LoadAssets(
            os.path.join(self.config.resource_dir.value, self.config.model_name.value),
            self.config.model_name.value + define.MODEL_JSON_SUFFIX)

        self.finished = True

    def set_finished(self):
        self.finished = True
        info = time.strftime("[INFO  %Y-%m-%d %H:%M:%S] motion finished", time.localtime(time.time()))
        print(info)
