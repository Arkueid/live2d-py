# ---- Live2D Test: loads a v2/v3 model (auto-detected) with GLFW ----
#
# 开关在 cmake/Config.cmake（LIVE2D_CPP_TEST），本地可用 cmake/Config.local.cmake 覆盖

if(NOT LIVE2D_CPP_TEST)
  return()
endif()
message("LIVE2D.CPP.TEST Enabled")


# ---- GLFW (git submodule) --------------------------------------------------
set(GLFW_BUILD_DOCS     OFF CACHE INTERNAL "")
set(GLFW_BUILD_TESTS    OFF CACHE INTERNAL "")
set(GLFW_BUILD_EXAMPLES OFF CACHE INTERNAL "")
add_subdirectory(glfw ${CMAKE_CURRENT_BINARY_DIR}/glfw)

# ---- Live2DTest executable ---------------------------------------------------
add_executable(Live2DTest
  main.cpp
)

target_link_libraries(Live2DTest PRIVATE
  Live2D::Live2D
  glfw
)

target_compile_features(Live2DTest PRIVATE cxx_std_17)

if(MSVC)
  target_compile_options(Live2DTest PRIVATE "/utf-8" "/wd4018" "/wd4244" "/wd4996")
endif()

# Output next to build dir for easy access to Resources/ (relative path)
set_target_properties(Live2DTest PROPERTIES
  RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/tests/cpp
)

# ---- Vulkan shaders ---------------------------------------------------------
# The CubismRenderer_Vulkan loads "FrameworkShaders/*.spv" relative to the CWD
# at renderer creation time; copy the compiled shaders next to the executable
# (Live2DTest sets its CWD to the exe directory at startup).
# The .spv output directory is exposed by the v3 library via the
# LIVE2D_SHADER_DIR property on the Framework target.
if(TARGET FrameworkShaders)
  add_custom_command(TARGET Live2DTest POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E make_directory $<TARGET_FILE_DIR:Live2DTest>/FrameworkShaders
    COMMAND ${CMAKE_COMMAND} -E copy_directory
            $<TARGET_PROPERTY:Framework,LIVE2D_SHADER_DIR> $<TARGET_FILE_DIR:Live2DTest>/FrameworkShaders
    COMMENT "Copy Vulkan .spv shaders next to Live2DTest"
  )
endif()

# V2 Vulkan shaders — VkRenderer loads "FrameworkShaders/V2*.spv" relative to the CWD;
# exposed by the v2 library via the LIVE2D_SHADER_DIR property on V2Shaders.
# 与 v3 Framework shader 放同一目录（文件名带 V2 前缀不冲突）。
if(TARGET V2Shaders)
  add_custom_command(TARGET Live2DTest POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_directory
            $<TARGET_PROPERTY:V2Shaders,LIVE2D_SHADER_DIR> $<TARGET_FILE_DIR:Live2DTest>/FrameworkShaders
    COMMENT "Copy V2 Vulkan .spv shaders next to Live2DTest"
  )
endif()

# Wire into ctest for local use: `ctest --test-dir build` (opens a GLFW window;
# not run in CI since runners are headless)
include(CTest)
if(BUILD_TESTING)
  add_test(NAME Live2DTest COMMAND Live2DTest)
endif()
