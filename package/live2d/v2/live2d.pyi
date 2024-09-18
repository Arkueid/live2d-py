from enum import Enum
from typing import Any

class MotionPriority(Enum):
    NONE = 0
    IDLE = 1
    NORMAL = 2
    FORCE = 3


class MotionGroup(Enum):
    IDLE = "Idle"
    TAP_HEAD = "TapHead"


class HitArea(Enum):
    HEAD = MotionGroup.TAP_HEAD.value


def init() -> None:
    """
    初始化
    """
    pass


def dispose() -> None:
    """
    释放
    """
    pass


def clearBuffer() -> None:
    """
    glClearColor(0.0, 0.0, 0.0, 0.0)
        
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT)
    
    glClearDepth(1.0)
    """
    pass


def setLogEnable(enable: bool):
    pass


class LAppModel:
    
    def __init__(self):
        pass
    
    def LoadModelJson(self, fileName: str | Any) -> None:
        pass

    def Resize(self, ww: int | Any, wh: int | Any) -> None:
        pass
    
    def Update(self) -> None:
        pass

    def Draw(self) -> None:
        pass

    def SetAutoEyeBlinkEnable(enable: bool) -> None:
        pass
    
    def StartMotion(self, group: str | Any, no: int | Any, priority: int | Any, onStartMotionHandler=None, onFinishMotionHandler=None) -> None:
        pass
    
    def StartRandomMotion(self, group: str | Any, priority: int | Any, onStartMotionHandler=None, onFinishMotionHandler=None) -> None:
        pass
    
    def SetExpression(self, expressionID: str | Any) -> None:
        pass
    
    def SetRandomExpression(self) -> None:
        pass
    
    def HitTest(self, x: float | Any, y: float | Any) -> str:
        pass
    
    def HasMocConsistencyFromFile(self, mocFileName: str | Any) -> bool:
        pass

    def Touch(self, x: float | Any, y: float | Any, onStartMotionHandler=None, onFinishMotionHandler=None) -> None:
        pass

    def Drag(self, x: float | Any, y: float | Any) -> None:
        pass

    def IsMotionFinished(self) -> bool:
        pass

    def SetOffset(self, dx: float | Any, dy: float | Any) -> None:
        pass

    def SetScale(self, scale: float | Any) -> None:
        pass

    def SetParameterValue(self, paramId: str, value: float, weight: float):
        pass

    def AddParameterValue(self, paramId: str, value: float):
        pass