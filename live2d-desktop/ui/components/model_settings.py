from abc import abstractmethod, ABC

from ui.components.design.model_settings_design import ModelSettingsDesign
from utils.model3json import MotionGroups

from config.configuration import Configuration


class ModelSettings(ModelSettingsDesign):
    class CallbackSet(ABC):

        @abstractmethod
        def onChangeModel(self):
            pass

        @abstractmethod
        def onModel3JsonChanged(self):
            pass

        @abstractmethod
        def onPlayMotion(self, group, idx):
            pass

    motion_groups: MotionGroups
    callBackSet: CallbackSet
    config: Configuration

    def __init__(self, config: Configuration):
        super().__init__(config)
        self.config = config

        self.setObjectName("model_settings")

    def setup(self, callbackSet: CallbackSet):
        self.callBackSet = callbackSet
        self.config.model_name.valueChanged.connect(self.callBackSet.onChangeModel)
        self.btn_save.pressed.connect(self.callBackSet.onModel3JsonChanged)
        self.motionEditor.playMotionFunc = self.callBackSet.onPlayMotion


