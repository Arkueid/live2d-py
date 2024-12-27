# 人脸动捕任务
import os.path
import time

import cv2
import mediapipe as mp

from facial_params import Params
from mediapipe_capture.calculation import calculate_head_pose, calculate_mouth_openness, calculate_eye_openness, \
    calculate_body_angle_x
from mediapipe_capture.capture_config import *
from mediapipe_capture.filters import initialize_kalman_filter, apply_kalman_filter
from mediapipe_capture.math_utils import clipValue, linearScale01, linearScale_11


current_dir = os.path.split(__file__)[0]
test_video = os.path.join(current_dir, "../test.mp4")


def mediapipe_capture_task(params: Params):
    # 初始化 Mediapipe Face Mesh 模块
    mp_face_mesh = mp.solutions.face_mesh
    face_mesh = mp_face_mesh.FaceMesh(static_image_mode=False,
                                      max_num_faces=1,
                                      min_detection_confidence=0.5,
                                      min_tracking_confidence=0.8)

    mp_pose = mp.solutions.pose
    pose = mp_pose.Pose()

    # 摄像头
    cap = cv2.VideoCapture(test_video)

    pTime = time.time()

    lkfs = [initialize_kalman_filter() for i in range(6)]
    rkfs = [initialize_kalman_filter() for i in range(6)]

    while True:
        ret, frame = cap.read()
        if not ret:
            break

        # 镜面效果
        # frame = cv2.flip(frame, 1)
        rgb_frame = cv2.cvtColor(frame, cv2.COLOR_BGR2RGB)
        frame = cv2.GaussianBlur(frame, (3, 3), 1)

        results = face_mesh.process(rgb_frame)

        results2 = pose.process(frame)
        # 绘制特征点
        if results.multi_face_landmarks:
            landmarks = results.multi_face_landmarks[0].landmark
            frame_height, frame_width, _ = frame.shape

            # left eye openness ratio
            leftEyePoints = [(landmarks[i].x, landmarks[i].y) for i in LEFT_EYE]
            newPoints = []
            for i in range(6):
                x = apply_kalman_filter(lkfs[i], leftEyePoints[i])
                newPoints.append(x)
            leftEyePoints = newPoints

            lEyeOpenRatio = calculate_eye_openness(leftEyePoints)
            # right eye openness ratio
            rightEyePoints = [(landmarks[i].x, landmarks[i].y) for i in RIGHT_EYE]
            newPoints = []
            for i in range(6):
                x = apply_kalman_filter(rkfs[i], rightEyePoints[i])
                newPoints.append(x)
            rightEyePoints = newPoints
            rEyeOpenRatio = calculate_eye_openness(rightEyePoints)
            # mouth openness ration
            mouthPoints = [(landmarks[i].x, landmarks[i].y) for i in LIP]
            mouthOpenRatio = calculate_mouth_openness(mouthPoints)

            # 面部角度
            headPoints = [(landmarks[i].x, landmarks[i].y, landmarks[i].z) for i in HEAD]
            roll_angle, yaw_angle, pitch_angle = calculate_head_pose(headPoints)

            if params:
                # 直接使用计算结果会存在抖动现象，需要后续处理
                params.EyeLOpen = round(linearScale01(lEyeOpenRatio, EYE_OPENNESS_MIN, EYE_OPENNESS_MAX), 1)
                params.EyeROpen = round(linearScale01(rEyeOpenRatio, EYE_OPENNESS_MIN, EYE_OPENNESS_MAX), 1)
                params.MouthOpenY = round(linearScale01(mouthOpenRatio, MOUTH_OPENNESS_MIN, MOUTH_OPENNESS_MAX), 1)
                params.MouthForm = linearScale_11(mouthOpenRatio, 0.2, 1.0)
                params.AngleX = clipValue(yaw_angle, -30, 30)
                params.AngleY = clipValue(pitch_angle, -30, 30)
                params.AngleZ = clipValue(roll_angle, -30, 30)

            # 绘制特征点
            # 眼睛特征点
            for point in leftEyePoints + rightEyePoints + mouthPoints + headPoints:
                cv2.circle(frame, (int(point[0] * frame_width), int(point[1] * frame_height)), 2, (0, 255, 0), -1)

        if results2.pose_landmarks:
            landmarks = results2.pose_landmarks.landmark

            # 获取躯干的关键点
            left_shoulder = landmarks[11]
            right_shoulder = landmarks[12]

            if results.multi_face_landmarks:
                nose_x = landmarks[1].x
                bodyAngleX = calculate_body_angle_x(nose_x, left_shoulder, right_shoulder)
                if params:
                    params.BodyAngleX = clipValue(bodyAngleX, -30, 30)

            # 在图像上绘制躯干关键点
            image_height, image_width, _ = frame.shape
            cv2.circle(frame, (int(left_shoulder.x * image_width), int(left_shoulder.y * image_height)), 5, (0, 255, 0),
                       -1)
            cv2.circle(frame, (int(right_shoulder.x * image_width), int(right_shoulder.y * image_height)), 5,
                       (0, 255, 0), -1)

            # 绘制躯干线
            cv2.line(frame,
                     (int(left_shoulder.x * image_width), int(left_shoulder.y * image_height)),
                     (int(right_shoulder.x * image_width), int(right_shoulder.y * image_height)),
                     (0, 255, 0), 2)

        cTime = time.time()
        fps = 1 / (cTime - pTime)
        pTime = cTime

        cv2.putText(frame, f"{int(fps)}", (20, 70), cv2.FONT_HERSHEY_PLAIN, 3, (0, 255, 0), 3)

        cv2.imshow("Face Capture", frame)

        if cv2.waitKey(1) & 0xFF == ord('q'):
            break

    cap.release()
    cv2.destroyAllWindows()


if __name__ == '__main__':
    import threading as td

    t = td.Thread(None, mediapipe_capture_task, "Capture Task", (None,))
    t.start()

    t.join()
