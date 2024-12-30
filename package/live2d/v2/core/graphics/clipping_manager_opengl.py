﻿from typing import List, Optional

from .clip_context import ClipContext
from .clip_matrix import ClipMatrix
from .clip_rectf import ClipRectF
from .texture_info import TextureInfo
from ..DEF import VERTEX_STEP, VERTEX_OFFSET
from ..live2d import Live2D
from ..type import Array, Float32Array


class ClippingManagerOpenGL:
    CHANNEL_COUNT = 4

    def __init__(self, aJ):
        self.clipContextList: List[Optional[ClipContext]] = Array()
        self.glcontext = aJ.gl
        self.dpGL = aJ
        self.curFrameNo = 0
        self.firstError_clipInNotUpdate = True
        self.colorBuffer = 0
        self.isInitGLFBFunc: bool = False
        self.tmpBoundsOnModel = ClipRectF()
        self.genMaskRenderTexture()
        self.tmpModelToViewMatrix = ClipMatrix()
        self.tmpMatrix2 = ClipMatrix()
        self.tmpMatrixForMask = ClipMatrix()
        self.tmpMatrixForDraw = ClipMatrix()
        self.channelColors: List[Optional[TextureInfo]] = Array()
        aI = TextureInfo()
        aI.r = 0
        aI.g = 0
        aI.b = 0
        aI.a = 1
        self.channelColors.append(aI)
        aI = TextureInfo()
        aI.r = 1
        aI.g = 0
        aI.b = 0
        aI.a = 0
        self.channelColors.append(aI)
        aI = TextureInfo()
        aI.r = 0
        aI.g = 1
        aI.b = 0
        aI.a = 0
        self.channelColors.append(aI)
        aI = TextureInfo()
        aI.r = 0
        aI.g = 0
        aI.b = 1
        aI.a = 0
        self.channelColors.append(aI)
        for aH in range(0, len(self.channelColors), 1):
            self.dpGL.setChannelFlagAsColor(aH, self.channelColors[aH])

    def releaseShader(self):
        aI = len(Live2D.frameBuffers)
        for aH in range(0, aI, 1):
            self.dpGL.deleteFramebuffer(Live2D.frameBuffers[aH].framebuffer)

        Live2D.frameBuffers = []
        Live2D.__glContext = []

    def init(self, aO, aN, aL):
        for aM in range(0, len(aN), 1):
            aH = aN[aM].getClipIDList()
            if aH is None:
                continue

            aJ = self.findSameClip(aH)
            if aJ is None:
                aJ = ClipContext(self, aO, aH)
                self.clipContextList.append(aJ)

            aI = aN[aM].getId()
            aK = aO.getDrawDataIndex(aI)
            aJ.addClippedDrawData(aI, aK)
            aP = aL[aM]
            aP.clipBufPre_clipContext = aJ

    def genMaskRenderTexture(self):
        self.dpGL.createFramebuffer()

    def setupClip(self, a1, aQ):
        aK = 0
        for aO in range(0, len(self.clipContextList), 1):
            aP = self.clipContextList[aO]
            self.calcClippedDrawTotalBounds(a1, aP)
            if aP.isUsing:
                aK += 1

        if aK > 0:
            oldFbo = aQ.gl.getParameter(aQ.gl.FRAMEBUFFER_BINDING)
            rect = Array(4)
            rect[0] = 0
            rect[1] = 0
            rect[2] = aQ.gl.width
            rect[3] = aQ.gl.height
            aQ.gl.viewport(0, 0, Live2D.clippingMaskBufferSize, Live2D.clippingMaskBufferSize)
            self.setupLayoutBounds(aK)
            aQ.gl.bindFramebuffer(aQ.gl.FRAMEBUFFER, aQ.framebufferObject.framebuffer)
            aQ.gl.clearColor(0, 0, 0, 0)
            aQ.gl.clear(aQ.gl.COLOR_BUFFER_BIT)
            for aO in range(0, len(self.clipContextList), 1):
                aP = self.clipContextList[aO]
                aT = aP.allClippedDrawRect
                aV = aP.layoutBounds
                aJ = 0.05
                self.tmpBoundsOnModel.setRect(aT)
                self.tmpBoundsOnModel.expand(aT.width * aJ, aT.height * aJ)
                aZ = aV.width / self.tmpBoundsOnModel.width
                aY = aV.height / self.tmpBoundsOnModel.height
                self.tmpMatrix2.identity()
                self.tmpMatrix2.translate(-1, -1, 0)
                self.tmpMatrix2.scale(2, 2, 1)
                self.tmpMatrix2.translate(aV.x, aV.y, 0)
                self.tmpMatrix2.scale(aZ, aY, 1)
                self.tmpMatrix2.translate(-self.tmpBoundsOnModel.x, -self.tmpBoundsOnModel.y, 0)
                self.tmpMatrixForMask.setMatrix(self.tmpMatrix2.m)
                self.tmpMatrix2.identity()
                self.tmpMatrix2.translate(aV.x, aV.y, 0)
                self.tmpMatrix2.scale(aZ, aY, 1)
                self.tmpMatrix2.translate(-self.tmpBoundsOnModel.x, -self.tmpBoundsOnModel.y, 0)
                self.tmpMatrixForDraw.setMatrix(self.tmpMatrix2.m)
                aH = self.tmpMatrixForMask.getArray()
                for aX in range(0, 16, 1):
                    aP.matrixForMask[aX] = aH[aX]

                a0 = self.tmpMatrixForDraw.getArray()
                for aX in range(0, 16, 1):
                    aP.matrixForDraw[aX] = a0[aX]

                aS = len(aP.clippingMaskDrawIndexList)
                for aU in range(0, aS, 1):
                    aR = aP.clippingMaskDrawIndexList[aU]
                    aI = a1.getDrawData(aR)
                    aL = a1.getDrawContext(aR)
                    aQ.setClipBufPre_clipContextForMask(aP)
                    aI.draw(aQ, a1, aL)

            aQ.gl.bindFramebuffer(aQ.gl.FRAMEBUFFER, oldFbo)
            aQ.setClipBufPre_clipContextForMask(None)
            aQ.gl.viewport(rect[0], rect[1], rect[2], rect[3])

    def getColorBuffer(self):
        return self.colorBuffer

    def findSameClip(self, aK):
        for aN in range(0, len(self.clipContextList), 1):
            aO = self.clipContextList[aN]
            aH = len(aO.clipIDList)
            if aH != len(aK):
                continue

            aI = 0
            for aM in range(0, aH, 1):
                aL = aO.clipIDList[aM]
                for aJ in range(0, aH, 1):
                    if aK[aJ] == aL:
                        aI += 1
                        break

            if aI == aH:
                return aO

        return None

    def calcClippedDrawTotalBounds(self, a6, aV):
        aU = a6.model.getModelImpl().getCanvasWidth()
        a5 = a6.model.getModelImpl().getCanvasHeight()
        aJ = aU if aU > a5 else a5
        aT = aJ
        aR = aJ
        aS = 0
        aP = 0
        aL = len(aV.clippedDrawContextList)
        for aM in range(0, aL, 1):
            aW = aV.clippedDrawContextList[aM]
            aN = aW.drawDataIndex
            aK = a6.getDrawContext(aN)
            if aK.isAvailable():
                aX = aK.getTransformedPoints()
                a4 = len(aX)
                aI = Float32Array(a4)
                aH = Float32Array(a4)
                aO = 0
                for a3 in range(VERTEX_OFFSET, a4, VERTEX_STEP):
                    aI[aO] = aX[a3]
                    aH[aO] = aX[a3 + 1]
                    aO += 1

                a2 = min(aI)
                a1 = min(aH)
                a0 = max(aI)
                aZ = max(aH)
                if a2 < aT:
                    aT = a2

                if a1 < aR:
                    aR = a1

                if a0 > aS:
                    aS = a0

                if aZ > aP:
                    aP = aZ

        if aT == aJ:
            aV.allClippedDrawRect.x = 0
            aV.allClippedDrawRect.y = 0
            aV.allClippedDrawRect.width = 0
            aV.allClippedDrawRect.height = 0
            aV.isUsing = False
        else:
            aQ = aS - aT
            aY = aP - aR
            aV.allClippedDrawRect.x = aT
            aV.allClippedDrawRect.y = aR
            aV.allClippedDrawRect.width = aQ
            aV.allClippedDrawRect.height = aY
            aV.isUsing = True

    def setupLayoutBounds(self, aQ):
        aI = aQ / ClippingManagerOpenGL.CHANNEL_COUNT
        aP = aQ % ClippingManagerOpenGL.CHANNEL_COUNT
        aI = int(aI)
        aP = int(aP)
        aH = 0
        for aJ in range(0, ClippingManagerOpenGL.CHANNEL_COUNT, 1):
            aM = aI + (1 if aJ < aP else 0)
            if aM == 1:
                aL = self.clipContextList[aH]
                aH += 1
                aL.layoutChannelNo = aJ
                aL.layoutBounds.x = 0
                aL.layoutBounds.y = 0
                aL.layoutBounds.width = 1
                aL.layoutBounds.height = 1
            elif aM == 2:
                for aO in range(0, aM, 1):
                    aN = aO % 2
                    aN = int(aN)
                    aL = self.clipContextList[aH]
                    aH += 1
                    aL.layoutChannelNo = aJ
                    aL.layoutBounds.x = aN * 0.5
                    aL.layoutBounds.y = 0
                    aL.layoutBounds.width = 0.5
                    aL.layoutBounds.height = 1
            elif aM <= 4:
                for aO in range(0, aM, 1):
                    aN = aO % 2
                    aK = aO / 2
                    aN = int(aN)
                    aK = int(aK)
                    aL = self.clipContextList[aH]
                    aH += 1
                    aL.layoutChannelNo = aJ
                    aL.layoutBounds.x = aN * 0.5
                    aL.layoutBounds.y = aK * 0.5
                    aL.layoutBounds.width = 0.5
                    aL.layoutBounds.height = 0.5
            elif aM <= 9:
                for aO in range(0, aM, 1):
                    aN = aO % 3
                    aK = aO / 3
                    aN = int(aN)
                    aK = int(aK)
                    aL = self.clipContextList[aH]
                    aH += 1
                    aL.layoutChannelNo = aJ
                    aL.layoutBounds.x = aN / 3
                    aL.layoutBounds.y = aK / 3
                    aL.layoutBounds.width = 1 / 3
                    aL.layoutBounds.height = 1 / 3
