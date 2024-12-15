from .id import ID


class ParamID(ID):
    instances = {}

    def __init__(self, aH):
        super().__init__(aH)

    @staticmethod
    def releaseStored():
        ParamID.instances.clear()

    @staticmethod
    def getID(idStr: str) -> 'ParamID':
        id_obj = ParamID.instances.get(idStr, None)
        if id_obj is None:
            id_obj = ParamID(idStr)
            ParamID.instances[idStr] = id_obj

        return id_obj
