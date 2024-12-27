from abc import ABC, abstractmethod

class ISerializable(ABC):

    @abstractmethod
    def read(self, aH):
        pass
