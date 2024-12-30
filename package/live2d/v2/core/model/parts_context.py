from typing import TYPE_CHECKING, Optional
if TYPE_CHECKING:
    from .part import PartsData

class PartsDataContext:

    def __init__(self, parts):
        super().__init__()
        self.partsOpacity = None
        self.partsData: Optional['PartsData'] = parts
        self.screenColor = [0.0, 0.0, 0.0, 1.0]
        self.multiplyColor = [1.0, 1.0, 1.0, 1.0]

    def getPartsOpacity(self):
        return self.partsOpacity

    def setPartsOpacity(self, value):
        self.partsOpacity = value

    def setPartScreenColor(self, r, g, b, a):
        self.screenColor[0] = r
        self.screenColor[1] = g
        self.screenColor[2] = b
        self.screenColor[3] = a

    def setPartMultiplyColor(self, r, g, b, a):
        self.multiplyColor[0] = r
        self.multiplyColor[1] = g
        self.multiplyColor[2] = b
        self.multiplyColor[3] = a
