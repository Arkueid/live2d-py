import cv2
import numpy as np


def initialize_kalman_filter():
    kf = cv2.KalmanFilter(4, 2)

    # 状态转移矩阵 (F)
    kf.transitionMatrix = np.array([[1, 0, 1, 0],
                                    [0, 1, 0, 1],
                                    [0, 0, 1, 0],
                                    [0, 0, 0, 1]], np.float32)

    # 测量矩阵 (H)
    kf.measurementMatrix = np.array([[1, 0, 0, 0],
                                     [0, 1, 0, 0]], np.float32)

    # 过程噪声协方差 (Q)
    kf.processNoiseCov = np.array([[1, 0, 0, 0],
                                   [0, 1, 0, 0],
                                   [0, 0, 1, 0],
                                   [0, 0, 0, 1]], np.float32) * 1e-4

    # 测量噪声协方差 (R)
    kf.measurementNoiseCov = np.array([[1, 0],
                                       [0, 1]], np.float32) * 1e-2

    # 估计误差协方差矩阵 (P)
    kf.errorCovPost = np.eye(4, dtype=np.float32)

    return kf


# 使用卡尔曼滤波器进行平滑
def apply_kalman_filter(kf, point):
    # 更新测量
    measurement = np.array([[np.float32(point[0])],
                            [np.float32(point[1])]])

    # 预测
    kf.predict()

    # 更新卡尔曼滤波器并得到新的估计状态
    estimated = kf.correct(measurement)

    # 返回估计后的x和y位置
    return estimated[0][0], estimated[1][0]
