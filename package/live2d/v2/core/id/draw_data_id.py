from .id import Id


class DrawDataId(Id):
    instances = {}

    def __init__(self, aH):
        super().__init__(aH)

    @staticmethod
    def releaseStored():
        DrawDataId.instances.clear()

    @staticmethod
    def getID(idStr) -> 'DrawDataId':
        id_obj = DrawDataId.instances.get(idStr, None)
        if id_obj is None:
            id_obj = DrawDataId(idStr)
            DrawDataId.instances[idStr] = id_obj

        return id_obj
