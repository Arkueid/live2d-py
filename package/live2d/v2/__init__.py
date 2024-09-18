from .live2d import *


class MotionPriority:
    NONE: int = 0
    IDLE: int = 1
    NORMAL: int = 2
    FORCE: int = 3


class MotionGroup:
    IDLE: str = "Idle"
    TAP_HEAD: str = "TapHead"


class HitArea:
    HEAD: str = MotionGroup.TAP_HEAD


LIVE2D_VERSION = 2
