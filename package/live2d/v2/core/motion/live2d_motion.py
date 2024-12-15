from ..type import Array
from ..util import UtString
from .amotion import AMotion
from .motion import Motion


class Live2DMotion(AMotion):
    MTN_PREFIX_VISIBLE = "VISIBLE:"
    MTN_PREFIX_LAYOUT = "LAYOUT:"
    MTN_PREFIX_FADE_IN = "FADEIN:"
    MTN_PREFIX_FADE_OUT = "FADEOUT:"

    def __init__(self):

        super().__init__()
        self.motions = Array()
        self.srcFps = 30
        self.maxLength = 0
        self.loop = False
        self.loopFadeIn = True
        self.loopDurationMSec = -1
        self.lastWeight = 0

    def getDurationMSec(self):
        return -1 if self.loop else self.loopDurationMSec

    def getLoopDurationMSec(self):
        return self.loopDurationMSec

    def updateParamExe(self, aJ, aN, aQ, a3):
        aO = aN - a3.startTimeMSec
        a0 = aO * self.srcFps / 1000
        aK = int(a0)
        aR = a0 - aK
        for aZ in range(0, len(self.motions), 1):
            aV = self.motions[aZ]
            aL = len(aV.values)
            aT = aV.paramIdStr
            if aV.mtnType == Motion.MOTION_TYPE_PARTS_VISIBLE:
                aX = aV.values[(aL - 1 if aK >= aL else aK)]
                aJ.setParamFloat(aT, aX)
            else:
                if Motion.MOTION_TYPE_LAYOUT_X <= aV.mtnType <= Motion.MOTION_TYPE_LAYOUT_SCALE_Y:
                    pass
                else:
                    aH = aJ.getParamIndex(aT)
                    a4 = aJ.getModelContext()
                    aY = a4.getParamMax(aH)
                    aW = a4.getParamMin(aH)
                    aM = 0.4
                    aS = aM * (aY - aW)
                    aU = a4.getParamFloat(aH)
                    a2 = aV.values[(aL - 1 if aK >= aL else aK)]
                    a1 = aV.values[(aL - 1 if aK + 1 >= aL else aK + 1)]
                    if (a2 < a1 and a1 - a2 > aS) or (a2 > a1 and a2 - a1 > aS):
                        aI = a2
                    else:
                        aI = a2 + (a1 - a2) * aR

                    aP = aU + (aI - aU) * aQ
                    aJ.setParamFloat(aT, aP)

        if aK >= self.maxLength:
            if self.loop:
                a3.startTimeMSec = aN
                if self.loopFadeIn:
                    a3.fadeInStartTimeMSec = aN
            else:
                a3.finished = True

        self.lastWeight = aQ

    def isLoop(self):
        return self.loop

    def setLoop(self, aH):
        self.loop = aH

    def isLoopFadeIn(self):
        return self.loopFadeIn

    def setLoopFadeIn(self, value):
        self.loopFadeIn = value

    @staticmethod
    def loadMotion(aT: bytes):
        mtn = Live2DMotion()
        aI = [0]
        aQ = len(aT)
        mtn.maxLength = 0
        aJ = 0
        while aJ < aQ:
            aL = aT[aJ]
            aS = chr(aL)
            if aS == "\n" or aS == "\r":
                aJ += 1
                continue

            if aS == "#":
                while aJ < aQ:
                    if chr(aT[aJ]) == "\n" or chr(aT[aJ]) == "\r":
                        break
                    aJ += 1

                aJ += 1
                continue

            if aS == "":
                aV = aJ
                aK = -1
                while aJ < aQ:
                    aS = chr(aT[aJ])
                    if aS == "\r" or aS == "\n":
                        break

                    if aS == "=":
                        aK = aJ
                        break
                    aJ += 1

                aP = False
                if aK >= 0:
                    if aK == aV + 4 and chr(aT[aV + 1]) == "f" and chr(aT[aV + 2]) == "p" and chr(aT[aV + 3]) == "s":
                        aP = True

                    aJ = aK + 1
                    while aJ < aQ:
                        aS = chr(aT[aJ])
                        if aS == "\r" or aS == "\n":
                            break

                        if aS == "," or aS == " " or aS == "\t":
                            aJ += 1
                            continue

                        aM = UtString.strToFloat(aT, aQ, aJ, aI)
                        if aI[0] > 0:
                            if aP and 5 < aM < 121:
                                mtn.srcFps = aM

                        aJ = aI[0]
                        aJ += 1

                while aJ < aQ:
                    if chr(aT[aJ]) == "\n" or chr(aT[aJ]) == "\r":
                        break
                    aJ += 1

                aJ += 1
                continue

            if (97 <= aL <= 122) or (65 <= aL <= 90) or aS == "_":
                aV = aJ
                aK = -1
                while aJ < aQ:
                    aS = chr(aT[aJ])
                    if aS == "\r" or aS == "\n":
                        break

                    if aS == "=":
                        aK = aJ
                        break
                    aJ += 1

                if aK >= 0:
                    aO = Motion()
                    if UtString.startswith(aT, aV, Live2DMotion.MTN_PREFIX_VISIBLE):
                        aO.mtnType = Motion.MOTION_TYPE_PARTS_VISIBLE
                        aO.paramIdStr = UtString.createString(aT, aV, aK - aV)
                    else:
                        if UtString.startswith(aT, aV, Live2DMotion.MTN_PREFIX_LAYOUT):
                            aO.paramIdStr = UtString.createString(aT, aV + 7, aK - aV - 7)
                            if UtString.startswith(aT, aV + 7, "ANCHOR_X"):
                                aO.mtnType = Motion.MOTION_TYPE_LAYOUT_ANCHOR_X
                            else:
                                if UtString.startswith(aT, aV + 7, "ANCHOR_Y"):
                                    aO.mtnType = Motion.MOTION_TYPE_LAYOUT_ANCHOR_Y
                                else:
                                    if UtString.startswith(aT, aV + 7, "SCALE_X"):
                                        aO.mtnType = Motion.MOTION_TYPE_LAYOUT_SCALE_X
                                    else:
                                        if UtString.startswith(aT, aV + 7, "SCALE_Y"):
                                            aO.mtnType = Motion.MOTION_TYPE_LAYOUT_SCALE_Y
                                        else:
                                            if UtString.startswith(aT, aV + 7, "AffineEnt"):
                                                aO.mtnType = Motion.MOTION_TYPE_LAYOUT_X
                                            else:
                                                if UtString.startswith(aT, aV + 7, "Y"):
                                                    aO.mtnType = Motion.MOTION_TYPE_LAYOUT_Y
                        else:
                            aO.mtnType = Motion.MOTION_TYPE_PARAM
                            aO.paramIdStr = UtString.createString(aT, aV, aK - aV)

                    mtn.motions.append(aO)
                    aU = 0
                    aR = []
                    aJ = aK + 1
                    while aJ < aQ:
                        aS = chr(aT[aJ])
                        if aS == "\r" or aS == "\n":
                            break

                        if aS == "," or aS == " " or aS == "\t":
                            aJ += 1
                            continue

                        aM = UtString.strToFloat(aT, aQ, aJ, aI)
                        if aI[0] > 0:
                            aR.append(aM)
                            aU += 1
                            aH = aI[0]
                            if aH < aJ:
                                print("invalid state during loadMotion\n")
                                break

                            aJ = aH - 1
                        aJ += 1

                    aO.values = aR
                    if aU > mtn.maxLength:
                        mtn.maxLength = aU

            aJ += 1

        mtn.loopDurationMSec = int((1000 * mtn.maxLength) / mtn.srcFps)
        return mtn
