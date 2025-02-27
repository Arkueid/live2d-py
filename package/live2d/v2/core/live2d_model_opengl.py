from .alive2d_model import ALive2DModel
from .graphics import DrawParamOpenGL


class Live2DModelOpenGL(ALive2DModel):

    def __init__(self):
        super().__init__()
        self.drawParamGL: DrawParamOpenGL = DrawParamOpenGL()

    def resize(self, ww: int, wh: int):
        self.drawParamGL.resize(ww, wh)

    def update(self):
        self.modelContext.update()
        self.modelContext.preDraw(self.drawParamGL)

    def draw(self):
        self.modelContext.draw(self.drawParamGL)

    def getDrawParam(self):
        return self.drawParamGL

    def setMatrix(self, aH):
        self.drawParamGL.setMatrix(aH)

    @staticmethod
    def loadModel(aI):
        aH = Live2DModelOpenGL()
        ALive2DModel.loadModel_exe(aH, aI)
        return aH

    def setTexture(self, aI, aH):
        if self.drawParamGL is None:
            raise RuntimeError("current gl is none")

        self.drawParamGL.setTexture(aI, aH)
