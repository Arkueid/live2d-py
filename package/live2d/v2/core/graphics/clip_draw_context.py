from typing import TYPE_CHECKING
if TYPE_CHECKING:
    from ..id import Id

class ClipDrawContext:

    def __init__(self, aI: 'Id', aH: int):
        self.drawDataId = aI
        self.drawDataIndex = aH
