# 参数计算

from facial_capture.math_utils import euclideanDistance
import numpy as np


def calculate_eye_openness(landmarks, eye_points):
    """
    计算眼部开合
    :param landmarks: 所有面部特征点
    :param eye_points: 特征点索引
    :return: 眼部纵横比
    """
    # [(x1, y1), ..., (x6, y6)]
    points = [(landmarks[i].x, landmarks[i].y) for i in eye_points]
    eye_open_ratio = ((euclideanDistance(points[1], points[5]) +
                       euclideanDistance(points[2], points[4])) /
                      2 /
                      euclideanDistance(points[0], points[3]))
    return eye_open_ratio


def calculate_mouth_openness(landmarks, lip_points):
    """
    计算嘴部开合，同眼睛算法
    """
    return calculate_eye_openness(landmarks, lip_points)


def calculate_head_pose(landmarks):
    """
    计算面部角度
    :param landmarks:
    :return: 角度
    """
    # 左右眼中心
    left_eye_center = ((landmarks[33].x + landmarks[133].x) / 2, (landmarks[33].y + landmarks[133].y) / 2)
    right_eye_center = ((landmarks[362].x + landmarks[263].x) / 2, (landmarks[362].y + landmarks[263].y) / 2)

    # 计算Roll角（左右倾斜）
    delta_y = right_eye_center[1] - left_eye_center[1]
    delta_x = right_eye_center[0] - left_eye_center[0]
    roll_angle = np.degrees(np.arctan(delta_y / delta_x))

    # 计算Yaw角（左右旋转）
    # https://github.com/adrianiainlam/facial-landmarks-for-cubism/blob/master/src/faceXAngle.png
    nose_x = landmarks[1].x
    face_left_x = landmarks[454].x
    face_right_x = landmarks[234].x
    perpLeft = abs(nose_x - face_left_x)
    perpRight = abs(face_right_x - nose_x)
    yaw_angle = np.degrees(np.arcsin((perpRight - perpLeft) / (perpRight + perpLeft)))

    # 计算Pitch角（上下旋转）
    face_center_top = landmarks[10]
    face_center_bottom = landmarks[152]
    delta_y = face_center_top.y - face_center_bottom.y
    delta_z = face_center_bottom.z - face_center_top.z
    pitch_angle = np.degrees(np.arctan(delta_z / delta_y))

    return roll_angle, yaw_angle, pitch_angle
