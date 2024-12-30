from abc import ABC, abstractmethod

from ..type import Float32Array


class DrawParam(ABC):
    DEFAULT_FIXED_TEXTURE_COUNT = 32
    CLIPPING_PROCESS_NONE = 0
    CLIPPING_PROCESS_OVERWRITE_ALPHA = 1
    CLIPPING_PROCESS_MULTIPLY_ALPHA = 2
    CLIPPING_PROCESS_DRAW = 3
    CLIPPING_PROCESS_CLEAR_ALPHA = 4

    def __init__(self):
        self.fixedTextureCount = DrawParam.DEFAULT_FIXED_TEXTURE_COUNT
        self.baseAlpha = 1
        self.baseRed = 1
        self.baseGreen = 1
        self.baseBlue = 1
        self.culling = False
        self.matrix4x4 = Float32Array(16)
        self.preMultipliedAlpha = False
        self.anisotropy = 0
        self.clippingProcess = DrawParam.CLIPPING_PROCESS_NONE
        self.clipBufPre_clipContextMask = None
        self.clipBufPre_clipContextDraw = None
        self.channel_colors = {}

    def setChannelFlagAsColor(self, aH, aI):
        self.channel_colors[aH] = aI

    def getChannelFlagAsColor(self, aY):
        return self.channel_colors[aY]

    @abstractmethod
    def setupDraw(self):
        pass

    @abstractmethod
    def drawTexture(self, texNo, screenColor, indexArray, vertexArray, uvArray, opacity, comp, multiplyColor):
        pass

    def setCulling(self, aH):
        self.culling = aH

    def setMatrix(self, aH):
        for aI in range(0, 16, 1):
            self.matrix4x4[aI] = aH[aI]

    def getMatrix(self):
        return self.matrix4x4

    def setPreMultipliedAlpha(self, aH):
        self.preMultipliedAlpha = aH

    def isPreMultipliedAlpha(self):
        return self.preMultipliedAlpha

    def setAnisotropy(self, aH):
        self.anisotropy = aH

    def getAnisotropy(self):
        return self.anisotropy

    def getClippingProcess(self):
        return self.clippingProcess

    def setClippingProcess(self, aH):
        self.clippingProcess = aH

    def setClipBufPre_clipContextForMask(self, aH):
        self.clipBufPre_clipContextMask = aH

    def getClipBufPre_clipContextMask(self):
        return self.clipBufPre_clipContextMask

    def setClipBufPre_clipContextForDraw(self, aH):
        self.clipBufPre_clipContextDraw = aH

    def getClipBufPre_clipContextDraw(self):
        return self.clipBufPre_clipContextDraw
