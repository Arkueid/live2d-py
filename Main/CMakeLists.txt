SET(APP_NAME live2d)

# Set directory paths.
set(CORE_PATH ${PROJECT_ROOT}/Core)
set(FRAMEWORK_PATH ${PROJECT_ROOT}/Framework)
set(LOCAL_INCLUDE_PATH ${PROJECT_ROOT}/include)
set(GLEW_PATH ${PROJECT_ROOT}/glew)

# Configure for Linux / Windows
if(CMAKE_SYSTEM_NAME MATCHES "Linux")
  # select cubism framework platform support
  set(CSM_TARGET CSM_TARGET_LINUX_GL)
  # select cubism core platform support
  set(CORE_LIB_NAME linux/libLive2DCubismCore.a)
  add_definitions(-fPIC)
else(CMAKE_SYSTEM_NAME MATCHES "Windows")
  # select cubism framework platform support
  set(CSM_TARGET CSM_TARGET_WIN_GL)
  # select cubism core platform support
  if (CMAKE_CL_64)
    message("Host: x64")
    set(CORE_LIB_NAME windows/x64/Live2DCubismCore_MT.lib)
  else()
    message("Host: x86")
    set(CORE_LIB_NAME windows/x86/Live2DCubismCore_MT.lib)
  endif()
endif()

# Specify version of compiler.
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)


# ---Configure OpenGL---
# Add GLEW ,GLFW.
add_subdirectory(${GLEW_PATH}/build/cmake ${CMAKE_BINARY_DIR}/glew_s)

# ---Configure Framework---
# Specify Cubism Framework rendering.
set(FRAMEWORK_SOURCE OpenGL)
# Add Cubism Native Framework.
add_subdirectory(${FRAMEWORK_PATH} ${CMAKE_BINARY_DIR}/Framework)
# Add rendering definition to framework.
target_compile_definitions(Framework PRIVATE ${CSM_TARGET})
# Add include path of GLEW to framework.
target_include_directories(Framework PUBLIC ${GLEW_PATH}/include)
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
add_subdirectory(live2d)

# Link libraries to app.
target_link_libraries(${APP_NAME}
  Framework
  ${OPENGL_LIBRARIES}
)

# Specify include directories.
target_include_directories(${APP_NAME} PUBLIC 
  ${LOCAL_INCLUDE_PATH}
  live2d
)




