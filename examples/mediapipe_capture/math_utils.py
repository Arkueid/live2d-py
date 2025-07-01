import math


def clipValue(value, min_value, max_value):
    if value > max_value:
        return max_value
    elif value < min_value:
        return min_value
    return value


def linearScale01(value, min_value, max_value):
    """
    将值映射到 0.0 ~ 1.0 之间
    :param value: 当前值
    :param min_value: 最小值
    :param max_value: 最大值
    :return: 映射结果
    """
    if value < min_value:
        value = min_value
    if value > max_value:
        value = max_value
    return (value - min_value) / (max_value - min_value)


def linearScale_11(value, min_value, max_value):
    """
    -1 ~ 1
    :param value:
    :param min_value:
    :param max_value:
    :return:
    """
    middle = (max_value + min_value) / 2
    if value < min_value:
        value = min_value
    if value > max_value:
        value = max_value
    return (value - middle) / (max_value - middle)


def euclideanDistance(p1, p2):
    """
    欧几里得距离
    :param p1: (x, y)
    :param p2: (x, y)
    :return: 两点间距离，float
    """
    return math.sqrt((p1[0] - p2[0]) ** 2 + (p1[1] - p2[1]) ** 2)
