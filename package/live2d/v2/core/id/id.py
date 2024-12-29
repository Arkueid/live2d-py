import abc


class Id(abc.ABC):

    def __init__(self, aH):
        self.id = aH

    def __str__(self):
        return self.id

    def __eq__(self, other):
        if isinstance(other, Id):
            return id(other) == id(self) or other.id == self.id
        elif isinstance(other, str):
            return other == self.id
        return False

    @staticmethod
    @abc.abstractmethod
    def getID(idStr: str) -> 'Id':
        pass

    @staticmethod
    @abc.abstractmethod
    def releaseStored():
        pass
