add_library(Framework STATIC
    IPlatformManager.h
    L2DBaseModel.cpp
    L2DBaseModel.h
    L2DExpressionMotion.cpp
    L2DExpressionMotion.h
    L2DEyeBlink.cpp
    L2DEyeBlink.h
    L2DMatrix44.cpp
    L2DMatrix44.h
    L2DModelMatrix.cpp
    L2DModelMatrix.h
    L2DMotionManager.cpp
    L2DMotionManager.h
    L2DPhysics.cpp
    L2DPhysics.h
    L2DPose.cpp
    L2DPose.h
    L2DStandardID.h
    L2DTargetPoint.cpp
    L2DTargetPoint.h
    L2DTextureDesc.cpp
    L2DTextureDesc.h
    L2DViewMatrix.cpp
    L2DViewMatrix.h
    Live2DFramework.cpp
    Live2DFramework.h
)

target_include_directories(Framework PUBLIC ${LIVE2D_INCLUDE} .)