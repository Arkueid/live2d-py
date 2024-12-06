cmake_minimum_required(VERSION 3.13)

project(LAppModelWrapper)

if("${CMAKE_CXX_COMPILER_ID}" STREQUAL "MSVC")
  execute_process(
      COMMAND chcp 65001
  )
endif()

# set(CMAKE_BUILD_TYPE "Release")

message("${CMAKE_BUILD_TYPE}")

set(LIB_NAME LAppModelWrapper)
set(PROJECT_ROOT ${CMAKE_CURRENT_SOURCE_DIR})
set(RES_PATH ${CMAKE_CURRENT_SOURCE_DIR}/Resources)

# prevent python root dir being shadowed by registry
set(Python3_FIND_REGISTRY "NEVER")

if(DEFINED PYTHON_INSTALLATION_PATH)
    message("Found PYTHON_INSTALLATION_PATH in environment variables")
    set(CMAKE_PREFIX_PATH ${PYTHON_INSTALLATION_PATH})
else()
    # 寻找Python
    message("Not found PYTHON_INSTALLATION_PATH in environment variables. \nUse default path.")
    set(CMAKE_PREFIX_PATH D:/Python/x86/3.12.0)
endif()

if("${CMAKE_CXX_COMPILER_ID}" STREQUAL "MSVC")
  set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)
  set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)
  set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/static)
  set(CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE} /Zi")
  set(CMAKE_C_FLAGS_RELEASE "${CMAKE_C_FLAGS_RELEASE} /Zi")
  add_compile_options("/utf-8" "/wd4018" "/wd4244" "/wd4996")
  add_link_options("/NODEFAULTLIB:LIBCMT" "/DEBUG")
else()
  set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib/${CMAKE_BUILD_TYPE})
  set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin/${CMAKE_BUILD_TYPE})
  set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/static/${CMAKE_BUILD_TYPE})
endif()  

find_package(Python3 REQUIRED Development)

message("Python_LIBRARIES " ${Python_LIBRARIES})

add_subdirectory(Main)

# 创建Python扩展模块
add_library(LAppModelWrapper SHARED LAppModelWrapper.cpp)
target_link_libraries(LAppModelWrapper PRIVATE Python3::Python live2d)

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
