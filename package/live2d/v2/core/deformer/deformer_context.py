from abc import ABC

from typing import TYPE_CHECKING, List

if TYPE_CHECKING:
    from .deformer import Deformer


class DeformerContext(ABC):

    def __init__(self, deformer: 'Deformer'):
        self.partsIndex = None
        self.outsideParam: List[bool] = [False]
        self.available = True
        self.deformer = deformer
        self.totalScale = 1.0
        self.interpolatedOpacity = 1.0
        self.totalOpacity = 1.0

    def isAvailable(self) -> bool:
        return self.available and not self.outsideParam[0]

    def setAvailable(self, value: bool):
        self.available = value

    def getDeformer(self):
        return self.deformer

    def setPartsIndex(self, aH):
        self.partsIndex = aH

    def getPartsIndex(self):
        return self.partsIndex

    def isOutsideParam(self):
        return self.outsideParam[0]

    def setOutsideParam(self, value: bool):
        self.outsideParam[0] = value

    def getTotalScale(self):
        return self.totalScale

    def setTotalScale_notForClient(self, aH):
        self.totalScale = aH

    def getInterpolatedOpacity(self):
        return self.interpolatedOpacity

    def setInterpolatedOpacity(self, value: float):
        self.interpolatedOpacity = value

    def getTotalOpacity(self):
        return self.totalOpacity

    def setTotalOpacity(self, aH):
        self.totalOpacity = aH
