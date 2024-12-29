from .id import Id


class PartsDataId(Id):
    instances = {}

    def __init__(self, aH):
        super().__init__(aH)

    @staticmethod
    def releaseStored():
        PartsDataId.instances.clear()

    @staticmethod
    def getID(idStr: str) -> 'PartsDataId':
        id_obj = PartsDataId.instances.get(idStr, None)
        if id_obj is None:
            id_obj = PartsDataId(idStr)
            PartsDataId.instances[idStr] = id_obj

        return id_obj
