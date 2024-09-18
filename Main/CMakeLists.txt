set(LIB_PATH ${PROJECT_ROOT}/lib)
set(LIVE2D_INCLUDE ${PROJECT_ROOT}/include/live2d)
set(LOCAL_INCLUDE_PATH ${PROJECT_ROOT}/include)
set(FRAMEWORK_PATH ${PROJECT_ROOT}/Framework)
set(GLEW_PATH ${PROJECT_ROOT}/glew)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

add_subdirectory(${GLEW_PATH}/build/cmake ${CMAKE_BINARY_DIR}/glew_s)

add_subdirectory(${FRAMEWORK_PATH} ${CMAKE_BINARY_DIR}/Framework)

add_library(Live2DOpenGL STATIC IMPORTED)
set_target_properties(Live2DOpenGL
  PROPERTIES
    IMPORTED_LOCATION
      ${LIB_PATH}/live2d_opengl.lib
    INTERFACE_INCLUDE_DIRECTORIES
      ${LIVE2D_INCLUDE}
)

add_library(live2d STATIC
    Default.hpp
    FileManager.h
    FileManager.cpp
    LAppDefine.h
    LAppModel.h
    LAppModel.cpp
    LAppTextureDesc.h
    LAppTextureDesc.cpp
    LAppWavFileHandler.hpp
    LAppWavFileHandler.cpp
    Log.hpp
    Log.cpp
    MatrixManager.h
    MatrixManager.cpp
    ModelSetting.h
    ModelSettingJson.h
    PlatformManager.h
    PlatformManager.cpp
)
target_compile_definitions(live2d PRIVATE L2D_TARGET_WIN_GL)
target_link_libraries(live2d Live2DOpenGL Framework glew_s)
target_include_directories(live2d PUBLIC ${CMAKE_CURRENT_SOURCE_DIR} ${LIVE2D_INCLUDE} ${LOCAL_INCLUDE_PATH})