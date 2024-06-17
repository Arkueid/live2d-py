import os

from PySide2.QtCore import Qt
from PySide2.QtGui import QIcon
from PySide2.QtWidgets import QApplication

from config import Configuration
from ui.components.api_settings import ApiSettings
from ui.components.app_settings import AppSettings
from ui.components.model_settings import ModelSettings

from qfluentwidgets import FluentWindow


class Settings(FluentWindow):
    appSettings: AppSettings
    modelSettings: ModelSettings
    apiSettings: ApiSettings

    config: Configuration

    def __init__(self, config: Configuration):
        super().__init__()
        self.resource_dir = config.resource_dir.value
        self.setWindowFlags(self.windowFlags() | Qt.WindowType.FramelessWindowHint | Qt.WindowType.Tool)
        self.appSettings = AppSettings(config)
        self.modelSettings = ModelSettings(config)
        self.addSubInterface(self.appSettings, self.icon("app_settings.svg"), "应用设置")
        self.addSubInterface(self.modelSettings, self.icon("model_settings.svg"), "模型设置")
        self.setMinimumSize(700, 500)

    def setup(self,
              config: Configuration,
              as_callback_set: AppSettings.CallBackSet,
              ms_callback_set: ModelSettings.CallbackSet):

        self.config = config

        self.appSettings.setup(as_callback_set)

        self.modelSettings.setup(ms_callback_set)

    def show(self):
        self.hide()
        size = QApplication.primaryScreen().size()
        self.move(size.width() // 2 - self.width() // 2, size.height() // 2 - self.height() // 2)
        self.setVisible(True)
        self.adjustSize()
        self.setMicaEffectEnabled(True)

    def icon(self, path):
        return QIcon(os.path.join(self.resource_dir, path))
