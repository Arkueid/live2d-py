from abc import ABC, abstractmethod
from facial_capture.facial_params import FacialParams


class OnCapturedListener(ABC):
    """
    监听器，用于监听每帧捕获的面部参数
    """

    @abstractmethod
    def onCaptured(self, params: FacialParams):
        pass
