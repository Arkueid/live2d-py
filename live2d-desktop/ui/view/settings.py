from abc import ABC, abstractmethod

from PySide6.QtCore import Qt
from PySide6.QtWidgets import QApplication
from qfluentwidgets import FluentWindow, FluentIcon

from config import Configuration
from ui.components.api_settings import ApiSettings
from ui.components.app_settings import AppSettings
from ui.components.model_settings import ModelSettings


class Settings(FluentWindow):
    appSettings: AppSettings
    modelSettings: ModelSettings
    apiSettings: ApiSettings

    config: Configuration

    def __init__(self, config: Configuration):
        super().__init__()
        self.setWindowFlags(Qt.WindowType.Tool | Qt.WindowType.FramelessWindowHint)
        self.appSettings = AppSettings(config)
        self.modelSettings = ModelSettings(config)
        self.addSubInterface(self.appSettings, FluentIcon.APPLICATION, "应用设置")
        self.addSubInterface(self.modelSettings, FluentIcon.APPLICATION, "模型设置")
        self.setMinimumSize(700, 500)

    def setup(self,
              config: Configuration,
              as_callback_set: AppSettings.CallBackSet,
              ms_callback_set: ModelSettings.CallbackSet):

        self.config = config

        self.appSettings.setup(as_callback_set)

        self.modelSettings.setup(ms_callback_set)

    def show(self):
        size = QApplication.primaryScreen().size()
        self.move(size.width() // 2 - self.width() // 2, size.height() // 2 - self.height() // 2)
        self.setVisible(True)
        self.adjustSize()
