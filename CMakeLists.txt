cmake_minimum_required(VERSION 3.13)

project(LAppModelWrapper)

set(LIB_NAME LAppModelWrapper)

set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib/${CMAKE_BUILD_TYPE})
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin/${CMAKE_BUILD_TYPE})
set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/static/${CMAKE_BUILD_TYPE})

set(PROJECT_ROOT ${CMAKE_CURRENT_SOURCE_DIR})

set(RES_PATH ${CMAKE_CURRENT_SOURCE_DIR}/Resources)

# 寻找Python
find_package(Python REQUIRED COMPONENTS Development)

add_subdirectory(Main)

# 创建Python扩展模块
add_library(LAppModelWrapper MODULE LAppModelWrapper.cpp)
target_link_libraries(LAppModelWrapper PRIVATE Python::Python live2d)

# Post build commands
add_custom_command(
  TARGET ${LIB_NAME}
  POST_BUILD
  COMMAND
    ${CMAKE_COMMAND} -E
      copy $<TARGET_FILE_DIR:${LIB_NAME}>/lib${LIB_NAME}.so ${CMAKE_CURRENT_SOURCE_DIR}/example/live2d.so
)

