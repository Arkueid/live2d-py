from .deformer import Deformer
from .deformer_context import DeformerContext


class WarpContext(DeformerContext):

    def __init__(self, aH):
        super().__init__(aH)

        self.tmpDeformerIndex = Deformer.DEFORMER_INDEX_NOT_INIT
        self.interpolatedPoints = None
        self.transformedPoints = None
