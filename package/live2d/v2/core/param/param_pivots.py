from ..io.iserializable import ISerializable


class ParamPivots(ISerializable):
    PARAM_INDEX_NOT_INIT = -2

    def __init__(self):
        self.pivotCount = 0
        self.paramId = None
        self.pivotValues = None
        self.paramIndex = ParamPivots.PARAM_INDEX_NOT_INIT
        self.initVersion = -1
        self.tmpPivotIndex = 0
        self.tmpT = 0

    def read(self, aH):
        self.paramId = aH.readObject()
        self.pivotCount = aH.readInt32()
        self.pivotValues = aH.readObject()

    def getParamIndex(self, initVersion):
        if self.initVersion != initVersion:
            self.paramIndex = ParamPivots.PARAM_INDEX_NOT_INIT

        return self.paramIndex

    def Pb_(self, aI, aH):
        self.paramIndex = aI
        self.initVersion = aH

    def getParamID(self):
        return self.paramId

    def setParamId(self, aH):
        self.paramId = aH

    def getPivotCount(self):
        return self.pivotCount

    def getPivotValues(self):
        return self.pivotValues

    def setPivotValues(self, count, values):
        self.pivotCount = count
        self.pivotValues = values

    def getTmpPivotIndex(self):
        return self.tmpPivotIndex

    def setTmpPivotIndex(self, index):
        self.tmpPivotIndex = index

    def getTmpT(self):
        return self.tmpT

    def setTmpT(self, value):
        self.tmpT = value
