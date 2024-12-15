from ..io.iserializable import ISerializable

class ParamDefFloat(ISerializable):

    def __init__(self):
        self.minValue = None
        self.maxValue = None
        self.defaultValue = None
        self.paramId = None

    def read(self, br):
        self.minValue = br.readFloat32()
        self.maxValue = br.readFloat32()
        self.defaultValue = br.readFloat32()
        self.paramId = br.readObject()

    def getMinValue(self) -> float:
        return self.minValue

    def getMaxValue(self) -> float:
        return self.maxValue

    def getDefaultValue(self) -> float:
        return self.defaultValue

    def getParamID(self):
        return self.paramId
