from .l2d_matrix44 import L2DMatrix44


class L2DModelMatrix(L2DMatrix44):

    def __init__(self, w, h):
        super().__init__()
        self.width = w
        self.height = h
        self.ocx = 0
        self.ocy = 0

    def setPosition(self, x, y):
        self.translate(x, y)

    def setCenterPosition(self, x, y):
        self.ocx = x
        self.ocy = y
        w = self.width * self.getScaleX()
        h = self.height * self.getScaleY()
        self.translate(x - w / 2, y - h / 2)

    def top(self, y):
        self.setY(y)

    def bottom(self, y):
        h = self.height * self.getScaleY()
        self.translateY(y - h)

    def left(self, x):
        self.setX(x)

    def right(self, x):
        w = self.width * self.getScaleX()
        self.translateX(x - w)

    def centerX(self, x):
        w = self.width * self.getScaleX()
        self.translateX(x - w / 2)

    def centerY(self, y):
        h = self.height * self.getScaleY()
        self.translateY(y - h / 2)

    def setX(self, x):
        self.translateX(x)

    def setY(self, y):
        self.translateY(y)

    def setHeight(self, h):
        scale_x = h / self.height
        scale_y = -scale_x
        self.scale(scale_x, scale_y)

    def setWidth(self, w):
        scale_x = w / self.width
        scale_y = -scale_x
        self.scale(scale_x, scale_y)
