import socket
import struct
from typing import Optional

from facial_params import Params
from open_see_face import config
from open_see_face.calculation import calc_face_x_angle, calc_mouth_form, calc_face_y_angle, calc_eye_openness, \
    calc_mouth_openness, calc_face_z_angle
from open_see_face.math_utils import avg

__faceXAngle = []
__faceYAngle = []
__faceZAngle = []
__mouthOpenness = []
__mouthForm = []
__leftEyeOpenness = []
__rightEyeOpenness = []


def set_params(params: Params):
    params.AngleX = avg(__faceXAngle)
    params.AngleY = avg(__faceYAngle) + config.faceYAngleCorrection
    params.AngleZ = avg(__faceZAngle)
    params.MouthOpenY = avg(__mouthOpenness)
    params.MouthForm = avg(__mouthForm)

    left_eye = avg(__leftEyeOpenness, 1)
    right_eye = avg(__rightEyeOpenness, 1)
    sync = config.winkEnable
    if config.winkEnable:
        if right_eye < 0.1 and left_eye > 0.2:
            left_eye = 1
            right_eye = 0
        elif left_eye < 0.1 and right_eye > 0.2:
            left_eye = 0
            right_eye = 1
        else:
            sync = True

    if sync:
        both_eyes = (left_eye + right_eye) / 2
        left_eye = both_eyes
        right_eye = both_eyes

    params.EyeLOpen = left_eye
    params.EyeROpen = right_eye

    if (left_eye <= config.eyeSmileEyeOpenThreshold and
            right_eye <= config.eyeSmileEyeOpenThreshold and
            params.MouthForm > config.eyeSmileMouthFormThreshold and
            params.MouthOpenY > config.eyeSmileMouthOpenThreshold):
        params.leftEyeSmile = 1
        params.rightEyeSmile = 1
    else:
        params.leftEyeSmile = 0
        params.rightEyeSmile = 0


def append_and_clip(ls: list[float], value: float, max_size: int):
    ls.append(value)
    if len(ls) > max_size:
        ls.remove(ls[0])


def open_see_face_task(params: Optional[Params]):
    sock = socket.socket(socket.AF_INET, socket.SOCK_DGRAM)
    # sock.bind(("127.0.0.1", 11451))
    sock.bind((config.osfIpAddress, config.osfPort))

    packet_size = 8 + 4 + 2 * 4 + 2 * 4 + 1 + 4 + 3 * 4 + 3 * 4 + 4 * 4 + 4 * 68 + 4 * 2 * 68 + 4 * 3 * 70 + 4 * 14
    landmarks_offset = 8 + 4 + 2 * 4 + 2 * 4 + 1 + 4 + 3 * 4 + 3 * 4 + 4 * 4 + 4 * 68

    n_points = 68
    while True:
        data = sock.recvfrom(packet_size)[0]
        face_id = struct.unpack("<i", data[8:8 + 4])[0]
        if face_id != 0:
            continue

        landmarks = struct.unpack(f"<{n_points * 2}f", data[landmarks_offset:landmarks_offset + n_points * 2 * 4])

        landmarks = [(landmarks[i], landmarks[i+1]) for i in range(0, len(landmarks), 2)]
        face_x_rot = calc_face_x_angle(landmarks)
        append_and_clip(__faceXAngle, face_x_rot, config.faceXAngleNumTaps)

        mouthForm = calc_mouth_form(landmarks)
        append_and_clip(__mouthForm, mouthForm, config.mouthFormNumTaps)

        faceYRot = calc_face_y_angle(face_x_rot, mouthForm, landmarks)
        append_and_clip(__faceYAngle, faceYRot, config.faceYAngleNumTaps)

        faceZRot = calc_face_z_angle(landmarks)
        append_and_clip(__faceZAngle, faceZRot, config.faceZAngleNumTaps)

        mouthOpen = calc_mouth_openness(mouthForm, landmarks)
        append_and_clip(__mouthOpenness, mouthOpen, config.mouthOpenNumTaps)

        eyeLeftOpen = calc_eye_openness(True, faceYRot, landmarks)
        append_and_clip(__leftEyeOpenness, eyeLeftOpen, config.leftEyeOpenNumTaps)
        eyeRightOpen = calc_eye_openness(False, faceYRot, landmarks)
        append_and_clip(__rightEyeOpenness, eyeRightOpen, config.rightEyeOpenNumTaps)

        if params:
            set_params(params)


if __name__ == '__main__':
    params = Params()
    open_see_face_task(params)
