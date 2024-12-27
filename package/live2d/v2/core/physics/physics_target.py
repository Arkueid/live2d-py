from .physics_constants import TARGET_FROM_ANGLE, TARGET_FROM_ANGLE_V
from .iphysics_param import IPhysicsParam


class PhysicsTarget(IPhysicsParam):

    def __init__(self, aI, aK, aJ, aH):
        super().__init__(aK, aJ, aH)
        self.YP_ = aI

    def update(self, aI, aH):
        if self.YP_ == TARGET_FROM_ANGLE:
            aI.setParamFloat(self.paramId, self.scale * aH._5r(), self.weight)
        elif self.YP_ == TARGET_FROM_ANGLE_V:
            aI.setParamFloat(self.paramId, self.scale * aH.Cs_(), self.weight)
