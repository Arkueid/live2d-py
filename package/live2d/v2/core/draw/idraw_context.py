from typing import TYPE_CHECKING, Optional, List

if TYPE_CHECKING:
    from mesh import Mesh

class IDrawContext:

    def __init__(self, dd: 'Mesh'):
        self.interpolatedDrawOrder: Optional[int] = None
        self.paramOutside: List[bool] = [False]
        self.partsOpacity: float = 0
        self.available: bool = True
        self.baseOpacity: float = 1.0
        self.clipBufPre_clipContext = None
        self.drawData: 'Mesh' = dd
        self.partsIndex = -1

    def isParamOutside(self):
        return self.paramOutside[0]

    def isAvailable(self):
        return self.available and not self.paramOutside[0]

    def getDrawData(self) -> 'Mesh':
        return self.drawData
