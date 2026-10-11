# cmake/Config.cmake — 线上默认配置（会提交到仓库）
#
# 本地覆盖：新建 cmake/Config.local.cmake（已被 .gitignore 忽略，不会推送），
# 其中用普通 set() 写覆盖项即可，例如：
#   set(LIVE2D_CPP_TEST on)
# 优先级：Config.local.cmake > -D 命令行 > 本文件默认值
# （local 文件用非 CACHE 的 set，在 include 之后对整个 configure 生效）

# backend - Vulkan | OpenGL
set(GR_BACKEND OpenGL CACHE STRING "Graphics backend: Vulkan or OpenGL")

set(VIEWER off CACHE BOOL "enable viewer and disable wrapper")

set(DEBUG_ENABLE_CALLSTACK on CACHE BOOL "print backtrace when crashing")

set(FORMAT_UTIL on CACHE BOOL "format source code by batch")

set(DEBUG_ENABLE_ASAN off CACHE BOOL "detect memory corruption")

# build the C++ test app (tests/cpp, needs the glfw submodule)
set(LIVE2D_CPP_TEST off CACHE BOOL "build Live2DTest in tests/cpp")