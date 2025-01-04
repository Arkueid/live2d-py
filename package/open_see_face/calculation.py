import math
from typing import Sequence

from open_see_face import config
from open_see_face.math_utils import centroid, dis, solve_cosine_rule_angle, linear_scale01


def calc_face_z_angle(landmarks: Sequence[Sequence[float]]):
    eye_right = centroid(*landmarks[36:41 + 1])
    eye_left = centroid(*landmarks[42:48])

    nose_left = landmarks[35]
    nose_right = landmarks[31]

    eye_y_diff = eye_right[1] - eye_left[1]
    eye_x_diff = eye_right[0] - eye_right[0]

    angle1 = math.atan(eye_y_diff / (eye_x_diff + 0.0001))

    nose_y_diff = nose_right[1] - nose_left[1]
    nose_x_diff = nose_right[0] - nose_left[0]

    angle2 = math.atan(nose_y_diff / nose_x_diff)

    return math.degrees((angle1 + angle2) / 2)


def calc_face_y_angle(face_x_angle: float, mouth_form: float, landmarks: Sequence[Sequence[float]]):
    c = dis(landmarks[31], landmarks[35])
    a = dis(landmarks[30], landmarks[31])
    b = dis(landmarks[30], landmarks[35])

    angle = solve_cosine_rule_angle(c, a, b)

    corr_angle = angle * (1 + (abs(face_x_angle) / 30 * config.faceYAngleCorrection))

    corr_angle *= (1 - mouth_form * config.faceYAngleSmileCorrection)

    if corr_angle >= config.faceYAngleZeroValue:
        return -30 * linear_scale01(corr_angle, config.faceYAngleZeroValue,
                                    config.faceYAngleDownThreshold,
                                    False, False)

    else:
        return -30 * (1 - linear_scale01(corr_angle,
                                         config.faceYAngleDownThreshold,
                                         config.faceYAngleZeroValue,
                                         False, False))


def calc_face_x_angle(landmarks: Sequence[Sequence[float]]):
    y0 = centroid(landmarks[27], landmarks[28], landmarks[29],
                  landmarks[30])
    y1 = centroid(landmarks[48], landmarks[49], landmarks[50],
                  landmarks[51], landmarks[52])

    left = centroid(landmarks[14], landmarks[15], landmarks[16])
    right = centroid(landmarks[0], landmarks[1], landmarks[2])

    opp = dis(right, y0)
    adj1 = dis(y0, y1)
    adj2 = dis(y1, right)
    angle = solve_cosine_rule_angle(opp, adj1, adj2)
    perp_right = adj2 * math.sin(angle)

    opp = dis(left, y0)
    adj2 = dis(y1, left)
    angle = solve_cosine_rule_angle(opp, adj1, adj2)
    perp_left = adj2 * math.sin(angle)

    theta = math.asin((perp_right - perp_left) / (perp_right + perp_left))

    theta = math.degrees(theta)
    if theta < -30:
        theta = -30
    if theta > 30:
        theta = 30
    return theta


def calc_mouth_openness(mouth_form: float, landmarks: Sequence[Sequence[float]]):
    height_left = dis(landmarks[61], landmarks[63])
    height_middle = dis(landmarks[60], landmarks[64])
    height_right = dis(landmarks[59], landmarks[65])

    avg_height = (height_left + height_middle + height_right) / 3
    width = dis(landmarks[58], landmarks[62])

    normalized = avg_height / width

    scaled = linear_scale01(normalized,
                            config.mouthClosedThreshold,
                            config.mouthOpenThreshold,
                            True, False)

    scaled *= (1 + config.mouthOpenLaughCorrection * mouth_form)

    return scaled


def calc_mouth_form(landmarks: Sequence[Sequence[float]]):
    eye1 = centroid(landmarks[36], landmarks[37], landmarks[38],
                    landmarks[39], landmarks[40], landmarks[41])
    eye2 = centroid(landmarks[42], landmarks[43], landmarks[44],
                    landmarks[45], landmarks[46], landmarks[47])
    dist_eyes = dis(eye1, eye2)
    dist_mouth = dis(landmarks[58], landmarks[62])

    form = linear_scale01(dist_mouth / dist_eyes,
                          config.mouthNormalThreshold,
                          config.mouthSmileThreshold)

    return form


def calc_eye_openness(is_left: bool, face_y_angle: float, landmarks: Sequence[Sequence[float]]):
    if is_left:
        eye_aspect_ratio = calc_eye_aspect_ratio(landmarks[42], landmarks[43], landmarks[44],
                                              landmarks[45], landmarks[46], landmarks[47])
    else:
        eye_aspect_ratio = calc_eye_aspect_ratio(landmarks[36], landmarks[37], landmarks[38],
                                              landmarks[39], landmarks[40], landmarks[41])
    corr_eye_asp_rat = eye_aspect_ratio / math.cos(math.radians(face_y_angle))

    return linear_scale01(corr_eye_asp_rat, config.eyeClosedThreshold, config.eyeOpenThreshold)


def calc_eye_aspect_ratio(p1: Sequence[float], p2: Sequence[float], p3: Sequence[float],
                          p4: Sequence[float], p5: Sequence[float], p6: Sequence[float]):
    eyeWidth = dis(p1, p4)
    eyeHeight1 = dis(p2, p6)
    eyeHeight2 = dis(p3, p5)

    return (eyeHeight1 + eyeHeight2) / (2 * eyeWidth)
