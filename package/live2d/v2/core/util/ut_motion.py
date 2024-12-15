import math


class UtMotion:
    @staticmethod
    def r2_(aH):
        if aH < 0:
            return 0

        if aH > 1:
            return 1

        return 0.5 - 0.5 * math.cos(aH * math.pi)
