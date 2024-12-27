# 参数计算

from mediapipe_capture.math_utils import euclideanDistance
import numpy as np


def calculate_eye_openness(points):
    """
    计算眼部开合
    :param points: 所有面部特征点
    :return: 眼部纵横比
    """
    # [(x1, y1), ..., (x6, y6)]
    eye_open_ratio = ((euclideanDistance(points[1], points[5]) +
                       euclideanDistance(points[2], points[4])) /
                      2 /
                      euclideanDistance(points[0], points[3]))
    return eye_open_ratio


def calculate_mouth_openness(points):
    """
    计算嘴部开合，同眼睛算法
    """
    return calculate_eye_openness(points)


def calculate_head_pose(points):
    """
    计算面部角度
    :param points:
    :return: 角度
    """
    # 左右眼中心
    left_eye_center = ((points[0][0] + points[1][0]) / 2, (points[0][1] + points[1][1]) / 2)
    right_eye_center = ((points[2][0] + points[3][0]) / 2, (points[2][1] + points[3][1]) / 2)

    # 计算Roll角（左右倾斜）
    delta_y = right_eye_center[1] - left_eye_center[1]
    delta_x = right_eye_center[0] - left_eye_center[0]
    roll_angle = np.degrees(np.arctan(delta_y / delta_x))

    # 计算Yaw角（左右旋转）
    # https://github.com/adrianiainlam/facial-landmarks-for-cubism/blob/master/src/faceXAngle.png
    nose_x = points[4][0]
    face_left_x = points[5][0]
    face_right_x = points[6][0]
    perpLeft = abs(nose_x - face_left_x)
    perpRight = abs(face_right_x - nose_x)
    yaw_angle = np.degrees(np.arcsin((perpRight - perpLeft) / (perpRight + perpLeft)))

    # 计算Pitch角（上下旋转）
    face_center_top = points[7]
    face_center_bottom = points[8]
    delta_y = face_center_top[1] - face_center_bottom[1]
    delta_z = face_center_bottom[2] - face_center_top[2]
    pitch_angle = np.degrees(np.arctan(delta_z / delta_y))

    return roll_angle, yaw_angle, pitch_angle


def calculate_body_angle_x(body_center_x, left_shoulder, right_shoulder):
    perpLeft = abs(left_shoulder.x - body_center_x)
    perpRight = abs(right_shoulder.x - body_center_x)
    return np.degrees(np.arcsin((perpRight - perpLeft) / (perpRight + perpLeft)))
