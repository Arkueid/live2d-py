# Live2D

All c modules required by live2d-py. 

C module list:
* Cubism Native SDK (released by Cubism Live2D)
    * Core
    * Framework 
* Main (Live2D Model)
* Glad (OpenGL, required on Windows, Linux, macOS)

Integrate in your project:

```cmake
include(Live2D/cmake/Live2D.cmake)

target_link_libraries(<target> Live2D::Main)
```
