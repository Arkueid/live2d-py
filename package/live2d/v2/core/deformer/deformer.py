from abc import abstractmethod
from typing import TYPE_CHECKING

from ..DEF import LIVE2D_FORMAT_VERSION_V2_10_SDK2
from ..id import BaseDataID
from ..io.iserializable import ISerializable
from ..util import UtInterpolate

if TYPE_CHECKING:
    from ..model_context import ModelContext
    from .deformer_context import DeformerContext


class Deformer(ISerializable):
    DEFORMER_INDEX_NOT_INIT = -2
    TYPE_ROTATION = 1
    TYPE_WARP = 2

    def __init__(self):
        self.id = None
        self.targetId = None
        self.dirty = True
        self.pivotOpacities = None

    def read(self, br):
        self.id = br.readObject()
        self.targetId = br.readObject()

    def readOpacity(self, br):
        if br.getFormatVersion() >= LIVE2D_FORMAT_VERSION_V2_10_SDK2:
            self.pivotOpacities = br.readFloat32Array()

    @abstractmethod
    def init(self, mc: 'ModelContext'):
        pass

    @abstractmethod
    def setupInterpolate(self, modelContext: 'ModelContext', deformerContext: 'DeformerContext'):
        pass

    def interpolateOpacity(self, aJ, aK, aI, aH):
        if self.pivotOpacities is None:
            aI.setInterpolatedOpacity(1)
        else:
            aI.setInterpolatedOpacity(UtInterpolate.interpolateFloat(aJ, aK, aH, self.pivotOpacities))

    @abstractmethod
    def setupTransform(self, mc, dc) -> bool:
        pass

    @abstractmethod
    def transformPoints(self, mc: 'ModelContext', dc: 'DeformerContext',
                        srcPoints: list[float],
                        dstPoints: list[float],
                        numPoint: int,
                        ptOffset: int,
                        ptStep: int):
        pass

    @abstractmethod
    def getType(self) -> int:
        pass

    def setTargetId(self, aH):
        self.targetId = aH

    def setId(self, aH):
        self.id = aH

    def getTargetId(self) -> BaseDataID:
        return self.targetId

    def getId(self) -> BaseDataID:
        return self.id

    def needTransform(self) -> bool:
        return self.targetId is not None and (self.targetId != BaseDataID.DST_BASE_ID())
