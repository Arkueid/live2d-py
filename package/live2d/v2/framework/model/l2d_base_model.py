from typing import Dict, Union, Optional

from ...core import Live2DModelOpenGL, Live2DMotion
from ..Live2DFramework import Live2DFramework
from ..matrix import L2DModelMatrix
from ..motion import L2DExpressionMotion, L2DMotionManager
from ..physics import L2DPhysics
from ..pose import L2DPose


class L2DBaseModel:
    texCount = 0

    def __init__(self):
        self.live2DModel: Optional[Live2DModelOpenGL] = None
        self.modelMatrix: Optional[L2DModelMatrix] = None
        self.eyeBlink = None
        self.physics = None
        self.pose: Union[None, L2DPose] = None
        self.debugMode = False
        self.initialized = False
        self.updating = False
        self.alpha = 1
        self.accAlpha = 0
        self.accelX = 0
        self.accelY = 0
        self.accelZ = 0
        self.dragX = 0
        self.dragY = 0
        self.startTimeMSec = 0
        self.mainMotionManager = L2DMotionManager()
        self.expressionManager = L2DMotionManager()
        self.motions = {}
        self.expressions: Dict = {}
        self.isTexLoaded = False

    def getModelMatrix(self):
        return self.modelMatrix

    def setAlpha(self, a):
        if a > 0.999:
            a = 1
        if a < 0.001:
            a = 0
        self.alpha = a

    def getAlpha(self):
        return self.alpha

    def isInitialized(self):
        return self.initialized

    def setInitialized(self, v):
        self.initialized = v

    def isUpdating(self):
        return self.updating

    def setUpdating(self, v):
        self.updating = v

    def getLive2DModel(self):
        return self.live2DModel

    def setLipSync(self, v):
        self.lipSync = v

    def setLipSyncValue(self, v):
        self.lipSyncValue = v

    def setAccel(self, x, y, z):
        self.accelX = x
        self.accelY = y
        self.accelZ = z

    def setDrag(self, x, y):
        self.dragX = x
        self.dragY = y

    def getMainMotionManager(self):
        return self.mainMotionManager

    def getExpressionManager(self):
        return self.expressionManager

    def loadModelData(self, path):
        pm = Live2DFramework.getPlatformManager()
        if self.debugMode:
            pm.log("Load model : " + path)

        self.live2DModel = pm.loadLive2DModel(path)
        self.live2DModel.saveParam()

        self.modelMatrix = L2DModelMatrix(self.live2DModel.getCanvasWidth(),
                                          self.live2DModel.getCanvasHeight())
        self.modelMatrix.setWidth(2)
        self.modelMatrix.setCenterPosition(0, 0)

        return self.live2DModel

    def loadTexture(self, no, path):
        self.texCount += 1
        pm = Live2DFramework.getPlatformManager()
        if self.debugMode:
            pm.log("Load Texture : " + path)

        pm.loadTexture(self.live2DModel, no, path)

        self.texCount -= 1
        if self.texCount == 0:
            self.isTexLoaded = True

    def loadMotion(self, name, path):
        pm = Live2DFramework.getPlatformManager()
        if self.debugMode:
            pm.log("Load Motion : " + path)

        buf = pm.loadBytes(path)

        motion = Live2DMotion.loadMotion(buf)
        if name is not None:
            self.motions[name] = motion
        return motion

    def loadExpression(self, name, path):
        pm = Live2DFramework.getPlatformManager()
        if self.debugMode:
            pm.log("Load Expression : " + path)

        if name is not None:
            buf = pm.loadBytes(path)
            self.expressions[name] = L2DExpressionMotion.loadJson(buf)

    def loadPose(self, path) -> L2DPose:
        pm = Live2DFramework.getPlatformManager()
        if self.debugMode:
            pm.log("Load Pose : " + path)
        buf = pm.loadBytes(path)
        self.pose = L2DPose.load(buf)
        return self.pose

    def loadPhysics(self, path):
        pm = Live2DFramework.getPlatformManager()
        if self.debugMode:
            pm.log("Load Physics : " + path)
        buf = pm.loadBytes(path)
        self.physics = L2DPhysics.load(buf)

    def hitTestSimple(self, drawID, testX, testY):
        draw_index = self.live2DModel.getDrawDataIndex(drawID)
        if draw_index < 0:
            return False
        points = self.live2DModel.getTransformedPoints(draw_index)
        left = self.live2DModel.getCanvasWidth()
        right = 0
        top = self.live2DModel.getCanvasHeight()
        bottom = 0
        for j in range(0, len(points), 2):
            x = points[j]
            y = points[j + 1]
            if x < left:
                left = x
            if x > right:
                right = x
            if y < top:
                top = y
            if y > bottom:
                bottom = y

        tx = self.modelMatrix.invertTransformX(testX)
        ty = self.modelMatrix.invertTransformY(testY)
        return left <= tx <= right and top <= ty <= bottom
