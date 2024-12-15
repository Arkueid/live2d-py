from ..io.iserializable import ISerializable


class Avatar(ISerializable):

    def __init__(self):
        self.id = None
        self.deformerList = None
        self.drawDataList = None

    def getDeformer(self):
        return self.deformerList

    def getDrawDataList(self):
        return self.drawDataList

    def read(self, br):
        self.id = br.readObject()
        self.drawDataList = br.readObject()
        self.deformerList = br.readObject()

    def replacePartsData(self, parts):
        parts.setDeformer(self.deformerList)
        parts.setDrawDataList(self.drawDataList)
        self.deformerList = None
        self.drawDataList = None
