class ClipRectF:

    def __init__(self):
        self.x = None
        self.y = None
        self.width = None
        self.height = None

    def getRight(self):
        return self.x + self.width

    def getBottom(self):
        return self.y + self.height

    def expand(self, aH, aI):
        self.x -= aH
        self.y -= aI
        self.width += aH * 2
        self.height += aI * 2

    def setRect(self, other):
        self.x = other.x
        self.y = other.y
        self.width = other.width
        self.height = other.height
