# ---- V2 Test: loads kasumi model with GLFW + glad ----
#
# Enable with: cmake -DV2CPP_TEST=ON ...
#
# default to off for manylinux workflow
set(V2CPP_TEST 1)

if(DEFINED V2CPP_TEST)
  message("V2CPP.TEST Enabled")
else()
  return()
endif()


# ---- GLFW (git submodule) --------------------------------------------------
set(GLFW_BUILD_DOCS     OFF CACHE INTERNAL "")
set(GLFW_BUILD_TESTS    OFF CACHE INTERNAL "")
set(GLFW_BUILD_EXAMPLES OFF CACHE INTERNAL "")
add_subdirectory(glfw ${CMAKE_CURRENT_BINARY_DIR}/glfw)

# ---- V2Test executable -----------------------------------------------------
add_executable(V2Test
  main.cpp
)

target_link_libraries(V2Test PRIVATE
  Live2D::Live2D
  glfw
)

target_compile_features(V2Test PRIVATE cxx_std_17)

if(MSVC)
  target_compile_options(V2Test PRIVATE "/utf-8" "/wd4018" "/wd4244" "/wd4996")
endif()

# Output next to build dir for easy access to Resources/ (relative path)
set_target_properties(V2Test PROPERTIES
  RUNTIME_OUTPUT_DIRECTORY_RELEASE ${CMAKE_BINARY_DIR}/tests/v2/Release
  RUNTIME_OUTPUT_DIRECTORY_DEBUG   ${CMAKE_BINARY_DIR}/tests/v2/Debug
)

# ---- Vulkan shaders ---------------------------------------------------------
# The CubismRenderer_Vulkan loads "FrameworkShaders/*.spv" relative to the CWD
# at renderer creation time; copy the compiled shaders next to the executable
# (V2Test sets its CWD to the exe directory at startup).
# The .spv output directory is exposed by the v3 library via the
# LIVE2D_SHADER_DIR property on the Framework target.
if(TARGET FrameworkShaders)
  add_custom_command(TARGET V2Test POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E make_directory $<TARGET_FILE_DIR:V2Test>/FrameworkShaders
    COMMAND ${CMAKE_COMMAND} -E copy_directory
            $<TARGET_PROPERTY:Framework,LIVE2D_SHADER_DIR> $<TARGET_FILE_DIR:V2Test>/FrameworkShaders
    COMMENT "Copy Vulkan .spv shaders next to V2Test"
  )
endif()

# Wire into ctest for local use: `ctest --test-dir build` (opens a GLFW window;
# not run in CI since runners are headless)
include(CTest)
if(BUILD_TESTING)
  add_test(NAME V2Test COMMAND V2Test)
endif()
