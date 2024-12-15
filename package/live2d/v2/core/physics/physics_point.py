class PhysicsPoint:

    def __init__(self):
        self.mass = 1
        self.x = 0
        self.y = 0
        self.vx = 0
        self.vy = 0
        self.ax = 0
        self.ay = 0
        self.fx = 0
        self.fy = 0
        self.lastX = 0
        self.lastY = 0
        self.lastVX = 0
        self.lastVY = 0

    def setupLast(self):
        self.lastX = self.x
        self.lastY = self.y
        self.lastVX = self.vx
        self.lastVY = self.vy
