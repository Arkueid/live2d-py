# `Python Develop.SABIModule` is introduced in 3.26
cmake_minimum_required(VERSION 3.26)

project(LAppModelWrapper)

if(APPLE)
  set(CMAKE_CXX_STANDARD 11)
  set(CMAKE_CXX_STANDARD_REQUIRED ON)
  set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11")

  set(CMAKE_OSX_ARCHITECTURES "arm64")
endif()

if("${CMAKE_CXX_COMPILER_ID}" STREQUAL "MSVC")
  set(CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE} /Zi")
  set(CMAKE_C_FLAGS_RELEASE "${CMAKE_C_FLAGS_RELEASE} /Zi")
  add_compile_options("/utf-8" "/wd4018" "/wd4244" "/wd4996")
  add_link_options("/NODEFAULTLIB:LIBCMT")
  if (CMAKE_CL_64)
    add_link_options("/BASE:0x800000000")
  endif()
endif()  

if("${CMAKE_CXX_COMPILER_ID}" STREQUAL "MSVC")
  execute_process(COMMAND chcp 65001)
endif()


include(Live2D/cmake/Live2D.cmake)

# set(VIEWER 1)

if(DEFINED VIEWER)
  message("Build for Viewer")
  include(cmake/Live2DViewer.cmake)
else()
  message("Build for Wrapper")
  include(cmake/Wrapper.cmake)
endif()