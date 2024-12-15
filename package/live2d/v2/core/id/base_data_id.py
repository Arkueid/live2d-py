from .id import ID


class BaseDataID(ID):
    DST_BASE = None
    instances = {}

    def __init__(self, idStr: str):
        super().__init__(idStr)

    @staticmethod
    def DST_BASE_ID() -> 'BaseDataID':
        if BaseDataID.DST_BASE is None:
            BaseDataID.DST_BASE = BaseDataID.getID("DST_BASE")

        return BaseDataID.DST_BASE

    @staticmethod
    def releaseStored():
        BaseDataID.instances.clear()
        BaseDataID.DST_BASE = None

    @staticmethod
    def getID(idStr) -> 'BaseDataID':
        id_obj = BaseDataID.instances.get(idStr, None)
        if id_obj is None:
            id_obj = BaseDataID(idStr)
            BaseDataID.instances[idStr] = id_obj

        return id_obj
