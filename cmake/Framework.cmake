set(FRAMEWORK_SOURCE OpenGL)
# Add Cubism Native Framework.
add_subdirectory(Framework)
# Add rendering definition to framework.
target_compile_definitions(Framework PUBLIC ${CSM_TARGET})
# Add include path of GLEW to framework.
target_include_directories(Framework PUBLIC Glad/include Main/src)
# If current compiler is mingw, add some options
if(CMAKE_SYSTEM_NAME MATCHES "Windows")
  # extra compile options for mingw on Windows
  if("${CMAKE_CXX_COMPILER_ID}" STREQUAL "GNU")
    target_compile_options(Framework PRIVATE -fpermissive -Wconversion-null)
  endif()  
endif()
# Link libraries to framework.
target_link_libraries(Framework Live2DCubismCore glad)