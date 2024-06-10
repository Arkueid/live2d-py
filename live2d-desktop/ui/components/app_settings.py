from abc import ABC, abstractmethod

from config.configuration import Configuration
from ui.components.design.app_settings_design import AppSettingsDesign


class AppSettings(AppSettingsDesign):
    class CallBackSet(ABC):
        pass

    config: Configuration
    callbackSet: CallBackSet

    def __init__(self, config: Configuration):
        super().__init__(config)
        self.config = config

        self.setObjectName("app_settings")

    def setup(self, callbackSet: CallBackSet):
        self.callbackSet = callbackSet

