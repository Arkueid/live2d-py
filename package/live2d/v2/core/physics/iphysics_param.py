from abc import ABC, abstractmethod

class IPhysicsParam(ABC):

    def __init__(self, paramId, scale, weight):
        self.paramId = paramId
        self.scale = scale
        self.weight = weight

    @abstractmethod
    def update(self, aI, aH):
        pass
