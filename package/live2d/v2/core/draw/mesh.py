from typing import TYPE_CHECKING

from .idraw_data import IDrawData
from .mesh_context import MeshContext
from ..DEF import LIVE2D_FORMAT_VERSION_V2_8_TEX_OPTION, VERTEX_STEP, VERTEX_TYPE, VERTEX_OFFSET, \
    VERTEX_TYPE_OFFSET0_STEP2, REVERSE_TEXTURE_T, VERTEX_TYPE_OFFSET2_STEP5
from ..live2d import Live2D
from ..param import PivotManager
from ..type import Int16Array, Float32Array
from ..util import UtInterpolate

if TYPE_CHECKING:
    from ..model_context import ModelContext


class Mesh(IDrawData):
    INSTANCE_COUNT = 0
    MASK_COLOR_COMPOSITION = 30
    COLOR_COMPOSITION_NORMAL = 0
    COLOR_COMPOSITION_SCREEN = 1
    COLOR_COMPOSITION_MULTIPLY = 2
    paramOutside = [False]

    def __init__(self):
        super().__init__()
        self.textureNo = -1
        self.pointCount = 0
        self.polygonCount = 0
        self.optionFlag = None
        self.indexArray = None
        self.pivotPoints = None
        self.uvs = None
        self.colorCompositionType = Mesh.COLOR_COMPOSITION_NORMAL
        self.culling = True
        self.instanceNo = Mesh.INSTANCE_COUNT
        Mesh.INSTANCE_COUNT += 1

    def setTextureNo(self, aH):
        self.textureNo = aH

    def getTextureNo(self):
        return self.textureNo

    def getUvs(self):
        return self.uvs

    def getOptionFlag(self):
        return self.optionFlag

    def getNumPoints(self):
        return self.pointCount

    def getType(self):
        return IDrawData.TYPE_MESH

    def read(self, br):
        super().read(br)
        self.textureNo = br.readInt32()
        self.pointCount = br.readInt32()
        self.polygonCount = br.readInt32()
        obj = br.readObject()
        self.indexArray = Int16Array(self.polygonCount * 3)
        for aJ in range(self.polygonCount * 3 - 1, 0 - 1, -1):
            self.indexArray[aJ] = obj[aJ]

        self.pivotPoints = br.readObject()
        self.uvs = br.readObject()
        if br.getFormatVersion() >= LIVE2D_FORMAT_VERSION_V2_8_TEX_OPTION:
            self.optionFlag = br.readInt32()
            if self.optionFlag != 0:
                if (self.optionFlag & 1) != 0:
                    _ = br.readInt32()
                    raise RuntimeError("not handled")

                if (self.optionFlag & Mesh.MASK_COLOR_COMPOSITION) != 0:
                    self.colorCompositionType = (self.optionFlag & Mesh.MASK_COLOR_COMPOSITION) >> 1
                else:
                    self.colorCompositionType = Mesh.COLOR_COMPOSITION_NORMAL

                if (self.optionFlag & 32) != 0:
                    self.culling = False
        else:
            self.optionFlag = 0

    def init(self, aL):
        ctx = MeshContext(self)
        aI = self.pointCount * VERTEX_STEP
        aH = self.needTransform()
        if ctx.interpolatedPoints is not None:
            ctx.interpolatedPoints = None

        ctx.interpolatedPoints = Float32Array(aI)
        if ctx.transformedPoints is not None:
            ctx.transformedPoints = None

        ctx.transformedPoints = Float32Array(aI) if aH else None
        aM = VERTEX_TYPE

        if aM == VERTEX_TYPE_OFFSET0_STEP2:
            if REVERSE_TEXTURE_T:
                for aJ in range(self.pointCount - 1, 0 - 1, -1):
                    aO = aJ << 1
                    self.uvs[aO + 1] = 1 - self.uvs[aO + 1]
        elif aM == VERTEX_TYPE_OFFSET2_STEP5:
            for aJ in range(self.pointCount - 1, 0 - 1, -1):
                aO = aJ << 1
                aK = aJ * VERTEX_STEP
                aQ = self.uvs[aO]
                aP = self.uvs[aO + 1]
                ctx.interpolatedPoints[aK] = aQ
                ctx.interpolatedPoints[aK + 1] = aP
                ctx.interpolatedPoints[aK + 4] = 0
                if aH:
                    ctx.transformedPoints[aK] = aQ
                    ctx.transformedPoints[aK + 1] = aP
                    ctx.transformedPoints[aK + 4] = 0

        return ctx

    def setupInterpolate(self, aJ, aH):
        aK = aH
        if not (self == aK.getDrawData()):
            print("### assert!! ### ")

        if not self.pivotMgr.checkParamUpdated(aJ):
            return

        super().setupInterpolate(aJ, aK)
        if aK.paramOutside[0]:
            return

        aI = Mesh.paramOutside
        aI[0] = False
        UtInterpolate.interpolatePoints(aJ, self.pivotMgr, aI, self.pointCount, self.pivotPoints, aK.interpolatedPoints,
                                        VERTEX_OFFSET, VERTEX_STEP)

    def setupTransform(self, mc, dc=None):
        if not (self == dc.getDrawData()):
            raise RuntimeError("context not match")

        aL = False
        if dc.paramOutside[0]:
            aL = True

        if not aL:
            super().setupTransform(mc)
            if self.needTransform():
                target_id = self.getTargetId()
                if dc.tmpDeformerIndex == IDrawData.DEFORMER_INDEX_NOT_INIT:
                    dc.tmpDeformerIndex = mc.getDeformerIndex(target_id)

                if dc.tmpDeformerIndex < 0:
                    print(f"deformer not found: {target_id}")
                else:
                    d = mc.getDeformer(dc.tmpDeformerIndex)
                    dctx = mc.getDeformerContext(dc.tmpDeformerIndex)
                    if d is not None and not dctx.isOutsideParam():
                        d.transformPoints(mc, dctx, dc.interpolatedPoints, dc.transformedPoints, self.pointCount,
                                          VERTEX_OFFSET, VERTEX_STEP)
                        dc.available = True
                    else:
                        dc.available = False

                    dc.baseOpacity = dctx.getTotalOpacity()

    def draw(self, dp, mctx: 'ModelContext', dctx: 'MeshContext'):
        if not (self == dctx.getDrawData()):
            raise RuntimeError("context not match")

        if dctx.paramOutside[0]:
            return

        texNr = self.textureNo
        if texNr < 0:
            texNr = 1

        opacity = (self.getOpacity(dctx) *
                    dctx.partsOpacity *
                    dctx.baseOpacity)
        # print("op1: ", opacity)
        vertices = dctx.transformedPoints if (dctx.transformedPoints is not None) else dctx.interpolatedPoints
        dp.setClipBufPre_clipContextForDraw(dctx.clipBufPre_clipContext)
        dp.setCulling(self.culling)
        pctx = mctx.getPartsContext(dctx.partsIndex)
        dp.drawTexture(texNr, pctx.screenColor, self.indexArray, vertices, self.uvs, opacity, self.colorCompositionType,
                       pctx.multiplyColor)

    def getIndexArray(self):
        return self.indexArray
