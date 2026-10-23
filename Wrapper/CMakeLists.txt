# ---- v3 Wrapper ----
set(WRAPPER Live2DWrapper)

add_library(${WRAPPER} SHARED
    Init.cpp
    PyModel.cpp
)

target_compile_definitions(${WRAPPER} PRIVATE MODULE_LOG_TAG="pyi")

target_link_libraries(${WRAPPER} PRIVATE Live2D::Live2D Python3::SABIModule)

set_wrapper_output(${WRAPPER} "_live2d")

# 运行时 shader 目录：所有后端/模块的 shader 都汇总到 package/live2d/FrameworkShaders，
# 文件名互不冲突（V3: VertShaderSrc*/FragShaderSrc*；V2: V2*.spv / V2*GL.*）
set(WRAPPER_SHADER_OUT ${CMAKE_SOURCE_DIR}/package/live2d/FrameworkShaders)
set(WRAPPER_SHADER_DIRS)

if (${GR_BACKEND} MATCHES "OpenGL" OR ${GR_BACKEND} MATCHES "Dual")
    # V3 GL 着色器源文件 + V2 GL 着色器源文件
    list(APPEND WRAPPER_SHADER_DIRS
        $<TARGET_PROPERTY:Framework,LIVE2D_GL_SHADER_DIR>
        $<TARGET_PROPERTY:V2,LIVE2D_GL_SHADER_DIR>)
endif()

if (${GR_BACKEND} MATCHES "Vulkan" OR ${GR_BACKEND} MATCHES "Dual")
    # V3 Vulkan .spv + V2 Vulkan .spv（编译产物）
    list(APPEND WRAPPER_SHADER_DIRS
        $<TARGET_PROPERTY:Framework,LIVE2D_VK_SHADER_DIR>
        $<TARGET_PROPERTY:V2Shaders,LIVE2D_VK_SHADER_DIR>)
endif()

add_custom_command(
    TARGET ${WRAPPER} POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E make_directory ${CMAKE_SOURCE_DIR}/package/live2d/
    COMMAND ${CMAKE_COMMAND} -E copy $<TARGET_FILE:${WRAPPER}> ${CMAKE_SOURCE_DIR}/package/live2d/
    COMMAND ${CMAKE_COMMAND} -E make_directory ${WRAPPER_SHADER_OUT}
)

foreach(shaderDir ${WRAPPER_SHADER_DIRS})
    add_custom_command(
        TARGET ${WRAPPER} POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E copy_directory ${shaderDir} ${WRAPPER_SHADER_OUT}
    )
endforeach()