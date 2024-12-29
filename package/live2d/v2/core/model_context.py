﻿import math
from typing import TYPE_CHECKING

from .DEF import PIVOT_TABLE_SIZE, MAX_INTERPOLATION
from .draw import IDrawData
from .graphics import ClippingManagerOpenGL
from .id import BaseDataID
from .id import DrawDataID
from .type import Array, Float32Array, Int16Array

if TYPE_CHECKING:
    from .draw import MeshContext
    from .model import PartsDataContext


class ModelContext:
    __verbose = True
    NOT_USED_ORDER = -1
    NO_NEXT = -1
    DEFAULT_PARAM_UPDATE_FLAG = False
    PARAM_UPDATED = True
    PARAM_FLOAT_MIN = (-1000000)
    PARAM_FLOAT_MAX = 1000000
    DEFAULT_ARRAY_LENGTH = 0

    def __init__(self, model):
        self.needSetup = True
        self.initVersion = -1
        self.nextParamPos = 0
        self.paramIdList = Array(ModelContext.DEFAULT_ARRAY_LENGTH)
        self.paramValues = Float32Array(ModelContext.DEFAULT_ARRAY_LENGTH)
        self.lastParamValues = Float32Array(ModelContext.DEFAULT_ARRAY_LENGTH)
        self.paramMinValues = Float32Array(ModelContext.DEFAULT_ARRAY_LENGTH)
        self.paramMaxValues = Float32Array(ModelContext.DEFAULT_ARRAY_LENGTH)
        self.savedParamValues = Float32Array(ModelContext.DEFAULT_ARRAY_LENGTH)
        self.updatedParamFlags = Array(ModelContext.DEFAULT_ARRAY_LENGTH)
        self.deformerList = Array()
        self.drawDataList = Array()
        self.tmpDrawDataList = None
        self.partsDataList = Array()
        self.deformerContextList = Array()
        self.drawContextList = Array()
        self.partsContextList: 'list[PartsDataContext]' = Array()
        self.orderList_firstDrawIndex = None
        self.orderList_lastDrawIndex = None
        self.nextList_drawIndex = None
        self.tmpPivotTableIndices = Int16Array(PIVOT_TABLE_SIZE)
        self.tempTArray = Float32Array(MAX_INTERPOLATION)
        self.model = model
        self.clipManager = None
        self.dpGL = None

    def getDrawDataIndex(self, aI):
        for aH in range(len(self.drawDataList) - 1, 0 - 1, -1):
            if self.drawDataList[aH] is not None and self.drawDataList[aH].getId() == aI:
                return aH

        return -1

    def getDrawData(self, aH):
        if isinstance(aH, DrawDataID):
            if self.tmpDrawDataList is None:
                self.tmpDrawDataList = {}
                count = len(self.drawDataList)
                for i in range(0, count, 1):
                    dd = self.drawDataList[i]
                    dd_id = dd.getId()
                    if dd_id is None:
                        continue

                    self.tmpDrawDataList[dd_id] = dd

            return self.tmpDrawDataList[id]
        else:
            if aH < len(self.drawDataList):
                return self.drawDataList[aH]
            else:
                return None

    def release(self):
        self.deformerList.clear()
        self.drawDataList.clear()
        self.partsDataList.clear()
        if self.tmpDrawDataList is not None:
            self.tmpDrawDataList.clear()

        self.deformerContextList.clear()
        self.drawContextList.clear()
        self.partsContextList.clear()

    def init(self):
        self.initVersion += 1
        if len(self.partsDataList) > 0:
            self.release()

        aO = self.model.getModelImpl()
        parts_data_list = aO.getPartsDataList()
        aS = len(parts_data_list)
        aH = Array()
        a3 = Array()
        for aV in range(0, aS, 1):
            a4 = parts_data_list[aV]
            self.partsDataList.append(a4)
            self.partsContextList.append(a4.init())
            base_data_list = a4.getDeformer()
            aR = len(base_data_list)
            for aU in range(0, aR, 1):
                aH.append(base_data_list[aU])

            for aU in range(0, aR, 1):
                aM = base_data_list[aU].init(self)
                aM.setPartsIndex(aV)
                a3.append(aM)

            a1 = a4.getDrawData()
            aP = len(a1)
            for aU in range(0, aP, 1):
                aZ = a1[aU]
                a0 = aZ.init(self)
                a0.partsIndex = aV
                self.drawDataList.append(aZ)
                self.drawContextList.append(a0)

        aY = len(aH)
        aN = BaseDataID.DST_BASE_ID()
        while True:
            aX = False
            for aV in range(0, aY, 1):
                aL = aH[aV]
                if aL is None:
                    continue

                a2 = aL.getTargetId()
                if a2 is None or a2 == aN or self.getDeformerIndex(a2) >= 0:
                    self.deformerList.append(aL)
                    self.deformerContextList.append(a3[aV])
                    aH[aV] = None
                    aX = True

            if not aX:
                break

        aI = aO.E2_()
        if aI is not None:
            aJ = aI.getParamDefFloatList()
            if aJ is not None:
                aW = len(aJ)
                for aV in range(0, aW, 1):
                    aQ = aJ[aV]
                    if aQ is None:
                        continue

                    self.extendAndAddParam(aQ.getParamID(), aQ.getDefaultValue(), aQ.getMinValue(), aQ.getMaxValue())

        self.clipManager = ClippingManagerOpenGL(self.dpGL)
        self.clipManager.init(self, self.drawDataList, self.drawContextList)
        self.needSetup = True

    def update(self):
        aK = len(self.paramValues)
        for i in range(0, aK, 1):
            if self.paramValues[i] != self.lastParamValues[i]:
                self.updatedParamFlags[i] = ModelContext.PARAM_UPDATED
                self.lastParamValues[i] = self.paramValues[i]

        aX = False
        aQ = len(self.deformerList)
        aN = len(self.drawDataList)
        aS = IDrawData.getTotalMinOrder()
        aZ = IDrawData.getTotalMaxOrder()
        aU = aZ - aS + 1
        if self.orderList_firstDrawIndex is None or len(self.orderList_firstDrawIndex) < aU:
            self.orderList_firstDrawIndex = Int16Array(aU)
            self.orderList_lastDrawIndex = Int16Array(aU)

        for i in range(0, aU, 1):
            self.orderList_firstDrawIndex[i] = ModelContext.NOT_USED_ORDER
            self.orderList_lastDrawIndex[i] = ModelContext.NOT_USED_ORDER

        if self.nextList_drawIndex is None or len(self.nextList_drawIndex) < aN:
            self.nextList_drawIndex = Int16Array(aN)

        for i in range(0, aN, 1):
            self.nextList_drawIndex[i] = ModelContext.NO_NEXT

        aL = None
        for aV in range(0, aQ, 1):
            aJ = self.deformerList[aV]
            aH = self.deformerContextList[aV]
            aJ.setupInterpolate(self, aH)
            aJ.setupTransform(self, aH)

        if aL is not None:
            if ModelContext.__verbose:
                print(aL)

        aR = None
        for aO in range(0, aN, 1):
            aM = self.drawDataList[aO]
            aI = self.drawContextList[aO]
            aM.setupInterpolate(self, aI)
            if aI.isParamOutside():
                continue

            aM.setupTransform(self, aI)
            aT = math.floor(aM.getDrawOrder(aI) - aS)

            aP = self.orderList_lastDrawIndex[aT]

            if aP == ModelContext.NOT_USED_ORDER:
                self.orderList_firstDrawIndex[aT] = aO
            else:
                self.nextList_drawIndex[aP] = aO

            self.orderList_lastDrawIndex[aT] = aO

        if aR is not None:
            if ModelContext.__verbose:
                print(aR)

        for i in range(len(self.updatedParamFlags) - 1, -1, -1):
            self.updatedParamFlags[i] = ModelContext.DEFAULT_PARAM_UPDATE_FLAG

        self.needSetup = False
        return aX

    def preDraw(self, aH):
        if self.clipManager is not None:
            aH.setupDraw()
            self.clipManager.setupClip(self, aH)

    def draw(self, aM):
        if self.orderList_firstDrawIndex is None:
            print("call Ri_.update() before Ri_.draw() ")
            return

        aP = len(self.orderList_firstDrawIndex)
        aM.setupDraw()
        for aK in range(0, aP, 1):
            aN = self.orderList_firstDrawIndex[aK]
            if aN == ModelContext.NOT_USED_ORDER:
                continue

            while True:
                aH = self.drawDataList[aN]
                aI = self.drawContextList[aN]
                if aI.isAvailable():
                    aJ = aI.partsIndex
                    aL = self.partsContextList[aJ]
                    aI.partsOpacity = aL.getPartsOpacity()
                    aH.draw(aM, self, aI)

                aO = self.nextList_drawIndex[aN]
                if aO <= aN or aO == ModelContext.NO_NEXT:
                    break

                aN = aO

    def getParamIndex(self, paramId):
        for i in range(0, len(self.paramIdList), 1):
            p = self.paramIdList[i]
            if p == paramId:
                return i

        return self.extendAndAddParam(paramId, 0, ModelContext.PARAM_FLOAT_MIN, ModelContext.PARAM_FLOAT_MAX)

    def getDeformerIndex(self, aH):
        for aI in range(len(self.deformerList) - 1, 0 - 1, -1):
            if self.deformerList[aI] is not None and self.deformerList[aI].getId() == aH:
                return aI

        return -1

    def extendAndAddParam(self, param_id, default_val, max_val, min_val):
        self.paramIdList.append(param_id)
        self.paramValues.append(default_val)
        self.lastParamValues.append(default_val)
        self.paramMinValues.append(max_val)
        self.paramMaxValues.append(min_val)
        self.updatedParamFlags.append(ModelContext.DEFAULT_PARAM_UPDATE_FLAG)
        ret = self.nextParamPos
        self.nextParamPos += 1
        return ret

    def setDeformer(self, aI, aH):
        self.deformerList[aI] = aH

    def setParamFloat(self, aH, aI):
        if aI < self.paramMinValues[aH]:
            aI = self.paramMinValues[aH]

        if aI > self.paramMaxValues[aH]:
            aI = self.paramMaxValues[aH]

        self.paramValues[aH] = aI

    def loadParam(self):
        for idx, i in enumerate(self.savedParamValues):
            self.paramValues[idx] = i

    def saveParam(self):
        size = len(self.savedParamValues)
        for idx, i in enumerate(self.paramValues):
            if idx >= size:
                self.savedParamValues.append(i)
                continue

            self.savedParamValues[idx] = i

    def getInitVersion(self):
        return self.initVersion

    def requireSetup(self) -> bool:
        return self.needSetup

    def isParamUpdated(self, index) -> bool:
        return self.updatedParamFlags[index] == ModelContext.PARAM_UPDATED

    def getTempPivotTableIndices(self):
        return self.tmpPivotTableIndices

    def getTempT(self):
        return self.tempTArray

    def getDeformer(self, aH):
        return self.deformerList[aH]

    def getParamFloat(self, aH):
        return self.paramValues[aH]

    def getParamMax(self, aH):
        return self.paramMaxValues[aH]

    def getParamMin(self, aH):
        return self.paramMinValues[aH]

    def setPartsOpacity(self, aJ, aH):
        aI = self.partsContextList[aJ]
        aI.setPartsOpacity(aH)

    def getPartsOpacity(self, aI):
        aH = self.partsContextList[aI]
        return aH.getPartsOpacity()

    def getPartsDataIndex(self, aI):
        for aH in range(len(self.partsDataList) - 1, 0 - 1, -1):
            if self.partsDataList[aH] is not None and self.partsDataList[aH].getId() == aI:
                return aH

        return -1

    def getDeformerContext(self, aH):
        return self.deformerContextList[aH]

    def getDrawContext(self, aH) -> 'MeshContext':
        return self.drawContextList[aH]

    def getPartsContext(self, aH) -> 'PartsDataContext':
        return self.partsContextList[aH]

    def setPartMultiplyColor(self, aH, r, g, b, a):
        aI = self.partsContextList[aH]
        aI.multiplyColor[0] = r
        aI.multiplyColor[1] = g
        aI.multiplyColor[2] = b
        aI.multiplyColor[3] = a

    def getPartMultiplyColor(self, aH):
        return self.partsContextList[aH].multiplyColor

    def setPartScreenColor(self, aH, r, g, b, a):
        aI = self.partsContextList[aH]
        aI.screenColor[0] = r
        aI.screenColor[1] = g
        aI.screenColor[2] = b
        aI.screenColor[3] = a

    def getPartScreenColor(self, aH):
        return self.partsContextList[aH].screenColor

    def setDrawParam(self, aH):
        self.dpGL = aH

    def getDrawParam(self):
        return self.dpGL
