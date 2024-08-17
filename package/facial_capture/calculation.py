# 参数计算

from facial_capture.math_utils import euclideanDistance


def calculate_eye_open_ratio(landmark, eye_points):
    """
    计算眼部开合
    :param landmark: 所有面部特征点
    :param eye_points: 特征点索引
    :return: 眼部纵横比
    """
    # [(x1, y1), ..., (x6, y6)]
    points = [(landmark[i].x, landmark[i].y) for i in eye_points]
    eye_open_ratio = ((euclideanDistance(points[1], points[5]) +
                       euclideanDistance(points[2], points[4])) /
                      2 /
                      euclideanDistance(points[0], points[3]))
    return eye_open_ratio
