﻿from abc import ABC

from typing import TYPE_CHECKING

if TYPE_CHECKING:
    from live2d.core.deformer import Deformer


class DeformerContext(ABC):

    def __init__(self, deformer: 'Deformer'):
        self.partsIndex = None
        self.outsideParam = False
        self.available = True
        self.deformer = deformer
        self.totalScale = 1
        self.interpolatedOpacity = 1
        self.totalOpacity = 1

    def isAvailable(self) -> bool:
        return self.available and not self.outsideParam

    def setAvailable(self, value: bool):
        self.available = value

    def getDeformer(self):
        return self.deformer

    def setPartsIndex(self, aH):
        self.partsIndex = aH

    def getPartsIndex(self):
        return self.partsIndex

    def isOutsideParam(self):
        return self.outsideParam

    def setOutsideParam(self, value):
        self.outsideParam = value

    def getTotalScale(self):
        return self.totalScale

    def setTotalScale_notForClient(self, aH):
        self.totalScale = aH

    def getInterpolatedOpacity(self):
        return self.interpolatedOpacity

    def setInterpolatedOpacity(self, aH):
        self.interpolatedOpacity = aH

    def getTotalOpacity(self):
        return self.totalOpacity

    def setTotalOpacity(self, aH):
        self.totalOpacity = aH
