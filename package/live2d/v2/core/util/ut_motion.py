import math


class UtMotion:
    @staticmethod
    def getEasingSine(value: float):
        if value < 0:
            return 0

        if value > 1:
            return 1

        return 0.5 - 0.5 * math.cos(value * math.pi)
