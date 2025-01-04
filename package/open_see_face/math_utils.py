# reference: https://github.com/adrianiainlam/facial-landmarks-for-cubism/
import math
from typing import List


def centroid(*points) -> List[float]:
    size = len(points)
    if size == 0:
        return [0, 0]

    x = sum([i[0] for i in points]) / size
    y = sum([i[1] for i in points]) / size

    return [x, y]


def dis(p1, p2) -> float:
    dx = p1[0] - p2[0]
    dy = p1[1] - p2[1]

    return math.hypot(dx, dy)


def linear_scale01(value: float, min_value: float, max_value: float, clip_min=True,
                   clip_max=True) -> float:
    if value < min_value and clip_min:
        return 0.0

    if value > max_value and clip_max:
        return 1.0

    return (value - min_value) / (max_value - min_value)


def solve_cosine_rule_angle(opp: float, adj1: float, adj2: float):
    return math.acos((opp ** 2 - adj1 ** 2 - adj2 ** 2) / (-2 * adj1 * adj2))


def avg(nums, default=0):
    size = len(nums)
    if size == 0: return default
    return sum(nums) / size
