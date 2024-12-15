from ..io.iserializable import ISerializable
from ..type import Array


class ParamDefSet(ISerializable):

    def __init__(self):
        self.paramDefList = None

    def getParamDefFloatList(self):
        return self.paramDefList

    def initDirect(self):
        self.paramDefList = Array()

    def read(self, br):
        self.paramDefList = br.readObject()
