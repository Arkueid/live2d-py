﻿from typing import TYPE_CHECKING, Optional

if TYPE_CHECKING:
    from mesh import Mesh

class IDrawContext:

    def __init__(self, dd: 'Mesh'):
        self.interpolatedDrawOrder: Optional[int] = None
        self.paramOutside: bool = False
        self.partsOpacity: float = 0
        self.available = True
        self.baseOpacity = 1
        self.clipBufPre_clipContext = None
        self.drawData = dd
        self.partsIndex = -1

    def isParamOutside(self):
        return self.paramOutside

    def isAvailable(self):
        return self.available and not self.paramOutside

    def getDrawData(self) -> 'Mesh':
        return self.drawData
