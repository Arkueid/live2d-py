import math

from ...core import UtSystem


class L2DTargetPoint:
    FRAME_RATE = 30
    TIME_TO_MAX_SPEED = 0.15
    FACE_PARAM_MAX_V = 40.0 / 7.5
    MAX_V = FACE_PARAM_MAX_V / FRAME_RATE
    FRAME_TO_MAX_SPEED = TIME_TO_MAX_SPEED * FRAME_RATE

    def __init__(self):
        self.EPSILON = 0.01
        self.faceTargetX = 0
        self.faceTargetY = 0
        self.faceX = 0
        self.faceY = 0
        self.faceVX = 0
        self.faceVY = 0
        self.lastTimeSec = 0

    def setPoint(self, x, y):
        self.faceTargetX = x
        self.faceTargetY = y

    def getX(self):
        return self.faceX

    def getY(self):
        return self.faceY

    def update(self):

        if self.lastTimeSec == 0:
            self.lastTimeSec = UtSystem.getUserTimeMSec()
            return

        cur_time_sec = UtSystem.getUserTimeMSec()
        delta_time_weight = (cur_time_sec - self.lastTimeSec) * L2DTargetPoint.FRAME_RATE / 1000.0
        self.lastTimeSec = cur_time_sec
        max_a = delta_time_weight * self.MAX_V / self.FRAME_TO_MAX_SPEED
        dx = (self.faceTargetX - self.faceX)
        dy = (self.faceTargetY - self.faceY)
        if abs(dx) <= self.EPSILON and abs(dy) <= self.EPSILON:
            return
        d = math.sqrt(dx * dx + dy * dy)
        vx = self.MAX_V * dx / d
        vy = self.MAX_V * dy / d
        ax = vx - self.faceVX
        ay = vy - self.faceVY
        a = math.sqrt(ax * ax + ay * ay)
        if a < -max_a or a > max_a:
            ax *= max_a / a
            ay *= max_a / a

        self.faceVX += ax
        self.faceVY += ay

        max_v = 0.5 * (math.sqrt(max_a * max_a + 16 * max_a * d - 8 * max_a * d) - max_a)
        cur_v = math.sqrt(self.faceVX * self.faceVX + self.faceVY * self.faceVY)
        if cur_v > max_v:
            self.faceVX *= max_v / cur_v
            self.faceVY *= max_v / cur_v

        self.faceX += self.faceVX
        self.faceY += self.faceVY
