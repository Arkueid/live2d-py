class Motion:
    MOTION_TYPE_PARAM = 0
    MOTION_TYPE_PARTS_VISIBLE = 1
    MOTION_TYPE_LAYOUT_X = 100
    MOTION_TYPE_LAYOUT_Y = 101
    MOTION_TYPE_LAYOUT_ANCHOR_X = 102
    MOTION_TYPE_LAYOUT_ANCHOR_Y = 103
    MOTION_TYPE_LAYOUT_SCALE_X = 104
    MOTION_TYPE_LAYOUT_SCALE_Y = 105

    def __init__(self):
        self.paramIdStr = None
        self.values = None
        self.mtnType = None
