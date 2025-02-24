SET(MAIN_NAME Main)

# Set directory paths.
set(CORE_PATH ${PROJECT_ROOT}/Core)
set(FRAMEWORK_PATH ${PROJECT_ROOT}/Framework)
set(STB_IMAGE_INCLUDE ${PROJECT_ROOT}/stb_image)
set(GLEW_PATH ${PROJECT_ROOT}/glew)

# Configure for Linux / Windows
if(CMAKE_SYSTEM_NAME MATCHES "Linux")
  # select cubism framework platform support
  set(CSM_TARGET CSM_TARGET_LINUX_GL)
  # select cubism core platform support
  set(CORE_LIB_NAME linux/x86_64/libLive2DCubismCore.a)
  add_definitions(-fPIC)
elseif(CMAKE_SYSTEM_NAME MATCHES "Windows")
  # select cubism framework platform support
  set(CSM_TARGET CSM_TARGET_WIN_GL)
  # select cubism core platform support
  if (CMAKE_CL_64)
    message("Host: x64")
    set(CORE_LIB_NAME windows/x86_64/143/Live2DCubismCore_MT.lib)
  else()
    message("Host: x86")
    set(CORE_LIB_NAME windows/x86/143/Live2DCubismCore_MT.lib)
  endif()
elseif(CMAKE_SYSTEM_NAME MATCHES "Darwin")
  set(CSM_TARGET CSM_TARGET_MAC_GL)
  if(CMAKE_SYSTEM_PROCESSOR MATCHES "arm64")
    set(CORE_LIB_NAME macos/arm64/libLive2DCubismCore.a)
  else()
    set(CORE_LIB_NAME macos/x86_64/libLive2DCubismCore.a)
  endif()
endif()

# Specify version of compiler.
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

# ---Configure OpenGL---
# Add GLEW
option(BUILD_UTILS OFF)
add_subdirectory(${GLEW_PATH}/build/cmake ${CMAKE_BINARY_DIR}/glew_s)

# ---Configure Framework---
# Specify Cubism Framework rendering.
set(FRAMEWORK_SOURCE OpenGL)
# Add Cubism Native Framework.
add_subdirectory(${FRAMEWORK_PATH} ${CMAKE_BINARY_DIR}/Framework)
# Add rendering definition to framework.
target_compile_definitions(Framework PUBLIC ${CSM_TARGET})
# Add include path of GLEW to framework.
target_include_directories(Framework PUBLIC ${GLEW_PATH}/include src)
# If current compiler is mingw, add some options
if(CMAKE_SYSTEM_NAME MATCHES "Windows")
  # extra compile options for mingw on Windows
  if("${CMAKE_CXX_COMPILER_ID}" STREQUAL "GNU")
    target_compile_options(Framework PRIVATE -fpermissive -Wconversion-null)
  endif()  
endif()
# Link libraries to framework.
add_library(Live2DCubismCore STATIC IMPORTED)

# ---CubismCoreLib---
# Import CubismCoreLib
set_target_properties(Live2DCubismCore
  PROPERTIES
    IMPORTED_LOCATION
      ${CORE_PATH}/lib/${CORE_LIB_NAME}
    INTERFACE_INCLUDE_DIRECTORIES
      ${CORE_PATH}/include
)
target_compile_definitions(Live2DCubismCore
  INTERFACE
  ${CSM_TARGET}
)

target_link_libraries(Framework Live2DCubismCore glew_s)

# Find opengl libraries.
find_package(OpenGL REQUIRED)

# Add source files.
add_subdirectory(src)

# Link libraries to app.
target_link_libraries(${MAIN_NAME}
  Framework
  ${OPENGL_LIBRARIES}
)

if(APPLE)
  target_link_libraries(${MAIN_NAME}
    ${OPENGL_LIBRARIES}
    ${COCOA_LIBRARY}
    ${IOKIT_LIBRARY} 
    ${COREVIDEO_LIBRARY}
  )
endif()

# Specify include directories.
target_include_directories(${MAIN_NAME} PUBLIC 
  ${STB_IMAGE_INCLUDE}
  src
)

# 在配置阶段立即执行文件修改脚本
include(${CMAKE_CURRENT_SOURCE_DIR}/insert_code.cmake)