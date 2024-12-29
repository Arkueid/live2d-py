from .id import ID


class PartsDataID(ID):
    instances = {}

    def __init__(self, aH):
        super().__init__(aH)

    @staticmethod
    def releaseStored():
        PartsDataID.instances.clear()

    @staticmethod
    def getID(idStr: str) -> 'PartsDataID':
        id_obj = PartsDataID.instances.get(idStr, None)
        if id_obj is None:
            id_obj = PartsDataID(idStr)
            PartsDataID.instances[idStr] = id_obj

        return id_obj
