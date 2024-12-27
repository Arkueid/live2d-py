from .iphysics_param import IPhysicsParam
from .physics_constants import SRC_TO_Y, SRC_TO_G_ANGLE, SRC_TO_X


class PhysicsSrc(IPhysicsParam):

    def __init__(self, aJ, aK, aI, aH):
        super().__init__(aK, aI, aH)
        self.tL_ = None
        self.tL_ = aJ

    def update(self, aJ, aH):
        aK = self.scale * aJ.getParamFloat(self.paramId)
        aL = aH.getPhysicsPoint1()

        switch = self.tL_
        if switch == SRC_TO_X:
            aL.x = aL.x + (aK - aL.x) * self.weight
        elif switch == SRC_TO_Y:
            aL.y = aL.y + (aK - aL.y) * self.weight
        elif switch == SRC_TO_G_ANGLE:
            aI = aH.qr_()
            aI = aI + (aK - aI) * self.weight
            aH.pr_(aI)
