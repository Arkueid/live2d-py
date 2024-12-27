from .alive2d_model import ALive2DModel
from .graphics import DrawParamOpenGL


class Live2DModelOpenGL(ALive2DModel):

    def __init__(self):
        super().__init__()
        self.drawParamGL = DrawParamOpenGL()

    def resize(self, ww, wh):
        self.drawParamGL.resize(ww, wh)

    def setTransform(self, aH):
        self.drawParamGL.setTransform(aH)

    def update(self):
        self.modelContext.update()
        self.modelContext.preDraw(self.drawParamGL)

    def draw(self):
        self.modelContext.draw(self.drawParamGL)

    def getDrawParam(self):
        return self.drawParamGL

    def setMatrix(self, aH):
        self.drawParamGL.setMatrix(aH)

    def setPremultipliedAlpha(self, aH):
        self.drawParamGL.setPremultipliedAlpha(aH)

    def isPremultipliedAlpha(self):
        return self.drawParamGL.isPremultipliedAlpha()

    def setAnisotropy(self, aH):
        self.drawParamGL.setAnisotropy(aH)

    def getAnisotropy(self):
        return self.drawParamGL.getAnisotropy()

    @staticmethod
    def loadModel(aI):
        aH = Live2DModelOpenGL()
        ALive2DModel.loadModel_exe(aH, aI)
        return aH

    def setTexture(self, aI, aH):
        if self.drawParamGL is None:
            raise RuntimeError("current gl is none")

        self.drawParamGL.setTexture(aI, aH)
