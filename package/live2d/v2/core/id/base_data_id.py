from .id import Id


class DeformerId(Id):
    DST_BASE = None
    instances = {}

    def __init__(self, idStr: str):
        super().__init__(idStr)

    @staticmethod
    def DST_BASE_ID() -> 'DeformerId':
        if DeformerId.DST_BASE is None:
            DeformerId.DST_BASE = DeformerId.getID("DST_BASE")

        return DeformerId.DST_BASE

    @staticmethod
    def releaseStored():
        DeformerId.instances.clear()
        DeformerId.DST_BASE = None

    @staticmethod
    def getID(idStr) -> 'DeformerId':
        id_obj = DeformerId.instances.get(idStr, None)
        if id_obj is None:
            id_obj = DeformerId(idStr)
            DeformerId.instances[idStr] = id_obj

        return id_obj
