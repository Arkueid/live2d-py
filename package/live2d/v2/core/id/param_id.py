from .id import Id


class ParamId(Id):
    instances = {}

    def __init__(self, aH):
        super().__init__(aH)

    @staticmethod
    def releaseStored():
        ParamId.instances.clear()

    @staticmethod
    def getID(idStr: str) -> 'ParamId':
        id_obj = ParamId.instances.get(idStr, None)
        if id_obj is None:
            id_obj = ParamId(idStr)
            ParamId.instances[idStr] = id_obj

        return id_obj
