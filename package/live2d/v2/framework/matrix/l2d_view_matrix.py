from framework.matrix.l2d_matrix44 import L2DMatrix44


class L2DViewMatrix(L2DMatrix44):

    def __init__(self):
        super().__init__()
        self.screenLeft = None
        self.screenRight = None
        self.screenTop = None
        self.screenBottom = None
        self.maxLeft = None
        self.maxRight = None
        self.maxTop = None
        self.maxBottom = None
        self.max = float('inf')
        self.min = 0

    def getMaxScale(self):
        return self.max

    def getMinScale(self):
        return self.min

    def setMaxScale(self, v):
        self.max = v

    def setMinScale(self, v):
        self.min = v

    def isMaxScale(self):
        return self.getScaleX() == self.max

    def isMinScale(self):
        return self.getScaleX() == self.min

    def adjustTranslate(self, shift_x, shift_y):
        if self.tr[0] * self.maxLeft + (self.tr[12] + shift_x) > self.screenLeft:
            shift_x = self.screenLeft - self.tr[0] * self.maxLeft - self.tr[12]
        if self.tr[0] * self.maxRight + (self.tr[12] + shift_x) < self.screenRight:
            shift_x = self.screenRight - self.tr[0] * self.maxRight - self.tr[12]
        if self.tr[5] * self.maxTop + (self.tr[13] + shift_y) < self.screenTop:
            shift_y = self.screenTop - self.tr[5] * self.maxTop - self.tr[13]
        if self.tr[5] * self.maxBottom + (self.tr[13] + shift_y) > self.screenBottom:
            shift_y = self.screenBottom - self.tr[5] * self.maxBottom - self.tr[13]
        tr1 = [1, 0, 0, 0, 0, 1, 0, 0, 0, 0, 1, 0, shift_x, shift_y, 0, 1]
        L2DMatrix44.mul(tr1, self.tr, self.tr)

    def adjustScale(self, cx, cy, scale):
        target_scale = scale * self.tr[0]
        if target_scale < self.min:
            if self.tr[0] > 0:
                scale = self.min / self.tr[0]
        elif target_scale > self.max:
            if self.tr[0] > 0:
                scale = self.max / self.tr[0]

        tr1 = [1, 0, 0, 0, 0, 1, 0, 0, 0, 0, 1, 0, cx, cy, 0, 1]
        tr2 = [scale, 0, 0, 0, 0, scale, 0, 0, 0, 0, 1, 0, 0, 0, 0, 1]
        tr3 = [1, 0, 0, 0, 0, 1, 0, 0, 0, 0, 1, 0, -cx, -cy, 0, 1]
        L2DMatrix44.mul(tr3, self.tr, self.tr)
        L2DMatrix44.mul(tr2, self.tr, self.tr)
        L2DMatrix44.mul(tr1, self.tr, self.tr)

    def setScreenRect(self, left, right, bottom, top):
        self.screenLeft = left
        self.screenRight = right
        self.screenTop = top
        self.screenBottom = bottom

    def setMaxScreenRect(self, left, right, bottom, top):
        self.maxLeft = left
        self.maxRight = right
        self.maxTop = top
        self.maxBottom = bottom

    def getScreenLeft(self):
        return self.screenLeft

    def getScreenRight(self):
        return self.screenRight

    def getScreenBottom(self):
        return self.screenBottom

    def getScreenTop(self):
        return self.screenTop

    def getMaxLeft(self):
        return self.maxLeft

    def getMaxRight(self):
        return self.maxRight

    def getMaxBottom(self):
        return self.maxBottom

    def getMaxTop(self):
        return self.maxTop
