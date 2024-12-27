# 动捕设置

# 眼部控制相关参数
# 开合阈值，因人脸、算法而异
EYE_OPEN_THRESHOLD = 0.25
# 最大值
EYE_OPENNESS_MAX = 0.38
# 最小值
EYE_OPENNESS_MIN = EYE_OPEN_THRESHOLD
# 计算眼部开合所用到的眼睛特征点索引 [p1 p2 p3 p4 p5 p6]
LEFT_EYE = [362, 385, 387, 263, 373, 380]
RIGHT_EYE = [33, 160, 158, 133, 153, 144]

# 嘴
LIP = [78, 82, 312, 308, 317, 87]
MOUTH_OPEN_THRESHOLD = 0.1
MOUTH_OPENNESS_MAX = 0.5
MOUTH_OPENNESS_MIN = MOUTH_OPEN_THRESHOLD

# 头部姿势 right, left, nose, upper_lip_center, lower_lip_center
HEAD = [33, 133, 362, 263, 1, 454, 234, 10, 152]

# 眼球
LEFT_EYE_BALL = [469, 470, 471, 472]
RIGHT_EYE_BALL = [474, 475, 476, 477]
