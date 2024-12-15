from ..type import Array, Float32Array
from .clip_rectf import ClipRectF
from .clip_draw_context import ClipDrawContext


class ClipContext:

    def __init__(self, aH, aK, aI):
        self.clipIDList = Array()
        self.clipIDList = aI
        self.clippingMaskDrawIndexList = Array()
        for aJ in range(0, len(aI), 1):
            self.clippingMaskDrawIndexList.append(aK.getDrawDataIndex(aI[aJ]))

        self.clippedDrawContextList = Array()
        self.isUsing = True
        self.layoutChannelNo = 0
        self.layoutBounds = ClipRectF()
        self.allClippedDrawRect = ClipRectF()
        self.matrixForMask = Float32Array(16)
        self.matrixForDraw = Float32Array(16)
        self.owner = aH

    def addClippedDrawData(self, aJ, aI):
        aH = ClipDrawContext(aJ, aI)
        self.clippedDrawContextList.append(aH)
