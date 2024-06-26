cmake_minimum_required(VERSION 3.13)

project(LAppModelWrapper)

set(CMAKE_BUILD_TYPE "Release")
set(ENABLE_LOG TRUE)

message("${CMAKE_BUILD_TYPE}")

set(LIB_NAME LAppModelWrapper)
set(PROJECT_ROOT ${CMAKE_CURRENT_SOURCE_DIR})
set(RES_PATH ${CMAKE_CURRENT_SOURCE_DIR}/Resources)

set(CMAKE_CONFIGURATION_TYPES "Release")

# 寻找Python
set(CMAKE_PREFIX_PATH D:/Python/3.12)

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

if(ENABLE_LOG)
message("Runtime log: on")
# 关闭 log 输出
else()
message("Runtime log: off")
add_compile_definitions(LOG_MODE_RELEASE)
endif()

find_package(Python REQUIRED COMPONENTS Development)

message("Python_LIBRARIES " ${Python_LIBRARIES})

add_subdirectory(Main)

# 创建Python扩展模块
add_library(LAppModelWrapper SHARED LAppModelWrapper.cpp)
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
add_custom_command(
  TARGET ${LIB_NAME}
  POST_BUILD
  COMMAND
    ${CMAKE_COMMAND} -E
      copy $<TARGET_FILE_DIR:${LIB_NAME}>/${MODULE_NAME} ${CMAKE_CURRENT_SOURCE_DIR}/package/live2d/v3/${OUTPUT_NAME}
)
