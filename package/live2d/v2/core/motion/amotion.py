from abc import ABC, abstractmethod

from ..util import UtSystem
from ..util.ut_motion import UtMotion


class AMotion(ABC):

    def __init__(self):
        self.fadeInMSec = 1000
        self.fadeOutMSec = 1000
        self.weight = 1

    def setFadeIn(self, aH):
        self.fadeInMSec = aH

    def setFadeOut(self, aH):
        self.fadeOutMSec = aH

    def setWeight(self, aH):
        self.weight = aH

    def getFadeOut(self):
        return self.fadeOutMSec

    def getWeight(self):
        return self.weight

    def getDurationMSec(self):
        return -1

    def getLoopDurationMSec(self):
        return -1

    def updateParam(self, aJ, aN):
        if not aN.available or aN.finished:
            return

        aL = UtSystem.getUserTimeMSec()
        if aN.startTimeMSec < 0:
            aN.startTimeMSec = aL
            aN.fadeInStartTimeMSec = aL
            aM = self.getDurationMSec()
            if aN.endTimeMSec < 0:
                aN.endTimeMSec = -1 if (aM <= 0) else aN.startTimeMSec + aM

        aI = self.weight
        aH = 1 if (self.fadeInMSec == 0) else UtMotion.getEasingSine(((aL - aN.fadeInStartTimeMSec) / self.fadeInMSec))
        aK = 1 if (self.fadeOutMSec == 0 or aN.endTimeMSec < 0) else UtMotion.getEasingSine(((aN.endTimeMSec - aL) / self.fadeOutMSec))
        aI = aI * aH * aK
        if not (0 <= aI <= 1):
            print("### assert!! ### ")

        self.updateParamExe(aJ, aL, aI, aN)
        if 0 < aN.endTimeMSec < aL:
            aN.finished = True

    @abstractmethod
    def updateParamExe(self, aH, aI, aJ, aK):
        pass

    @staticmethod
    def getEasing(t, totalTime, accelerateTime):
        aQ = t / totalTime
        a1 = accelerateTime / totalTime
        aU = a1
        aZ = 1 / 3
        aR = 2 / 3
        a0 = 1 - (1 - a1) * (1 - a1)
        a2 = 1 - (1 - aU) * (1 - aU)
        aM = 0
        aL = ((1 - a1) * aZ) * a0 + (aU * aR + (1 - aU) * aZ) * (1 - a0)
        aK = (aU + (1 - aU) * aR) * a2 + (a1 * aZ + (1 - a1) * aR) * (1 - a2)
        aJ = 1
        aY = aJ - 3 * aK + 3 * aL - aM
        aX = 3 * aK - 6 * aL + 3 * aM
        aW = 3 * aL - 3 * aM
        aV = aM
        if aQ <= 0:
            return 0
        elif aQ >= 1:
            return 1

        aS = aQ
        aI = aS * aS
        aH = aS * aI
        aT = aY * aH + aX * aI + aW * aS + aV
        return aT
