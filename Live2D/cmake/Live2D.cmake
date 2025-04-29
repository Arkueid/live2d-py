get_filename_component(LIVE2D_ROOT ${CMAKE_CURRENT_LIST_DIR} DIRECTORY)

message("Live2D Python Root: ${LIVE2D_ROOT}")

include(${CMAKE_CURRENT_LIST_DIR}/Core.cmake)
if (CMAKE_SYSTEM_NAME MATCHES "Android")
else()
    include(${CMAKE_CURRENT_LIST_DIR}/Glad.cmake)
    add_library(Live2D::Glad ALIAS glad)
endif()
include(${CMAKE_CURRENT_LIST_DIR}/Framework.cmake)
include(${CMAKE_CURRENT_LIST_DIR}/Main.cmake)

add_library(Live2D::Core ALIAS Live2DCubismCore)
add_library(Live2D::Framework ALIAS Framework)
add_library(Live2D::Main ALIAS Main)