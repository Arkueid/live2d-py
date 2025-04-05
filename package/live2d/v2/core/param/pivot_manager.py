from typing import List, TYPE_CHECKING

if TYPE_CHECKING:
    from ..model_context import ModelContext

from ..DEF import GOSA, PIVOT_TABLE_SIZE
from ..io.iserializable import ISerializable
from ..param import ParamPivots


class PivotManager(ISerializable):

    def __init__(self):
        self.paramPivotTable = None

    def read(self, aH):
        self.paramPivotTable = aH.readObject()

    def checkParamUpdated(self, aK):
        if aK.requireSetup():
            return True

        aH = aK.getInitVersion()
        for aJ in range(len(self.paramPivotTable) - 1, -1, -1):
            aI = self.paramPivotTable[aJ].getParamIndex(aH)
            if aI == ParamPivots.PARAM_INDEX_NOT_INIT:
                aI = aK.getParamIndex(self.paramPivotTable[aJ].getParamID())

            if aK.isParamUpdated(aI):
                return True

        return False

    def calcPivotValues(self, mdc: 'ModelContext', ret: List[bool]):
        aX = len(self.paramPivotTable)
        aJ = mdc.getInitVersion()
        aN = 0
        for aK in range(0, aX, 1):
            aH = self.paramPivotTable[aK]
            aI = aH.getParamIndex(aJ)
            if aI == ParamPivots.PARAM_INDEX_NOT_INIT:
                aI = mdc.getParamIndex(aH.getParamID())
                aH.setParamIndex(aI, aJ)

            if aI < 0:
                raise Exception("err 23242 : " + aH.getParamID())

            aU = 0 if aI < 0 else mdc.getParamFloat(aI)
            aQ = aH.getPivotCount()
            aM = aH.getPivotValues()
            aP = -1
            aT = 0
            if aQ < 1:
                pass
            else:
                if aQ == 1:
                    aS = aM[0]
                    if aS - GOSA < aU < aS + GOSA:
                        aP = 0
                        aT = 0
                    else:
                        aP = 0
                        ret[0] = True
                else:
                    aS = aM[0]
                    if aU < aS - GOSA:
                        aP = 0
                        ret[0] = True
                    else:
                        if aU < aS + GOSA:
                            aP = 0
                        else:
                            aW = False
                            for aO in range(1, aQ, 1):
                                aR = aM[aO]
                                if aU < aR + GOSA:
                                    if aR - GOSA < aU:
                                        aP = aO
                                    else:
                                        aP = aO - 1
                                        aT = (aU - aS) / (aR - aS)
                                        aN += 1

                                    aW = True
                                    break

                                aS = aR

                            if not aW:
                                aP = aQ - 1
                                aT = 0
                                ret[0] = True

            aH.setTmpPivotIndex(aP)
            aH.setTmpT(aT)

        return aN

    def calcPivotIndices(self, aN, aT, aP):
        aR = 1 << aP
        if aR + 1 > PIVOT_TABLE_SIZE:
            print("err 23245\n")

        aS = len(self.paramPivotTable)
        aK = 1
        aH = 1
        aJ = 0
        for aQ in range(0, aR, 1):
            aN[aQ] = 0

        for aL in range(0, aS, 1):
            aI = self.paramPivotTable[aL]
            if aI.getTmpT() == 0:
                aO = aI.getTmpPivotIndex() * aK
                if aO < 0:
                    raise Exception("err 23246")

                for aQ in range(0, aR, 1):
                    aN[aQ] += aO
            else:
                aO = aK * aI.getTmpPivotIndex()
                aM = aK * (aI.getTmpPivotIndex() + 1)
                for aQ in range(0, aR, 1):
                    aN[aQ] += aO if (int(aQ / aH) % 2 == 0) else aM

                aT[aJ] = aI.getTmpT()
                aJ += 1
                aH *= 2

            aK *= aI.getPivotCount()

        aN[aR] = 65535
        aT[aJ] = -1

    def getParamCount(self):
        return len(self.paramPivotTable)
