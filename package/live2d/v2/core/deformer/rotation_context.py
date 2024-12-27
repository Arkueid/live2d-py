from typing import TYPE_CHECKING

from .deformer import Deformer
from .deformer_context import DeformerContext

if TYPE_CHECKING:
    from .roation_deformer import AffineEnt


class RotationContext(DeformerContext):

    def __init__(self, aH):
        super().__init__(aH)

        self.tmpDeformerIndex = Deformer.DEFORMER_INDEX_NOT_INIT
        self.interpolatedAffine: None | AffineEnt = None
        self.transformedAffine: None | AffineEnt = None
