from abc import abstractmethod
from typing import TYPE_CHECKING, List, Optional

from ..DEF import LIVE2D_FORMAT_VERSION_V2_10_SDK2
from ..id import Id
from ..io.iserializable import ISerializable
from ..util import UtInterpolate

if TYPE_CHECKING:
    from ..io import BinaryReader
    from ..model_context import ModelContext
    from .deformer_context import DeformerContext
    from ..param import PivotManager



class Deformer(ISerializable):
    DEFORMER_INDEX_NOT_INIT = -2
    TYPE_ROTATION = 1
    TYPE_WARP = 2

    def __init__(self):
        self.id: Optional[Id] = None
        self.targetId: Optional[Id] = None
        self.dirty: bool = True
        self.pivotOpacities: Optional[List[float]] = None

    def read(self, br: 'BinaryReader'):
        self.id = br.readObject()
        self.targetId = br.readObject()

    def readOpacity(self, br: 'BinaryReader'):
        if br.getFormatVersion() >= LIVE2D_FORMAT_VERSION_V2_10_SDK2:
            self.pivotOpacities = br.readFloat32Array()

    @abstractmethod
    def init(self, mc: 'ModelContext'):
        pass

    @abstractmethod
    def setupInterpolate(self, modelContext: 'ModelContext', deformerContext: 'DeformerContext'):
        pass

    def interpolateOpacity(self, mdc, pivotMgr: 'PivotManager', bctx: 'DeformerContext', ret: List[bool]):
        if self.pivotOpacities is None:
            bctx.setInterpolatedOpacity(1)
            # raise RuntimeError
        else:
            bctx.setInterpolatedOpacity(UtInterpolate.interpolateFloat(mdc, pivotMgr, ret, self.pivotOpacities))
            # print(bctx.getInterpolatedOpacity())

    @abstractmethod
    def setupTransform(self, mc, dc) -> bool:
        pass

    @abstractmethod
    def transformPoints(self, mc: 'ModelContext', dc: 'DeformerContext',
                        srcPoints: List[float],
                        dstPoints: List[float],
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

    def getTargetId(self) -> Id:
        return self.targetId

    def getId(self) -> Id:
        return self.id

    def needTransform(self) -> bool:
        return self.targetId is not None and (self.targetId != Id.DST_BASE_ID())
