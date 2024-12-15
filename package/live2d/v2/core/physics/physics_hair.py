import math

from ..type import Array
from ..util import UtMath
from .physics_constants import *
from .physics_point import PhysicsPoint
from .physics_src import PhysicsSrc
from .physics_target import PhysicsTarget


class PhysicsHair:
    SRC_TO_X = SRC_TO_X
    SRC_TO_Y = SRC_TO_Y
    SRC_TO_G_ANGLE = SRC_TO_G_ANGLE

    TARGET_FROM_ANGLE = TARGET_FROM_ANGLE
    TARGET_FROM_ANGLE_V = TARGET_FROM_ANGLE_V

    def __init__(self):
        self.p1 = PhysicsPoint()
        self.p2 = PhysicsPoint()
        self.Fo_ = 0
        self.Db_ = 0
        self.L2_ = 0
        self.M2_ = 0
        self.ks_ = 0
        self._9b = 0
        self.iP_ = 0
        self.iT_ = 0
        self.lL_ = Array()
        self.qP_ = Array()
        self.setup(0.3, 0.5, 0.1)

    def setup(self, aJ=None, aI=None, aH=None):
        self.ks_ = self.Yb_()
        self.p2.setupLast()
        if aH is not None:
            self.Fo_ = aJ
            self.L2_ = aI
            self.p1.mass = aH
            self.p2.mass = aH
            self.p2.y = aJ
            self.setup()

    def getPhysicsPoint1(self):
        return self.p1

    def getPhysicsPoint2(self):
        return self.p2

    def qr_(self):
        return self.Db_

    def pr_(self, aH):
        self.Db_ = aH

    def _5r(self):
        return self.M2_

    def Cs_(self):
        return self._9b

    def Yb_(self):
        return -180 * (math.atan2(self.p1.x - self.p2.x, -(self.p1.y - self.p2.y))) / math.pi

    def addSrcParam(self, aJ, aH, aL, aI):
        aK = PhysicsSrc(aJ, aH, aL, aI)
        self.lL_.append(aK)

    def addTargetParam(self, aJ, aH, aK, aI):
        aL = PhysicsTarget(aJ, aH, aK, aI)
        self.qP_.append(aL)

    def update(self, aI, aL):
        if self.iP_ == 0:
            self.iP_ = self.iT_ = aL
            self.Fo_ = (math.sqrt((self.p1.x - self.p2.x) * (self.p1.x - self.p2.x) + (self.p1.y - self.p2.y) * (
                    self.p1.y - self.p2.y)))
            return

        aK = (aL - self.iT_) / 1000
        if aK != 0:
            for aJ in range(len(self.lL_) - 1, 0 - 1, -1):
                aM = self.lL_[aJ]
                aM.update(aI, self)

            self.oo_(aI, aK)
            self.M2_ = self.Yb_()
            self._9b = (self.M2_ - self.ks_) / aK
            self.ks_ = self.M2_

        for aJ in range(len(self.qP_) - 1, 0 - 1, -1):
            aH = self.qP_[aJ]
            aH.update(aI, self)

        self.iT_ = aL

    def oo_(self, aN, aI):
        if aI < 0.033:
            aI = 0.033

        aU = 1 / aI
        self.p1.vx = (self.p1.x - self.p1.lastX) * aU
        self.p1.vy = (self.p1.y - self.p1.lastY) * aU
        self.p1.ax = (self.p1.vx - self.p1.lastVX) * aU
        self.p1.ay = (self.p1.vy - self.p1.lastVY) * aU
        self.p1.fx = self.p1.ax * self.p1.mass
        self.p1.fy = self.p1.ay * self.p1.mass
        self.p1.setupLast()
        aM = -(math.atan2((self.p1.y - self.p2.y), self.p1.x - self.p2.x))
        aR = math.cos(aM)
        aH = math.sin(aM)
        aW = 9.8 * self.p2.mass
        aQ = (self.Db_ * UtMath.DEG_TO_RAD)
        aP = (aW * math.cos(aM - aQ))
        aL = (aP * aH)
        aV = (aP * aR)
        aK = (-self.p1.fx * aH * aH)
        aT = (-self.p1.fy * aH * aR)
        aJ = (-self.p2.vx * self.L2_)
        aS = (-self.p2.vy * self.L2_)
        self.p2.fx = (aL + aK + aJ)
        self.p2.fy = (aV + aT + aS)
        self.p2.ax = self.p2.fx / self.p2.mass
        self.p2.ay = self.p2.fy / self.p2.mass
        self.p2.vx += self.p2.ax * aI
        self.p2.vy += self.p2.ay * aI
        self.p2.x += self.p2.vx * aI
        self.p2.y += self.p2.vy * aI
        aO = (math.sqrt(
            (self.p1.x - self.p2.x) * (self.p1.x - self.p2.x) + (self.p1.y - self.p2.y) * (self.p1.y - self.p2.y)))
        self.p2.x = self.p1.x + self.Fo_ * (self.p2.x - self.p1.x) / aO
        self.p2.y = self.p1.y + self.Fo_ * (self.p2.y - self.p1.y) / aO
        self.p2.vx = (self.p2.x - self.p2.lastX) * aU
        self.p2.vy = (self.p2.y - self.p2.lastY) * aU
        self.p2.setupLast()
