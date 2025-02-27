import math
from typing import TYPE_CHECKING, Optional, List

from .deformer import Deformer
from .rotation_context import RotationContext
from ..DEF import LIVE2D_FORMAT_VERSION_V2_10_SDK2
from ..param import PivotManager
from ..type import Float32Array, Array
from ..util import UtMath

if TYPE_CHECKING:
    from .deformer_context import DeformerContext
    from ..model_context import ModelContext
    from ..io import BinaryReader


class RotationDeformer(Deformer):
    # 代替c语言中的指针/引用
    temp1 = [0.0, 0.0]
    temp2 = [0.0, 0.0]
    temp3 = [0.0, 0.0]
    temp4 = [0.0, 0.0]
    temp5 = [0.0, 0.0]
    temp6 = [0.0, 0.0]
    paramOutside = [False]

    def __init__(self):
        super().__init__()
        self.pivotManager: Optional['PivotManager'] = None
        self.affines: Optional[List['AffineEnt']] = None

    def getType(self) -> int:
        return Deformer.TYPE_ROTATION

    def read(self, br: 'BinaryReader'):
        super().read(br)
        self.pivotManager = br.readObject()
        self.affines = br.readObject()
        super().readOpacity(br)

    def init(self, mc) -> 'RotationContext':
        rctx = RotationContext(self)
        rctx.interpolatedAffine = AffineEnt()
        if self.needTransform():
            rctx.transformedAffine = AffineEnt()

        return rctx

    def setupInterpolate(self, mctx: 'ModelContext', rctx: 'RotationContext'):
        if not (self == rctx.getDeformer()):
            raise RuntimeError("context not match")

        if not self.pivotManager.checkParamUpdated(mctx):
            return

        success = RotationDeformer.paramOutside
        success[0] = False
        a2 = self.pivotManager.calcPivotValues(mctx, success)
        rctx.setOutsideParam(success[0])
        self.interpolateOpacity(mctx, self.pivotManager, rctx, success)
        a3 = mctx.getTempPivotTableIndices()
        ba = mctx.getTempT()
        self.pivotManager.calcPivotIndices(a3, ba, a2)
        if a2 <= 0:
            bn_3 = self.affines[a3[0]]
            rctx.interpolatedAffine.init(bn_3)
        else:
            if a2 == 1:
                bn_1 = self.affines[a3[0]]
                bl = self.affines[a3[1]]
                a9 = ba[0]
                rctx.interpolatedAffine.originX = bn_1.originX + (bl.originX - bn_1.originX) * a9
                rctx.interpolatedAffine.originY = bn_1.originY + (bl.originY - bn_1.originY) * a9
                rctx.interpolatedAffine.scaleX = bn_1.scaleX + (bl.scaleX - bn_1.scaleX) * a9
                rctx.interpolatedAffine.scaleY = bn_1.scaleY + (bl.scaleY - bn_1.scaleY) * a9
                rctx.interpolatedAffine.rotationDeg = bn_1.rotationDeg + (
                        bl.rotationDeg - bn_1.rotationDeg) * a9
            else:
                if a2 == 2:
                    bn_1 = self.affines[a3[0]]
                    bl = self.affines[a3[1]]
                    a1 = self.affines[a3[2]]
                    a0 = self.affines[a3[3]]
                    a9 = ba[0]
                    a8 = ba[1]
                    bC = bn_1.originX + (bl.originX - bn_1.originX) * a9
                    bB = a1.originX + (a0.originX - a1.originX) * a9
                    rctx.interpolatedAffine.originX = bC + (bB - bC) * a8
                    bC = bn_1.originY + (bl.originY - bn_1.originY) * a9
                    bB = a1.originY + (a0.originY - a1.originY) * a9
                    rctx.interpolatedAffine.originY = bC + (bB - bC) * a8
                    bC = bn_1.scaleX + (bl.scaleX - bn_1.scaleX) * a9
                    bB = a1.scaleX + (a0.scaleX - a1.scaleX) * a9
                    rctx.interpolatedAffine.scaleX = bC + (bB - bC) * a8
                    bC = bn_1.scaleY + (bl.scaleY - bn_1.scaleY) * a9
                    bB = a1.scaleY + (a0.scaleY - a1.scaleY) * a9
                    rctx.interpolatedAffine.scaleY = bC + (bB - bC) * a8
                    bC = bn_1.rotationDeg + (bl.rotationDeg - bn_1.rotationDeg) * a9
                    bB = a1.rotationDeg + (a0.rotationDeg - a1.rotationDeg) * a9
                    rctx.interpolatedAffine.rotationDeg = bC + (bB - bC) * a8
                else:
                    if a2 == 3:
                        aP = self.affines[a3[0]]
                        aO = self.affines[a3[1]]
                        bu = self.affines[a3[2]]
                        bs = self.affines[a3[3]]
                        aK = self.affines[a3[4]]
                        aJ = self.affines[a3[5]]
                        bj = self.affines[a3[6]]
                        bi = self.affines[a3[7]]
                        a9 = ba[0]
                        a8 = ba[1]
                        a6 = ba[2]
                        bC = aP.originX + (aO.originX - aP.originX) * a9
                        bB = bu.originX + (bs.originX - bu.originX) * a9
                        bz = aK.originX + (aJ.originX - aK.originX) * a9
                        by = bj.originX + (bi.originX - bj.originX) * a9
                        rctx.interpolatedAffine.originX = (1 - a6) * (bC + (bB - bC) * a8) + a6 * (
                                bz + (by - bz) * a8)
                        bC = aP.originY + (aO.originY - aP.originY) * a9
                        bB = bu.originY + (bs.originY - bu.originY) * a9
                        bz = aK.originY + (aJ.originY - aK.originY) * a9
                        by = bj.originY + (bi.originY - bj.originY) * a9
                        rctx.interpolatedAffine.originY = (1 - a6) * (bC + (bB - bC) * a8) + a6 * (
                                bz + (by - bz) * a8)
                        bC = aP.scaleX + (aO.scaleX - aP.scaleX) * a9
                        bB = bu.scaleX + (bs.scaleX - bu.scaleX) * a9
                        bz = aK.scaleX + (aJ.scaleX - aK.scaleX) * a9
                        by = bj.scaleX + (bi.scaleX - bj.scaleX) * a9
                        rctx.interpolatedAffine.scaleX = (1 - a6) * (bC + (bB - bC) * a8) + a6 * (
                                bz + (by - bz) * a8)
                        bC = aP.scaleY + (aO.scaleY - aP.scaleY) * a9
                        bB = bu.scaleY + (bs.scaleY - bu.scaleY) * a9
                        bz = aK.scaleY + (aJ.scaleY - aK.scaleY) * a9
                        by = bj.scaleY + (bi.scaleY - bj.scaleY) * a9
                        rctx.interpolatedAffine.scaleY = (1 - a6) * (bC + (bB - bC) * a8) + a6 * (
                                bz + (by - bz) * a8)
                        bC = aP.rotationDeg + (aO.rotationDeg - aP.rotationDeg) * a9
                        bB = bu.rotationDeg + (bs.rotationDeg - bu.rotationDeg) * a9
                        bz = aK.rotationDeg + (aJ.rotationDeg - aK.rotationDeg) * a9
                        by = bj.rotationDeg + (bi.rotationDeg - bj.rotationDeg) * a9
                        rctx.interpolatedAffine.rotationDeg = (1 - a6) * (bC + (bB - bC) * a8) + a6 * (
                                bz + (by - bz) * a8)
                    else:
                        if a2 == 4:
                            aT = self.affines[a3[0]]
                            aS = self.affines[a3[1]]
                            bE = self.affines[a3[2]]
                            bD = self.affines[a3[3]]
                            aN = self.affines[a3[4]]
                            aM = self.affines[a3[5]]
                            bp = self.affines[a3[6]]
                            bo = self.affines[a3[7]]
                            bh = self.affines[a3[8]]
                            bg = self.affines[a3[9]]
                            aY = self.affines[a3[10]]
                            aW = self.affines[a3[11]]
                            a7 = self.affines[a3[12]]
                            a5 = self.affines[a3[13]]
                            aR = self.affines[a3[14]]
                            aQ = self.affines[a3[15]]
                            a9 = ba[0]
                            a8 = ba[1]
                            a6 = ba[2]
                            a4 = ba[3]
                            bC = aT.originX + (aS.originX - aT.originX) * a9
                            bB = bE.originX + (bD.originX - bE.originX) * a9
                            bz = aN.originX + (aM.originX - aN.originX) * a9
                            by = bp.originX + (bo.originX - bp.originX) * a9
                            bv = bh.originX + (bg.originX - bh.originX) * a9
                            bt = aY.originX + (aW.originX - aY.originX) * a9
                            br = a7.originX + (a5.originX - a7.originX) * a9
                            bq = aR.originX + (aQ.originX - aR.originX) * a9
                            rctx.interpolatedAffine.originX = (1 - a4) * (
                                    (1 - a6) * (bC + (bB - bC) * a8) + a6 * (bz + (by - bz) * a8)) + a4 * (
                                                                      (1 - a6) * (
                                                                      bv + (bt - bv) * a8) + a6 * (
                                                                              br + (bq - br) * a8))
                            bC = aT.originY + (aS.originY - aT.originY) * a9
                            bB = bE.originY + (bD.originY - bE.originY) * a9
                            bz = aN.originY + (aM.originY - aN.originY) * a9
                            by = bp.originY + (bo.originY - bp.originY) * a9
                            bv = bh.originY + (bg.originY - bh.originY) * a9
                            bt = aY.originY + (aW.originY - aY.originY) * a9
                            br = a7.originY + (a5.originY - a7.originY) * a9
                            bq = aR.originY + (aQ.originY - aR.originY) * a9
                            rctx.interpolatedAffine.originY = (1 - a4) * (
                                    (1 - a6) * (bC + (bB - bC) * a8) + a6 * (bz + (by - bz) * a8)) + a4 * (
                                                                      (1 - a6) * (
                                                                      bv + (bt - bv) * a8) + a6 * (
                                                                              br + (bq - br) * a8))
                            bC = aT.scaleX + (aS.scaleX - aT.scaleX) * a9
                            bB = bE.scaleX + (bD.scaleX - bE.scaleX) * a9
                            bz = aN.scaleX + (aM.scaleX - aN.scaleX) * a9
                            by = bp.scaleX + (bo.scaleX - bp.scaleX) * a9
                            bv = bh.scaleX + (bg.scaleX - bh.scaleX) * a9
                            bt = aY.scaleX + (aW.scaleX - aY.scaleX) * a9
                            br = a7.scaleX + (a5.scaleX - a7.scaleX) * a9
                            bq = aR.scaleX + (aQ.scaleX - aR.scaleX) * a9
                            rctx.interpolatedAffine.scaleX = (1 - a4) * (
                                    (1 - a6) * (bC + (bB - bC) * a8) + a6 * (bz + (by - bz) * a8)) + a4 * (
                                                                     (1 - a6) * (
                                                                     bv + (bt - bv) * a8) + a6 * (
                                                                             br + (bq - br) * a8))
                            bC = aT.scaleY + (aS.scaleY - aT.scaleY) * a9
                            bB = bE.scaleY + (bD.scaleY - bE.scaleY) * a9
                            bz = aN.scaleY + (aM.scaleY - aN.scaleY) * a9
                            by = bp.scaleY + (bo.scaleY - bp.scaleY) * a9
                            bv = bh.scaleY + (bg.scaleY - bh.scaleY) * a9
                            bt = aY.scaleY + (aW.scaleY - aY.scaleY) * a9
                            br = a7.scaleY + (a5.scaleY - a7.scaleY) * a9
                            bq = aR.scaleY + (aQ.scaleY - aR.scaleY) * a9
                            rctx.interpolatedAffine.scaleY = (1 - a4) * (
                                    (1 - a6) * (bC + (bB - bC) * a8) + a6 * (bz + (by - bz) * a8)) + a4 * (
                                                                     (1 - a6) * (
                                                                     bv + (bt - bv) * a8) + a6 * (
                                                                             br + (bq - br) * a8))
                            bC = aT.rotationDeg + (aS.rotationDeg - aT.rotationDeg) * a9
                            bB = bE.rotationDeg + (bD.rotationDeg - bE.rotationDeg) * a9
                            bz = aN.rotationDeg + (aM.rotationDeg - aN.rotationDeg) * a9
                            by = bp.rotationDeg + (bo.rotationDeg - bp.rotationDeg) * a9
                            bv = bh.rotationDeg + (bg.rotationDeg - bh.rotationDeg) * a9
                            bt = aY.rotationDeg + (aW.rotationDeg - aY.rotationDeg) * a9
                            br = a7.rotationDeg + (a5.rotationDeg - a7.rotationDeg) * a9
                            bq = aR.rotationDeg + (aQ.rotationDeg - aR.rotationDeg) * a9
                            rctx.interpolatedAffine.rotationDeg = (1 - a4) * (
                                    (1 - a6) * (bC + (bB - bC) * a8) + a6 * (bz + (by - bz) * a8)) + a4 * (
                                                                          (1 - a6) * (
                                                                          bv + (bt - bv) * a8) + a6 * (
                                                                                  br + (bq - br) * a8))
                        else:
                            aV = int(pow(2, a2))
                            aZ = Float32Array(aV)
                            for bk in range(0, aV, 1):
                                aI = bk
                                aH = 1
                                for aL in range(0, a2, 1):
                                    aH *= (1 - ba[aL]) if (aI % 2 == 0) else ba[aL]
                                    aI /= 2

                                aZ[bk] = aH

                            bA = Array()
                            for aU in range(0, aV, 1):
                                bA[aU] = self.affines[a3[aU]]

                            be = 0
                            bc = 0
                            bd = 0
                            bb = 0
                            aX = 0
                            for aU in range(0, aV, 1):
                                be += aZ[aU] * bA[aU].originX
                                bc += aZ[aU] * bA[aU].originY
                                bd += aZ[aU] * bA[aU].scaleX
                                bb += aZ[aU] * bA[aU].scaleY
                                aX += aZ[aU] * bA[aU].rotationDeg

                            rctx.interpolatedAffine.originX = be
                            rctx.interpolatedAffine.originY = bc
                            rctx.interpolatedAffine.scaleX = bd
                            rctx.interpolatedAffine.scaleY = bb
                            rctx.interpolatedAffine.rotationDeg = aX

        bn = self.affines[a3[0]]
        rctx.interpolatedAffine.reflectX = bn.reflectX
        rctx.interpolatedAffine.reflectY = bn.reflectY

    def setupTransform(self, mctx: 'ModelContext', rctx: 'RotationContext'):
        if not (self == rctx.getDeformer()):
            raise RuntimeError("Invalid Deformer")

        rctx.setAvailable(True)
        if not self.needTransform():
            rctx.setTotalScale_notForClient(rctx.interpolatedAffine.scaleX)
            rctx.setTotalOpacity(rctx.getInterpolatedOpacity())
        else:
            aT = self.getTargetId()
            if rctx.tmpDeformerIndex == Deformer.DEFORMER_INDEX_NOT_INIT:
                rctx.tmpDeformerIndex = mctx.getDeformerIndex(aT)

            if rctx.tmpDeformerIndex < 0:
                print("deformer is not reachable")
                rctx.setAvailable(False)
            else:
                deformer = mctx.getDeformer(rctx.tmpDeformerIndex)
                if deformer is not None:
                    dctx = mctx.getDeformerContext(rctx.tmpDeformerIndex)
                    aS = RotationDeformer.temp1
                    aS[0] = rctx.interpolatedAffine.originX
                    aS[1] = rctx.interpolatedAffine.originY
                    aJ = RotationDeformer.temp2
                    aJ[0] = 0
                    aJ[1] = -0.1
                    aO = dctx.getDeformer().getType()
                    if aO == Deformer.TYPE_ROTATION:
                        aJ[1] = -10
                    else:
                        aJ[1] = -0.1

                    aQ = RotationDeformer.temp3
                    self.getDirectionOnDst(mctx, deformer, dctx, aS, aJ, aQ)
                    aP = UtMath.getAngleNotAbs(aJ, aQ)
                    deformer.transformPoints(mctx, dctx, aS, aS, 1, 0, 2)
                    rctx.transformedAffine.originX = aS[0]
                    rctx.transformedAffine.originY = aS[1]
                    rctx.transformedAffine.scaleX = rctx.interpolatedAffine.scaleX
                    rctx.transformedAffine.scaleY = rctx.interpolatedAffine.scaleY
                    rctx.transformedAffine.rotationDeg = rctx.interpolatedAffine.rotationDeg - aP * UtMath.RAD_TO_DEG
                    aK = dctx.getTotalScale()
                    rctx.setTotalScale_notForClient(aK * rctx.transformedAffine.scaleX)
                    aN = dctx.getTotalOpacity()
                    rctx.setTotalOpacity(aN * rctx.getInterpolatedOpacity())
                    rctx.transformedAffine.reflectX = rctx.interpolatedAffine.reflectX
                    rctx.transformedAffine.reflectY = rctx.interpolatedAffine.reflectY
                    rctx.setAvailable(dctx.isAvailable())
                else:
                    rctx.setAvailable(False)

    def transformPoints(self, mc: 'ModelContext', dc: 'RotationContext', srcPoints: List[float], dstPoints: List[float],
                        numPoint: int, ptOffset: int, ptStep: int):
        if not (self == dc.getDeformer()):
            raise RuntimeError("context not match")

        aH = dc
        aU = aH.transformedAffine if aH.transformedAffine is not None else aH.interpolatedAffine
        a0 = math.sin(UtMath.DEG_TO_RAD * aU.rotationDeg)
        aP = math.cos(UtMath.DEG_TO_RAD * aU.rotationDeg)
        a3 = aH.getTotalScale()
        aW = -1 if aU.reflectX else 1
        aV = -1 if aU.reflectY else 1
        aS = aP * a3 * aW
        aQ = -a0 * a3 * aV
        a1 = a0 * a3 * aW
        aZ = aP * a3 * aV
        aY = aU.originX
        aX = aU.originY
        aI = numPoint * ptStep
        for aK in range(ptOffset, aI, ptStep):
            aN = srcPoints[aK]
            aM = srcPoints[aK + 1]
            dstPoints[aK] = aS * aN + aQ * aM + aY
            dstPoints[aK + 1] = a1 * aN + aZ * aM + aX

    @staticmethod
    def getDirectionOnDst(mdc: 'ModelContext', targetToDst: 'Deformer', targetToDstContext: 'DeformerContext', srcOrigin, srcDir, retDir):
        if not (targetToDst == targetToDstContext.getDeformer()):
            raise RuntimeError("context not match")

        aO = RotationDeformer.temp4
        RotationDeformer.temp4[0] = srcOrigin[0]
        RotationDeformer.temp4[1] = srcOrigin[1]
        targetToDst.transformPoints(mdc, targetToDstContext, aO, aO, 1, 0, 2)
        aL = RotationDeformer.temp5
        aS = RotationDeformer.temp6
        aN = 10
        aJ = 1
        for aM in range(0, aN, 1):
            aS[0] = srcOrigin[0] + aJ * srcDir[0]
            aS[1] = srcOrigin[1] + aJ * srcDir[1]
            targetToDst.transformPoints(mdc, targetToDstContext, aS, aL, 1, 0, 2)
            aL[0] -= aO[0]
            aL[1] -= aO[1]
            if aL[0] != 0 or aL[1] != 0:
                retDir[0] = aL[0]
                retDir[1] = aL[1]
                return

            aS[0] = srcOrigin[0] - aJ * srcDir[0]
            aS[1] = srcOrigin[1] - aJ * srcDir[1]
            targetToDst.transformPoints(mdc, targetToDstContext, aS, aL, 1, 0, 2)
            aL[0] -= aO[0]
            aL[1] -= aO[1]
            if aL[0] != 0 or aL[1] != 0:
                aL[0] = -aL[0]
                aL[0] = -aL[0]
                retDir[0] = aL[0]
                retDir[1] = aL[1]
                return

            aJ *= 0.1


        print("Invalid state\n")


class AffineEnt:

    def __init__(self):
        self.originX = 0
        self.originY = 0
        self.scaleX = 1
        self.scaleY = 1
        self.rotationDeg = 0
        self.reflectX = False
        self.reflectY = False

    def init(self, other):
        self.originX = other.originX
        self.originY = other.originY
        self.scaleX = other.scaleX
        self.scaleY = other.scaleY
        self.rotationDeg = other.rotationDeg
        self.reflectX = other.reflectX
        self.reflectY = other.reflectY

    def read(self, br: 'BinaryReader'):
        self.originX = br.readFloat32()
        self.originY = br.readFloat32()
        self.scaleX = br.readFloat32()
        self.scaleY = br.readFloat32()
        self.rotationDeg = br.readFloat32()
        if br.getFormatVersion() >= LIVE2D_FORMAT_VERSION_V2_10_SDK2:
            self.reflectX = br.readBoolean()
            self.reflectY = br.readBoolean()
