from .id import Id
from .util import log
from .live2d_gl_wrapper import Live2DGLWrapper

class Live2D:
    L2D_OUTSIDE_PARAM_AVAILABLE = False
    __firstInit = True
    clippingMaskBufferSize = 256

    @staticmethod
    def init():
        if Live2D.__firstInit:
            Live2D.__firstInit = False

    @staticmethod
    def clearBuffer():
        Live2DGLWrapper.clearColor(0, 0, 0, 0)
        Live2DGLWrapper.clear(Live2DGLWrapper.COLOR_BUFFER_BIT)

    @staticmethod
    def dispose():
        Id.releaseStored()
