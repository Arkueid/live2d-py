from typing import TYPE_CHECKING, Union

from ..io.iserializable import ISerializable

if TYPE_CHECKING:
    from ..id import Id


class ParamPivots(ISerializable):
    PARAM_INDEX_NOT_INIT = -2

    def __init__(self):
        self.pivotCount: int = 0
        self.paramId: 'Id' | None = None
        self.pivotValues: list[float] | None = None
        self.paramIndex: int = ParamPivots.PARAM_INDEX_NOT_INIT
        self.initVersion: int = -1
        self.tmpPivotIndex: int = 0
        self.tmpT: float = 0

    def read(self, br):
        self.paramId = br.readObject()
        self.pivotCount = br.readInt32()
        self.pivotValues = br.readObject()

    def getParamIndex(self, initVersion):
        if self.initVersion != initVersion:
            self.paramIndex = ParamPivots.PARAM_INDEX_NOT_INIT

        return self.paramIndex

    def setParamIndex(self, index: int, initVersion: int):
        self.paramIndex = index
        self.initVersion = initVersion

    def getParamID(self) -> 'Id':
        return self.paramId

    def getPivotCount(self):
        return self.pivotCount

    def getPivotValues(self):
        return self.pivotValues

    def getTmpPivotIndex(self):
        return self.tmpPivotIndex

    def setTmpPivotIndex(self, index):
        self.tmpPivotIndex = index

    def getTmpT(self) -> float:
        return self.tmpT

    def setTmpT(self, value: float):
        self.tmpT = value
