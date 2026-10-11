set(V3_TARGET V3)

if(${GR_BACKEND} MATCHES "Vulkan")
  set(CSM_TARGET CSM_TARGET_VULKAN)
  add_compile_definitions(CSM_TARGET_VULKAN=1)
endif()
message("[GR_BACKEND] ${GR_BACKEND}")

include(${CMAKE_CURRENT_LIST_DIR}/Core.cmake)
if (${GR_BACKEND} MATCHES "Vulkan")
  include(${CMAKE_CURRENT_LIST_DIR}/Framework.Vulkan.cmake)
else()
  include(${CMAKE_CURRENT_LIST_DIR}/Framework.OpenGL.cmake)
endif()
# 在配置阶段立即执行文件修改脚本
include(${CMAKE_CURRENT_LIST_DIR}/PatchCubismShader_OpenGLES2.cmake)

add_subdirectory(${LIVE2D_ROOT}/V3/src)

# Vulkan バックエンドでは、v3 ライブラリのビルド時にシェーダー(.spv)も必ずコンパイルする
if(TARGET FrameworkShaders)
  add_dependencies(${V3_TARGET} FrameworkShaders)
endif()

target_compile_definitions(${V3_TARGET} PRIVATE MODULE_LOG_TAG="v3")

set_property(TARGET ${V3_TARGET} PROPERTY CXX_STANDARD 17)
set_property(TARGET ${V3_TARGET} PROPERTY CXX_STANDARD_REQUIRED ON)

target_include_directories(${V3_TARGET} 
  PUBLIC ${LIVE2D_ROOT}/V3/include/V3
  PUBLIC ${LIVE2D_ROOT}/V3/include
)

if(APPLE)
  set(CMAKE_CXX_STANDARD 11)
  set(CMAKE_CXX_STANDARD_REQUIRED ON)
  set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11")

  message(${CMAKE_OSX_ARCHITECTURES})
  find_library(COCOA_LIBRARY Cocoa REQUIRED)
  find_library(IOKIT_LIBRARY IOKit REQUIRED)
  find_library(COREVIDEO_LIBRARY CoreVideo REQUIRED)
endif()

target_link_libraries(${V3_TARGET} PUBLIC
  Framework
  Common
  ${OPENGL_LIBRARIES}
)
if (${GR_BACKEND} MATCHES "Vulkan")
  # VulkanManager / SwapchainManager / VkUtil（原 V3 内文件已迁移至此库）
  target_link_libraries(${V3_TARGET} PUBLIC Live2D::Vulkan)
endif()
if(CMAKE_SYSTEM_NAME STREQUAL "Linux")
  target_link_libraries(${V3_TARGET} PUBLIC stdc++fs)
endif()

if(APPLE)
  target_link_libraries(${V3_TARGET} PUBLIC
    ${COCOA_LIBRARY}
    ${IOKIT_LIBRARY}
    ${COREVIDEO_LIBRARY}
  )
endif()


add_library(Live2D::V3Core ALIAS Live2DCubismCore)
add_library(Live2D::V3Framework ALIAS Framework)
add_library(Live2D::V3 ALIAS ${V3_TARGET})