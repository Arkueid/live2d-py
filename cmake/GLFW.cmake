add_executable(GLFW ${PROJECT_ROOT}/GLFW/Main.cpp)

target_include_directories(GLFW PUBLIC 
    ${PROJECT_ROOT}/GLFW/include
    ${PROJECT_ROOT}/Glad/include
    ${PROJECT_ROOT}/Main/src
    ${PROJECT_ROOT}/Framework/src
    ${PROJECT_ROOT}/Core/include
)
target_link_libraries(GLFW 
    ${PROJECT_ROOT}/GLFW/lib/glfw3.lib
    Main
)

target_link_options(GLFW PUBLIC /NODEFAULTLIB:LIBCMTD /NODEFAULTLIB:MSVCRTD)

set_target_properties(GLFW PROPERTIES
    RUNTIME_OUTPUT_DIRECTORY_DEBUG ${PROJECT_ROOT}/x64/Debug
    RUNTIME_OUTPUT_DIRECTORY_RELEASE ${PROJECT_ROOT}/x64/Release
)