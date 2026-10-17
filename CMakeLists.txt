# `Python Develop.SABIModule` is introduced in 3.26
cmake_minimum_required(VERSION 3.26)

project(LAppModelWrapper)

if(APPLE)
  if(CMAKE_SYSTEM_PROCESSOR MATCHES "arm64")
    set(CMAKE_OSX_ARCHITECTURES "arm64")
  else()
    set(CMAKE_OSX_ARCHITECTURES "x86_64")
  endif()
endif()

if("${CMAKE_CXX_COMPILER_ID}" STREQUAL "MSVC")
  set(CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE} /Zi")
  set(CMAKE_C_FLAGS_RELEASE "${CMAKE_C_FLAGS_RELEASE} /Zi")
  add_compile_options("/utf-8" "/wd4018" "/wd4244" "/wd4996" "/we4150")
  add_link_options("/NODEFAULTLIB:LIBCMT")
  if(CMAKE_CL_64)
    add_link_options("/BASE:0x800000000")
  endif()
  execute_process(COMMAND chcp 65001)
endif()

# 统一 C++20：（vendored 目录保留自身设置）
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

set(CMAKE_POSITION_INDEPENDENT_CODE ON)

# ---- import config ----
include(cmake/Config.cmake)
# 本地覆盖（不入库，见 .gitignore）：存在时优先级最高
include(cmake/Config.local.cmake OPTIONAL)
include(cmake/VersionCheck.cmake)

# ---- Cubism Native SDK ----
if (DEBUG_ENABLE_ASAN)
  message("[Asan] on")
  add_compile_options(-fsanitize=address -fno-omit-frame-pointer)
  add_link_options(-fsanitize=address -shared-libasan)
endif()

if (DEBUG_ENABLE_CALLSTACK)
  message("[CallStack] on")
  add_compile_definitions(DEBUG_ENABLE_CALLSTACK)
endif()

add_subdirectory(Live2D)

if(VIEWER)
  message("[Target]: Viewer")
  include(cmake/Live2DViewer.cmake)
else()
  message("[Target]: Wrapper")
  # ---- Shared Wrapper config (Python3, output naming) ----
  include(cmake/Wrapper.cmake)

  add_subdirectory(Wrapper ${CMAKE_CURRENT_BINARY_DIR}/Wrapper/)
endif()

# testing
add_subdirectory(tests/cpp ${CMAKE_CURRENT_BINARY_DIR}/tests/cpp)

# --format--
include(cmake/Format.cmake)