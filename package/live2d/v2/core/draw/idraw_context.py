from typing import TYPE_CHECKING

if TYPE_CHECKING:
    from .mesh import Mesh

class IDrawContext:

    def __init__(self, dd):
        self.interpolatedDrawOrder = None
        self.paramOutside = [False]
        self.partsOpacity = 0
        self.available = True
        self.baseOpacity = 1
        self.clipBufPre_clipContext = None
        self.drawData = dd
        self.partsIndex = -1

    def isParamOutside(self):
        return self.paramOutside[0]

    def isAvailable(self):
        return self.available and not self.paramOutside[0]

    def getDrawData(self) -> 'Mesh':
        return self.drawData
