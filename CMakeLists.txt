cmake_minimum_required(VERSION 3.13)

project(Live2D-py-v2)

set(CMAKE_BUILD_TYPE "Release")
set(ENABLE_LOG TRUE)

message("${CMAKE_BUILD_TYPE}")

set(LIB_NAME LAppModelWrapper_v2)
set(PROJECT_ROOT ${CMAKE_CURRENT_SOURCE_DIR})
set(RES_PATH ${CMAKE_CURRENT_SOURCE_DIR}/Resources)

set(CMAKE_CONFIGURATION_TYPES "Release")

# 寻找Python
set(CMAKE_PREFIX_PATH D:/Python/x86/3.12.0)

set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)
set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/static)

if("${CMAKE_CXX_COMPILER_ID}" STREQUAL "MSVC")
  add_compile_options("/utf-8" "/wd4018" "/wd4244" "/wd4996" "/wd4828")
  add_link_options("/NODEFAULTLIB:LIBCMT")
endif()  

find_package(Python REQUIRED COMPONENTS Development)
message("Python_LIBRARIES " ${Python_LIBRARIES})

add_library(${LIB_NAME} SHARED LAppModelWrapper.cpp)
add_subdirectory(Main)

target_link_libraries(${LIB_NAME} PRIVATE Python::Python live2d)
target_include_directories(${LIB_NAME} PRIVATE ${PROJECT_ROOT}/include)


# Configure for Windows
set(MODULE_NAME ${LIB_NAME}.dll)
set(OUTPUT_NAME live2d.pyd)

# Post build commands
add_custom_command(
  TARGET ${LIB_NAME}
  POST_BUILD
  COMMAND
    ${CMAKE_COMMAND} -E
      copy $<TARGET_FILE_DIR:${LIB_NAME}>/${MODULE_NAME} ${CMAKE_CURRENT_SOURCE_DIR}/package/live2d/v2/${OUTPUT_NAME}
)