class L2DMatrix44:

    def __init__(self):
        self.tr = [0] * 16
        self.identity()

    def identity(self):
        for i in range(16):
            self.tr[i] = 1 if ((i % 5) == 0) else 0

    def getArray(self):
        return self.tr

    def getCopyMatrix(self):
        return self.tr.copy()

    def setMatrix(self, tr):
        if self.tr is None or self.tr.length != self.tr.length:
            return
        for i in range(16):
            self.tr[i] = tr[i]

    def getScaleX(self):
        return self.tr[0]

    def getScaleY(self):
        return self.tr[5]

    def transformX(self, src):
        return self.tr[0] * src + self.tr[12]

    def transformY(self, src):
        return self.tr[5] * src + self.tr[13]

    def invertTransformX(self, src):
        return (src - self.tr[12]) / self.tr[0]

    def invertTransformY(self, src):
        return (src - self.tr[13]) / self.tr[5]

    def multTranslate(self, shiftX, shiftY):
        tr1 = [1, 0, 0, 0,
               0, 1, 0, 0,
               0, 0, 1, 0,
               shiftX, shiftY, 0, 1]
        L2DMatrix44.mul(tr1, self.tr, self.tr)

    def translate(self, x, y):
        self.tr[12] = x
        self.tr[13] = y

    def translateX(self, x):
        self.tr[12] = x

    def translateY(self, y):
        self.tr[13] = y

    def multScale(self, scaleX, scaleY):
        tr1 = [scaleX, 0, 0, 0, 0, scaleY, 0, 0, 0, 0, 1, 0, 0, 0, 0, 1]
        L2DMatrix44.mul(tr1, self.tr, self.tr)

    def scale(self, scaleX, scaleY):
        self.tr[0] = scaleX
        self.tr[5] = scaleY

    @staticmethod
    def mul(a, b, dst):
        c = [0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0]
        n = 4
        for i in range(n):
            for j in range(n):
                for k in range(n):
                    c[i + j * 4] += a[i + k * 4] * b[k + j * 4]

        for i in range(16):
            dst[i] = c[i]
