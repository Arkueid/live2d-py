from .id import ID


class DrawDataID(ID):
    instances = {}

    def __init__(self, aH):
        super().__init__(aH)

    @staticmethod
    def releaseStored():
        DrawDataID.instances.clear()

    @staticmethod
    def getID(idStr) -> 'DrawDataID':
        id_obj = DrawDataID.instances.get(idStr, None)
        if id_obj is None:
            id_obj = DrawDataID(idStr)
            DrawDataID.instances[idStr] = id_obj

        return id_obj
