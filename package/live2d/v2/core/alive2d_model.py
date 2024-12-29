﻿from abc import ABC, abstractmethod

from .DEF import LIVE2D_FORMAT_VERSION_AVAILABLE, LIVE2D_FORMAT_VERSION_V2_8_TEX_OPTION
from .id import ParamID, PartsDataID, DrawDataID
from .io import BinaryReader
from .model import ModelImpl
from .model_context import ModelContext

from typing import TYPE_CHECKING
if TYPE_CHECKING:
    from .draw import MeshContext, IDrawData, Mesh


class ALive2DModel(ABC):

    def __init__(self):
        self.modelImpl = None
        self.modelContext = None
        self.modelContext = ModelContext(self)

    def setModelImpl(self, aH):
        self.modelImpl = aH

    def getModelImpl(self):
        if self.modelImpl is None:
            self.modelImpl = ModelImpl()
            self.modelImpl.initDirect()

        return self.modelImpl

    def getCanvasWidth(self):
        if self.modelImpl is None:
            return 0

        return self.modelImpl.getCanvasWidth()

    def getCanvasHeight(self):
        if self.modelImpl is None:
            return 0

        return self.modelImpl.getCanvasHeight()

    def getParamFloat(self, x):
        if not isinstance(x, int):
            x = self.modelContext.getParamIndex(ParamID.getID(x))

        return self.modelContext.getParamFloat(x)

    def setParamFloat(self, x: int | str, value: float, weight: float = 1):
        if not isinstance(x, int):
            x = self.modelContext.getParamIndex(ParamID.getID(x))
        value = 0 if value is None else value
        self.modelContext.setParamFloat(x, self.modelContext.getParamFloat(x) * (1 - weight) + value * weight)

    def addToParamFloat(self, x: str | int, value: float, weight: float = 1):
        if not isinstance(x, int):
            x = self.modelContext.getParamIndex(ParamID.getID(x))

        self.modelContext.setParamFloat(x, self.modelContext.getParamFloat(x) + value * weight)

    def multParamFloat(self, x: int | str, value: float, weight: float = 1):
        if not isinstance(x, int):
            x = self.modelContext.getParamIndex(ParamID.getID(x))

        self.modelContext.setParamFloat(x, self.modelContext.getParamFloat(x) * (1 + (value - 1) * weight))

    def getParamIndex(self, idStr: str) -> int:
        return self.modelContext.getParamIndex(ParamID.getID(idStr))

    def loadParam(self):
        self.modelContext.loadParam()

    def saveParam(self):
        self.modelContext.saveParam()

    def init(self):
        self.modelContext.init()

    def update(self):
        self.modelContext.update()

    @abstractmethod
    def draw(self):
        pass

    def getModelContext(self):
        return self.modelContext

    def setPartsOpacity(self, aI, aH):
        if not isinstance(aI, int):
            aI = self.modelContext.getPartsDataIndex(PartsDataID.getID(aI))

        self.modelContext.setPartsOpacity(aI, aH)

    def getPartsDataIndex(self, aH):
        if not (isinstance(aH, PartsDataID)):
            aH = PartsDataID.getID(aH)

        return self.modelContext.getPartsDataIndex(aH)

    def getPartsOpacity(self, aH):
        if not isinstance(aH, int):
            aH = self.modelContext.getPartsDataIndex(PartsDataID.getID(aH))

        if aH < 0:
            return 0

        return self.modelContext.getPartsOpacity(aH)

    @abstractmethod
    def getDrawParam(self):
        pass

    def getDrawDataIndex(self, aH):
        return self.modelContext.getDrawDataIndex(DrawDataID.getID(aH))

    def getDrawData(self, aH):
        return self.modelContext.getDrawData(aH)

    def getTransformedPoints(self, aH):
        aI = self.modelContext.getDrawContext(aH)
        if isinstance(aI, MeshContext):
            return aI.getTransformedPoints()

        return None

    def getIndexArray(self, aI):
        if aI < 0 or aI >= len(self.modelContext.drawDataList):
            return None

        aH = self.modelContext.drawDataList[aI]
        if aH is not None and aH.getType() == IDrawData.TYPE_MESH:
            if isinstance(aH, Mesh):
                return aH.getIndexArray()

        return None

    @staticmethod
    def loadModel_exe(model, buf: bytes):

        if not (isinstance(buf, bytes)):
            raise RuntimeError("param error")

        br = BinaryReader(buf)
        magic1 = br.readByte()
        magic2 = br.readByte()
        magic3 = br.readByte()
        # magic = 'moc'
        if magic1 == 109 and magic2 == 111 and magic3 == 99:
            version = br.readByte()
        else:
            raise RuntimeError("Invalid MOC file.")

        br.setFormatVersion(version)
        if version > LIVE2D_FORMAT_VERSION_AVAILABLE:
            es = "Unsupported version %d\n" % version
            raise RuntimeError(es)

        aL = br.readObject()
        if version >= LIVE2D_FORMAT_VERSION_V2_8_TEX_OPTION:
            aH = br.readUShort()
            aT = br.readUShort()
            if aH != -30584 or aT != -30584:
                raise RuntimeError("Invalid load EOF")

        model.setModelImpl(aL)
        model_context = model.getModelContext()
        model_context.setDrawParam(model.getDrawParam())
        model_context.init()
