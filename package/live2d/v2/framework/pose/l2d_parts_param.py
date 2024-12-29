﻿from ...core import PartsDataID


class L2DPartsParam:

    def __init__(self, ppid):
        self.paramIndex = -1
        self.partsIndex = -1
        self.link = None
        self.id = ppid

    def initIndex(self, model):
        self.paramIndex = model.getParamIndex("VISIBLE:" + self.id)
        self.partsIndex = model.getPartsDataIndex(PartsDataID.getID(self.id))
        model.setParamFloat(self.paramIndex, 1)
