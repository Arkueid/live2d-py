add_library(${V2_TARGET} STATIC
    Core/Id.cpp
    Core/BinaryReader.cpp
    Core/ObjectFactory.cpp
    Core/Parameter.cpp
    Core/Part.cpp
    Core/PivotManager.cpp
    Core/Moc.cpp
    Core/Deformer.cpp
    Core/AffineEnt.cpp
    Core/RotationDeformer.cpp
    Core/WarpDeformer.cpp
    Core/Drawable.cpp
    Core/UtInterpolate.cpp
    Framework/TargetPoint.cpp
    Framework/EyeBlink.cpp
    Framework/Pose.cpp
    Framework/Physics.cpp
    Framework/PhysicsHair.cpp
    Framework/ExpressionMotion.cpp
    Framework/MotionManager.cpp
    Framework/BaseModel.cpp
    Framework/Motion.cpp
    Model.cpp
)

if(${GR_BACKEND} MATCHES "OpenGL" OR ${GR_BACKEND} MATCHES "Dual")
    target_sources(${V2_TARGET} PRIVATE
        Renderer/GLRenderer.cpp
        Renderer/ClippingManagerOpenGL.cpp
    )
endif()
if(${GR_BACKEND} MATCHES "Vulkan" OR ${GR_BACKEND} MATCHES "Dual")
    target_sources(${V2_TARGET} PRIVATE
        Renderer/VkRenderer.cpp
        Renderer/ClippingManagerVulkan.cpp
    )
endif()


