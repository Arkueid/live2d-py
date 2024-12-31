class Id:
    __instances = {}

    def __init__(self, aH: str):
        self.id: str = aH

    def __str__(self) -> str:
        return self.id

    def __eq__(self, other) -> bool:
        if isinstance(other, Id):
            return id(other) == id(self) or other.id == self.id
        elif isinstance(other, str):
            return other == self.id
        return False

    @staticmethod
    def DST_BASE_ID() -> 'Id':
        return Id.getID("DST_BASE")

    @staticmethod
    def getID(idStr: str) -> 'Id':
        if not isinstance(idStr, str):
            raise RuntimeError
        id_obj = Id.__instances.get(idStr, None)
        if id_obj is None:
            id_obj = Id(idStr)
            Id.__instances[idStr] = id_obj
        return id_obj

    @staticmethod
    def releaseStored() -> None:
        Id.__instances.clear()
