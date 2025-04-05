from abc import abstractmethod

from ..DEF import LIVE2D_FORMAT_VERSION_AVAILABLE
from ..id import Id
from ..io.iserializable import ISerializable
from ..live2d import Live2D
from ..util import UtInterpolate
from typing import TYPE_CHECKING

if TYPE_CHECKING:
    from  .mesh_context import MeshContext


class IDrawData(ISerializable):
    DEFORMER_INDEX_NOT_INIT = -2
    DEFAULT_ORDER = 500
    TYPE_MESH = 2
    totalMinOrder = DEFAULT_ORDER
    totalMaxOrder = DEFAULT_ORDER

    def __init__(self):
        super().__init__()
        self.clipIDList = None
        self.clipID = None
        self.id = None
        self.targetId = None
        self.pivotMgr = None
        self.averageDrawOrder = None
        self.pivotDrawOrders = None
        self.pivotOpacities = None

    def read(self, aH):
        self.id = aH.readObject()
        self.targetId = aH.readObject()
        self.pivotMgr = aH.readObject()
        self.averageDrawOrder = aH.readInt32()
        self.pivotDrawOrders = aH.readInt32Array()
        self.pivotOpacities = aH.readFloat32Array()
        if aH.getFormatVersion() >= LIVE2D_FORMAT_VERSION_AVAILABLE:
            self.clipID = aH.readObject()
            self.clipIDList = self.convertClipIDForV2_11(self.clipID)
        else:
            self.clipIDList = None

        IDrawData.setDrawOrders(self.pivotDrawOrders)

    def getClipIDList(self):
        return self.clipIDList

    @staticmethod
    def convertClipIDForV2_11(s):
        ls = []
        if s is None:
            return None

        if len(s.id) == 0:
            return None

        if not s.id.find(','):
            ls.append(s.id)
            return ls

        ls = s.id.split(",")
        return ls

    def setupInterpolate(self, aI, aH: 'MeshContext'):
        aH.paramOutside = [False]
        aH.interpolatedDrawOrder = UtInterpolate.interpolateInt(aI, self.pivotMgr, aH.paramOutside,
                                                                self.pivotDrawOrders)
        if not Live2D.L2D_OUTSIDE_PARAM_AVAILABLE and aH.paramOutside[0]:
            return

        aH.interpolatedOpacity = UtInterpolate.interpolateFloat(aI, self.pivotMgr, aH.paramOutside, self.pivotOpacities)

    @abstractmethod
    def setupTransform(self, mc, dc=None):
        pass

    def getId(self):
        return self.id

    def setId(self, value):
        self.id = value

    @staticmethod
    def getOpacity(ctx):
        return ctx.interpolatedOpacity

    @staticmethod
    def getDrawOrder(ctx):
        return ctx.interpolatedDrawOrder

    def getTargetId(self):
        return self.targetId

    def setTargetId(self, aH):
        self.targetId = aH

    def needTransform(self):
        return self.targetId is not None and (self.targetId != Id.DST_BASE_ID())

    @abstractmethod
    def getType(self):
        pass

    @staticmethod
    def setDrawOrders(orders):
        for i in range(len(orders) - 1, 0 - 1, -1):
            order = orders[i]
            if order < IDrawData.totalMinOrder:
                IDrawData.totalMinOrder = order
            else:
                if order > IDrawData.totalMaxOrder:
                    IDrawData.totalMaxOrder = order

    @staticmethod
    def getTotalMinOrder():
        return IDrawData.totalMinOrder

    @staticmethod
    def getTotalMaxOrder():
        return IDrawData.totalMaxOrder
