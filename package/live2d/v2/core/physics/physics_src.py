from .iphysics_param import IPhysicsParam
from .physics_constants import SRC_TO_Y, SRC_TO_G_ANGLE, SRC_TO_X


class PhysicsSrc(IPhysicsParam):

    def __init__(self, paramId, aK, scale, weight):
        super().__init__(aK, scale, weight)
        self.tL_ = None
        self.tL_ = paramId

    def update(self, aJ, aH):
        aK = self.scale * aJ.getParamFloat(self.paramId)
        aL = aH.getPhysicsPoint1()

        if self.tL_ == SRC_TO_X:
            aL.x = aL.x + (aK - aL.x) * self.weight
        elif self.tL_ == SRC_TO_Y:
            aL.y = aL.y + (aK - aL.y) * self.weight
        elif self.tL_ == SRC_TO_G_ANGLE:
            aI = aH.qr_()
            aI = aI + (aK - aI) * self.weight
            aH.pr_(aI)
