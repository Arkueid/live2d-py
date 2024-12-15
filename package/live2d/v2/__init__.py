from .core import Live2D, Live2DGLWrapper, log
from .framework import Live2DFramework
from .lapp_define import MotionGroup, MotionPriority, HitArea
from .lapp_model import LAppModel
from .params import Parameter, StandardParams
from .platform_manager import PlatformManager


def init():
    Live2D.init()
    Live2DFramework.setPlatformManager(PlatformManager())


def clearBuffer(r=0.0, g=0.0, b=0.0, a=0.0):
    Live2DGLWrapper.clearColor(r, g, b, a)
    Live2DGLWrapper.clear(Live2DGLWrapper.COLOR_BUFFER_BIT)


def setLogEnable(enable: bool):
    log.setLogEnable(enable)


def logEnable() -> bool:
    return log.logEnable()


def glewInit():
    pass


def setGLProperties():
    pass


def dispose():
    pass


LIVE2D_VERSION = 2

__all__ = ['LAppModel', 'MotionPriority', 'MotionGroup', "HitArea", "StandardParams"]
