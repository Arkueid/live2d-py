# backend - Vulkan | OpenGL
set(GR_BACKEND Vulkan CACHE STRING "Graphics backend: Vulkan or OpenGL")

set(VIEWER off CACHE BOOL "enable viewer and disable wrapper")

set(DEBUG_ENABLE_CALLSTACK on CACHE BOOL "print backtrace when crashing")

set(FORMAT_UTIL on CACHE BOOL "format source code by batch")

set(DEBUG_ENABLE_ASAN on CACHE BOOL "detect memory corruption")