from ..io.iserializable import ISerializable
from ..type import Array


class ParamDefSet(ISerializable):

    def __init__(self):
        self.paramDefList = None

    def getParamDefFloatList(self):
        return self.paramDefList

    def read(self, br):
        self.paramDefList = br.readObject()
