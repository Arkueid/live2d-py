import os
import sys

from PySide6.QtWidgets import QApplication

import app.settings as settings
from app import define
from config.configuration import Configuration
from ui.components.app_settings import AppSettings
from utils.model import Model, find_model_dir
from ui.view.scene import Scene
from ui.view.settings import Settings
from ui.view.systray import Systray
from ui.components.model_settings import ModelSettings

from app import live2d

from qfluentwidgets import qconfig


class Application(Systray.CallbackSet, AppSettings.CallBackSet, ModelSettings.CallbackSet):

    app: QApplication

    systray: Systray

    scene: Scene

    model: Model

    config: Configuration

    settings: Settings

    def __init__(self) -> None:
        self.app = QApplication(sys.argv)

        self.config = Configuration()

    def load_config(self):
        qconfig.load(settings.CONFIG_PATH, self.config)
        self.config.model_list.extend(find_model_dir(self.config.resource_dir.value))
        self.config.model3Json.load(os.path.join(
            self.config.resource_dir.value,
            self.config.model_name.value,
            self.config.model_name.value + define.MODEL_JSON_SUFFIX
        ))

    def save_config(self) -> None:
        self.config.save()

    def start(self) -> None:
        self.systray.start()
        self.scene.start()

        self.app.exec()

    def exit(self) -> None:
        self.systray.hide()
        self.scene.hide()

        live2d.ReleaseCubism()

        self.app.exit()

    def setup(self):
        self.systray = Systray()
        self.scene = Scene()
        self.settings = Settings(self.config)
        self.model = Model()

        live2d.InitializeCubism()

        self.systray.setup(self.config, self)
        self.model.setup(self.config)
        self.scene.setup(self.config, self.model)
        self.settings.setup(self.config, self, self)

    def toggleCharacterVisibility(self):
        self.config.visible.value = not self.config.visible.value
        self.scene.show()

    def toggleEyeTracking(self):
        self.config.track_enable.value = not self.config.track_enable.value
        self.scene.setMouseTracking(self.config.track_enable.value)
        if not self.config.track_enable.value:
            self.model.onMouseMoved(self.scene.width() // 2, self.scene.height() // 2)

    def toggleClickTransparent(self):
        self.config.click_transparent.value = not self.config.click_transparent.value

    def lockWindow(self):
        self.config.enable = not self.config.enable

    def stickWindowToTop(self):
        self.config.stay_on_top.value = not self.config.stay_on_top.value
        self.scene.show()

    def openSettings(self):
        self.settings.show()

    def exitApplication(self):
        self.exit()

    def onChangeModel(self):
        self.model.load_model()
        self.config.model3Json.load(
            os.path.join(self.config.resource_dir.value,
                         self.config.model_name.value,
                         self.config.model_name.value + define.MODEL_JSON_SUFFIX)
        )

    def onModel3JsonChanged(self):
        self.config.model3Json.save()
        self.model.load_model()

    def onPlayMotion(self, group, no):
        self.model.model.StartMotion(group, no, live2d.MotionPriority.FORCE.value)

