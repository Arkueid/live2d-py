from abc import ABC, abstractmethod

class IPhysicsParam(ABC):

    def __init__(self, aJ, aI, aH):
        self.paramId = aJ
        self.scale = aI
        self.weight = aH

    @abstractmethod
    def update(self, aI, aH):
        pass
