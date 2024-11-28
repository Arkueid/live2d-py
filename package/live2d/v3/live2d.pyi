from enum import Enum
from typing import Any
from .params import Parameter


class MotionPriority(Enum):
    """
    动作优先级
    """
    NONE = 0
    IDLE = 1
    NORMAL = 2
    FORCE = 3


class MotionGroup(Enum):
    """
    内置动作组
    """
    IDLE = "Idle"
    TAP_HEAD = "TapHead"


class HitArea(Enum):
    HEAD = MotionGroup.TAP_HEAD.value


def init() -> None:
    """
    初始化 Cubism Framework
    """
    ...


def dispose() -> None:
    """
    释放 Cubism Framework
    """
    ...


def glewInit() -> None:
    """
    基于 Glew 实现的 live2d, 使用模型前应初始化 Glew
    """
    ...


def setGLProperties() -> None:
    """
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    
    glEnable(GL_BLEND);

    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    """
    ...


def clearBuffer(r=0.0, g=0.0, b=0.0, a=0.0) -> None:
    """
    glClearColor(0.0, 0.0, 0.0, 0.0)

    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT)
    
    glClearDepth(1.0)
    """
    ...


def setLogEnable(enable: bool):
    ...


def logEnable() -> bool:
    ...


class LAppModel:
    """
    The LAppModel class provides a structured way to interact with Live2D models, 
    enabling you to load assets, update the model per frame, manage motions, set 
    expressions, and perform hit testing. 
    """

    def __init__(self):
        ...

    def LoadModelJson(self, fileName: str | Any) -> None:
        """
        Load Live2D model assets.
        
        :param fileName: Name of the model's JSON configuration file.
        """
        ...

    def Resize(self, ww: int | Any, wh: int | Any) -> None:
        """
        按照画布尺寸调整模型绘制大小，画布大小改变以及第一次加载时应调用
        :param ww: 画布宽度
        :param wh: 画布高度
        :return:
        """

    def Draw(self) -> None:
        """
        绘制模型
        """
        ...

    def StartMotion(self, group: str | Any, no: int | Any, priority: int | Any, onStartMotionHandler=None,
                    onFinishMotionHandler=None) -> None:
        """
        Start a specific motion for the model.
        
        :param group: The group name of the motion.
        :param no: The motion number within the group.
        :param priority: Priority of the motion. Higher priority motions can interrupt lower priority ones.
        :param onStartMotionHandler: Optional callback function that gets called when the motion starts.
        :param onFinishMotionHandler: Optional callback function that gets called when the motion finishes.
        """
        ...

    def StartRandomMotion(self, group: str | Any, priority: int | Any, onStartMotionHandler=None,
                          onFinishMotionHandler=None) -> None:
        """
        Start a random motion from a specified group.
        
        :param group: The group name of the motion.
        :param priority: Priority of the motion. Higher priority motions can interrupt lower priority ones.
        :param onFinishedMotionHandler: Optional callback function that gets called when the motion finishes.
        """
        ...

    def SetExpression(self, expressionID: str | Any) -> None:
        """
        Set a specific expression for the model.
        
        :param expressionID: Identifier for the expression to be set.
        """
        ...

    def SetRandomExpression(self) -> None:
        """
        Set a random expression for the model.
        """
        ...

    def HitTest(self, x: float | Any, y: float | Any) -> str:
        """
        Perform a hit test to determine if a specific area of the model has been clicked.
        
        :param hitAreaName: Name of the hit area to be tested.
        :param x: X coordinate of the click.
        :param y: Y coordinate of the click.
        :return: The hit area name if a hit is detected, otherwise an empty string.
        """
        ...

    def HasMocConsistencyFromFile(self, mocFileName: str | Any) -> bool:
        """
        Check if the model's MOC file is consistent.
        
        :param mocFileName: Name of the MOC file to check.
        :return: True if the MOC file is consistent, otherwise False.
        """
        ...

    def Touch(self, x: float | Any, y: float | Any, onStartMotionHandler=None, onFinishMotionHandler=None) -> None:
        """
        :param x: global_mouse_x - window_x
        :param y: global_mouse_y - window_y
        """
        ...

    def Drag(self, x: float | Any, y: float | Any) -> None:
        """
        :param x: global_mouse_x - window_x
        :param y: global_mouse_y - window_y
        """
        ...

    def IsMotionFinished(self) -> bool:
        """
        当前正在播放的动作是否已经结束
        :return:
        """
        ...

    def SetOffset(self, dx: float | Any, dy: float | Any) -> None:
        """
        设置模型中心坐标的偏移量
        :param dx:
        :param dy:
        :return:
        """
        ...

    def SetScale(self, scale: float | Any) -> None:
        """
        设置模型缩放比例
        :param scale: 缩放比例
        :return:
        """
        ...

    def SetParameterValue(self, paramId: str, value: float, weight: float) -> None:
        """
        为对应控制参数设置具体值  
        调用时机: 在 CalcParameters 后，在 Update 之前
        :param paramId: live2d 模型内嵌的参数，详细见 live2d.v3.params.StandardParams
        :param value: 所有可操作参数见官方文档: https://docs.live2d.com/en/cubism-editor-manual/standard-parameter-list/
        :param weight: 当前传入的值和原值的比例，最终值=原值*(1-weight)+传入值*weight
        """
        ...

    def AddParameterValue(self, paramId: str, value: float) -> None:
        """
        最终值=原值+value
        """
        ...

    def Update(self) -> None:
        """
        初始化呼吸、动作、姿势、表情、各部分透明度等必要的参数值
        """
        ...

    def SetAutoBreathEnable(self, enable: bool) -> None:
        """
        开启自动呼吸
        """
        ...

    def SetAutoBlinkEnable(self, enable: bool) -> None:
        """
        开启自动眨眼
        """
        ...

    def GetParameterCount(self) -> int:
        ...

    def GetParameter(self, index: int) -> Parameter:
        ...

    def GetPartCount(self) -> int:
        ...

    def GetPartId(self, index: int) -> str:
        ...

    def GetPartIds(self) -> list[str]:
        ...

    def SetPartOpacity(self, index: int, opacity: float) -> None:
        ...

    def HitPart(self, x: float, y: float, topOnly: bool = False) -> list[str]:
        """
        
        :param x: 屏幕坐标 x
        :param y: 屏幕坐标 y
        :param topOnly: 只返回最顶部的 part id
        :return: part id 列表
        """
        ...
