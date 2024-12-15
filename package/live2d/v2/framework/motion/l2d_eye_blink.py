import random

from ...core import UtSystem


class L2DEyeBlink:

    def __init__(self):
        self.nextBlinkTime = None
        self.stateStartTime = None
        self.blinkIntervalMsec = None
        self.eyeState = EyeState.STATE_FIRST
        self.blinkIntervalMsec = 4000
        self.closingMotionMsec = 100
        self.closedMotionMsec = 50
        self.openingMotionMsec = 150
        self.closeIfZero = True
        self.eyeID_L = "PARAM_EYE_L_OPEN"
        self.eyeID_R = "PARAM_EYE_R_OPEN"

    def calcNextBlink(self):
        time = UtSystem.getUserTimeMSec()
        r = random.random()
        return time + r * (2 * self.blinkIntervalMsec - 1)

    def setInterval(self, blinkIntervalMsec):
        self.blinkIntervalMsec = blinkIntervalMsec

    def setEyeMotion(self, closingMotionMsec, closedMotionMsec, openingMotionMsec):
        self.closingMotionMsec = closingMotionMsec
        self.closedMotionMsec = closedMotionMsec
        self.openingMotionMsec = openingMotionMsec

    def updateParam(self, model):
        time = UtSystem.getUserTimeMSec()
        eye_param_value = 0

        _switch_2297 = self.eyeState
        if self.eyeState == EyeState.STATE_CLOSING:
            t = (time - self.stateStartTime) / self.closingMotionMsec
            if t >= 1:
                t = 1
                self.eyeState = EyeState.STATE_CLOSED
                self.stateStartTime = time

            eye_param_value = 1 - t
        elif self.eyeState == EyeState.STATE_CLOSED:
            t = (time - self.stateStartTime) / self.closedMotionMsec
            if t >= 1:
                self.eyeState = EyeState.STATE_OPENING
                self.stateStartTime = time

            eye_param_value = 0
        elif self.eyeState == EyeState.STATE_OPENING:
            t = (time - self.stateStartTime) / self.openingMotionMsec
            if t >= 1:
                t = 1
                self.eyeState = EyeState.STATE_INTERVAL
                self.nextBlinkTime = self.calcNextBlink()

            eye_param_value = t
        elif self.eyeState == EyeState.STATE_INTERVAL:
            if self.nextBlinkTime < time:
                self.eyeState = EyeState.STATE_CLOSING
                self.stateStartTime = time
            eye_param_value = 1
        else:
            self.eyeState = EyeState.STATE_INTERVAL
            self.nextBlinkTime = self.calcNextBlink()
            eye_param_value = 1
        if not self.closeIfZero:
            eye_param_value = -eye_param_value
        model.setParamFloat(self.eyeID_L, eye_param_value)
        model.setParamFloat(self.eyeID_R, eye_param_value)


class EyeState:
    STATE_FIRST = "STATE_FIRST"
    STATE_INTERVAL = "STATE_INTERVAL"
    STATE_CLOSING = "STATE_CLOSING"
    STATE_CLOSED = "STATE_CLOSED"
    STATE_OPENING = "STATE_OPENING"
