from .idraw_context import IDrawContext
from .idraw_data import IDrawData


class MeshContext(IDrawContext):

    def __init__(self, dd):
        super().__init__(dd)
        self.tmpDeformerIndex = IDrawData.DEFORMER_INDEX_NOT_INIT
        self.interpolatedPoints = None
        self.transformedPoints = None

    def getTransformedPoints(self):
        return self.transformedPoints if (self.transformedPoints is not None) else self.interpolatedPoints
