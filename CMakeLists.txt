cmake_minimum_required(VERSION 3.13)

project(LAppModelWrapper)

set(LIB_NAME LAppModelWrapper)

# set(CMAKE_BUILD_TYPE "Debug")

if("${CMAKE_CXX_COMPILER_ID}" STREQUAL "MSVC")
  set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)
  set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)
  set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/static)
  add_compile_options("/utf-8" "/wd4018" "/wd4244" "/wd4996")
  add_link_options("/NODEFAULTLIB:LIBCMT")
else()
  set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib/${CMAKE_BUILD_TYPE})
  set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin/${CMAKE_BUILD_TYPE})
  set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/static/${CMAKE_BUILD_TYPE})
endif()  

# Post build commands
if(CMAKE_BUILD_TYPE MATCHES "Release")
message("Runtime log: off")
# 关闭 log 输出
add_compile_definitions(LOG_MODE_RELEASE)
elseif(CMAKE_BUILD_TYPE MATCHES "Debug")
message("Runtime log: on")
endif()

set(PROJECT_ROOT ${CMAKE_CURRENT_SOURCE_DIR})

set(RES_PATH ${CMAKE_CURRENT_SOURCE_DIR}/Resources)

# 寻找Python
list(APPEND CMAKE_PREFIX_PATH D:/Miniconda)

find_package(Python REQUIRED COMPONENTS Development)

add_subdirectory(Main)

# 创建Python扩展模块
add_library(LAppModelWrapper MODULE LAppModelWrapper.cpp)
target_link_libraries(LAppModelWrapper PRIVATE Python::Python live2d)

# Configure for Linux / Windows
if(CMAKE_SYSTEM_NAME MATCHES "Linux")
  set(MODULE_NAME lib${LIB_NAME}.so)
  set(OUTPUT_NAME live2d.so)
else(CMAKE_SYSTEM_NAME MATCHES "Windows")
  set(MODULE_NAME ${LIB_NAME}.dll)
  set(OUTPUT_NAME live2d.pyd)
endif()

# Post build commands
if(CMAKE_BUILD_TYPE MATCHES "Release")
# 关闭 log 输出
add_compile_definitions(LOG_MODE_RELEASE)
add_custom_command(
  TARGET ${LIB_NAME}
  POST_BUILD
  COMMAND
    ${CMAKE_COMMAND} -E
      copy $<TARGET_FILE_DIR:${LIB_NAME}>/${MODULE_NAME} ${CMAKE_CURRENT_SOURCE_DIR}/package/live2d/${OUTPUT_NAME}
  COMMAND
    ${CMAKE_COMMAND} -E
      copy_directory ${CMAKE_CURRENT_SOURCE_DIR}/package/live2d ${CMAKE_CURRENT_SOURCE_DIR}/example/live2d
  COMMAND
    ${CMAKE_COMMAND} -E
      copy_directory ${CMAKE_CURRENT_SOURCE_DIR}/package/live2d ${CMAKE_CURRENT_SOURCE_DIR}/live2d-desktop/live2d
)
elseif(CMAKE_BUILD_TYPE MATCHES "Debug")
add_custom_command(
  TARGET ${LIB_NAME}
  POST_BUILD
  COMMAND
    ${CMAKE_COMMAND} -E
      copy ${CMAKE_CURRENT_SOURCE_DIR}/package/live2d/live2d.pyi ${CMAKE_CURRENT_SOURCE_DIR}/package/live2d/debug
  COMMAND
    ${CMAKE_COMMAND} -E
      copy ${CMAKE_CURRENT_SOURCE_DIR}/package/live2d/__init__.py ${CMAKE_CURRENT_SOURCE_DIR}/package/live2d/debug
  COMMAND
    ${CMAKE_COMMAND} -E
      copy $<TARGET_FILE_DIR:${LIB_NAME}>/${MODULE_NAME} ${CMAKE_CURRENT_SOURCE_DIR}/package/live2d/debug/${OUTPUT_NAME}
  COMMAND
    ${CMAKE_COMMAND} -E
      copy_directory ${CMAKE_CURRENT_SOURCE_DIR}/package/live2d ${CMAKE_CURRENT_SOURCE_DIR}/example/live2d/
  COMMAND
    ${CMAKE_COMMAND} -E
      copy_directory ${CMAKE_CURRENT_SOURCE_DIR}/package/live2d ${CMAKE_CURRENT_SOURCE_DIR}/live2d-desktop/live2d
)
endif()

add_custom_command(
  TARGET ${LIB_NAME}
  POST_BUILD
  COMMAND
    ${CMAKE_COMMAND} -E
      copy_directory ${CMAKE_CURRENT_SOURCE_DIR}/Resources ${CMAKE_CURRENT_SOURCE_DIR}/live2d-desktop/Resources
)

# add_executable(test main.cpp)
# add_subdirectory(thirdParty/glfw)
# target_link_libraries(test PRIVATE Python::Python live2d glfw)