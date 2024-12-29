from .idraw_context import DrawContext
from .idraw_data import DrawData


class MeshContext(DrawContext):

    def __init__(self, dd):
        super().__init__(dd)
        self.tmpDeformerIndex = DrawData.DEFORMER_INDEX_NOT_INIT
        self.interpolatedPoints = None
        self.transformedPoints = None

    def getTransformedPoints(self):
        return self.transformedPoints if (self.transformedPoints is not None) else self.interpolatedPoints
