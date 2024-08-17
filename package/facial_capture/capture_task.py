# 人脸动捕任务

import cv2 as cv
import mediapipe as mp
from facial_capture.math_utils import *
from facial_capture.facial_params import FacialParams
from facial_capture.capture_config import *
from facial_capture.calculation import *


def facial_capture_task(params: FacialParams):
    # 初始化 Mediapipe Face Mesh 模块
    mp_face_mesh = mp.solutions.face_mesh
    face_mesh = mp_face_mesh.FaceMesh(static_image_mode=False, max_num_faces=1, min_detection_confidence=0.5)

    # 摄像头
    cap = cv.VideoCapture(0)

    while True:
        ret, frame = cap.read()
        if not ret:
            break

        # 镜面效果
        frame = cv.flip(frame, 1)
        rgb_frame = cv.cvtColor(frame, cv.COLOR_BGR2RGB)

        results = face_mesh.process(rgb_frame)
        # 绘制特征点
        if results.multi_face_landmarks:
            landmarks = results.multi_face_landmarks[0].landmark
            frame_height, frame_width, _ = frame.shape

            # left eye openness ratio
            lor = calculate_eye_openness(landmarks, LEFT_EYE)
            # right eye openness ratio
            ror = calculate_eye_openness(landmarks, RIGHT_EYE)
            # mouth openness ration
            mor = calculate_mouth_openness(landmarks, LIP)

            # 面部角度
            roll_angle, yaw_angle, pitch_angle = calculate_head_pose(landmarks)

            # 直接使用计算结果会存在抖动现象，需要后续处理
            params.EyeLOpen = round(linearScalePercent(lor, EYE_OPENNESS_MIN, EYE_OPENNESS_MAX), 1)
            params.EyeROpen = round(linearScalePercent(ror, EYE_OPENNESS_MIN, EYE_OPENNESS_MAX), 1)
            params.MouthOpenY = round(linearScalePercent(mor, MOUTH_OPENNESS_MIN, MOUTH_OPENNESS_MAX), 1)
            params.AngleX = clipValue(yaw_angle, -30, 30)
            params.AngleY = clipValue(pitch_angle, -30, 30)
            params.AngleZ = clipValue(roll_angle, -30, 30)

            # 绘制特征点
            # 眼睛特征点
            for p in LEFT_EYE + RIGHT_EYE + LIP:
                point = landmarks[p]
                cv.circle(frame, (int(point.x * frame_width), int(point.y * frame_height)), 2, (0, 255, 0), -1)

        cv.imshow("Mediapipe Face Mesh", frame)

        if cv.waitKey(1) & 0xFF == ord('q'):
            break

    cap.release()
    cv.destroyAllWindows()


if __name__ == '__main__':
    import threading as td

    t = td.Thread(None, facial_capture_task, "Capture Task", (FacialParams(),))
    t.start()

    t.join()
