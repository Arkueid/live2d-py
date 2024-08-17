# 人脸动捕任务

import cv2 as cv
import mediapipe as mp
from facial_capture.math_utils import linearScale
from facial_capture.listeners import OnCapturedListener
from facial_capture.facial_params import FacialParams
from facial_capture.capture_config import *
from facial_capture.calculation import calculate_eye_open_ratio


def facial_capture_task(listener: OnCapturedListener):
    # 初始化 Mediapipe Face Mesh 模块
    mp_face_mesh = mp.solutions.face_mesh
    face_mesh = mp_face_mesh.FaceMesh(static_image_mode=False, max_num_faces=1, min_detection_confidence=0.5)

    # 摄像头
    cap = cv.VideoCapture(0)
    # 初始化面部参数
    params = FacialParams()

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
            face_landmarks = results.multi_face_landmarks[0]
            frame_height, frame_width, _ = frame.shape

            # left openness ratio
            lor = calculate_eye_open_ratio(face_landmarks.landmark, LEFT_EYE)
            # right openness ration
            ror = calculate_eye_open_ratio(face_landmarks.landmark, RIGHT_EYE)

            # 直接使用计算结果会存在抖动现象，需要后续处理
            params.paramEyeLOpen = round(linearScale(lor, EYE_OPENNESS_MIN, EYE_OPENNESS_MAX), 1)
            params.paramEyeROpen = round(linearScale(ror, EYE_OPENNESS_MIN, EYE_OPENNESS_MAX), 1)
            # 回调
            listener.onCaptured(params)

            # 绘制特征点
            # 眼睛特征点
            for p in LEFT_EYE + RIGHT_EYE:
                point = face_landmarks.landmark[p]
                cv.circle(frame, (int(point.x * frame_width), int(point.y * frame_height)), 2, (0, 255, 0), -1)

        cv.imshow("Mediapipe Face Mesh", frame)

        if cv.waitKey(1) & 0xFF == ord('q'):
            break

    cap.release()
    cv.destroyAllWindows()


if __name__ == '__main__':
    class MyListener(OnCapturedListener):

        def onCaptured(self, params: FacialParams):
            # 测试打印
            print(params.paramEyeLOpen, params.paramEyeROpen)


    import threading as td

    t = td.Thread(None, facial_capture_task, "Capture Task", (MyListener(),))
    t.start()

    t.join()
