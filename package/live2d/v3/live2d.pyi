from typing import Any
from .params import Parameter


def init() -> None:
    """
    initialize inner memory allocator for live2d models
    """
    ...


def dispose() -> None:
    """
    dispose Cubism Framework when no longer using live2d
    """
    ...


def glewInit() -> None:
    """
    initialize inner opengl functions
    """
    ...

def glInit() -> None:
    """
    initialize inner opengl functions
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
        adjust model canvas to window size
        :param ww: window width
        :param wh: window height
        :return:
        """
        ...

    def Draw(self) -> None:
        """
        update model shapes with the params set by `LAppModel.Update` and  `LAppModel.SetParameterValue`, and then render them 
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

    def StartRandomMotion(self, group: str | Any = None, priority: int | Any = 3, onStartMotionHandler=None,
                          onFinishMotionHandler=None) -> None:
        """
        Start a random motion from a specified group.
        
        :param group: The group name of the motion.
        :param priority: Priority of the motion. Higher priority motions can interrupt lower priority ones.
        :param onFinishedMotionHandler: Optional callback function that gets called when the motion finishes.
        """
        ...

    def SetExpression(self, expressionID: str | Any, fadeout=-1) -> None:
        """
        Set a specific expression for the model.
        
        :param expressionID: name of the expression to be set.
        """
        ...

    def SetRandomExpression(self, fadeout=-1) -> str:
        """
        Set a random expression for the model.
        """
        ...

    def HitTest(self, hitAreaName: str, x: float | Any, y: float | Any) -> bool:
        """
        to test if the current clicked area is the target `HitArea` defined in xxx.model3.json

        x, y are relative to the window topleft
        """
        ...

    def HasMocConsistencyFromFile(self, mocFileName: str | Any) -> bool:
        """
        Check if the model's MOC file is consistent.
        
        :param mocFileName: Name of the MOC file to check.
        :return: True if the MOC file is consistent, otherwise False.
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

    def Rotate(self, degrees: float) -> None:
        """
        counterclockwise when degrees > 0
        """

    def SetParameterValue(self, paramId: str, value: float, weight: float = 1.0) -> None:
        """
        为对应控制参数设置具体值  
        调用时机: 在 CalcParameters 后，在 Update 之前
        :param paramId: live2d 模型内嵌的参数，详细见 live2d.v3.params.StandardParams
        :param value: 所有可操作参数见官方文档: https://docs.live2d.com/en/cubism-editor-manual/standard-parameter-list/
        :param weight: 当前传入的值和原值的比例，最终值=原值*(1-weight)+传入值*weight
        """
        ...

    def SetIndexParamValue(self, index: int, value: float, weight: float = 1.0) -> None:
        ...

    def AddParameterValue(self, paramId: str, value: float) -> None:
        """
        最终值=原值+value
        """
        ...

    def AddIndexParamValue(self, index: int, value: float) -> None:
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
    
    def GetParamIds(self) -> list[str]:
        ...
    
    def GetParameterValue(self, index: int) -> float:
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

    def SetPartScreenColor(self, partIndex: int, r: float, g: float, b: float, a: float):
        ...
    
    def GetPartScreenColor(self, partIndex: int) -> tuple[float]:
        ...

    def SetPartMultiplyColor(self, partIndex: int, r: float, g: float, b: float, a: float):
        ...
    
    def GetPartMultiplyColor(self, partIndex: int) -> tuple[float]:
        ...

    def ResetExpression(self) -> None:
        """
        重置为默认表情
        """
        ...

    def StopAllMotions(self) -> None:
        ...

    def ResetParameters(self) -> None:
        ...

    def ResetPose(self) -> None:
        ...

    def GetExpressionIds(self) -> list[str]:
        ...

    def GetMotionGroups(self) -> dict[str, int]:
        ...

    def GetSoundPath(self, group: str, index: int) -> str:
        ...