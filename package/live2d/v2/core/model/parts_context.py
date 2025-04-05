from typing import TYPE_CHECKING, Optional, List

if TYPE_CHECKING:
    from .part import PartsData

class PartsDataContext:

    def __init__(self, parts: 'PartsData'):
        super().__init__()
        self.partsOpacity: Optional[float] = None
        self.partsData: Optional['PartsData'] = parts
        self.screenColor: List[float] = [0.0, 0.0, 0.0, 0.0]
        self.multiplyColor: List[float] = [1.0, 1.0, 1.0, 0.0]

    def getPartsOpacity(self) -> float:
        return self.partsOpacity

    def setPartsOpacity(self, value: float):
        self.partsOpacity = value

    def setPartScreenColor(self, r: float, g: float, b: float, a: float):
        self.screenColor[0] = r
        self.screenColor[1] = g
        self.screenColor[2] = b
        self.screenColor[3] = a

    def setPartMultiplyColor(self, r: float, g: float, b: float, a: float):
        self.multiplyColor[0] = r
        self.multiplyColor[1] = g
        self.multiplyColor[2] = b
        self.multiplyColor[3] = a
