#include "GraphicsBackend.hpp"


namespace Live2D {
namespace Common {
namespace Backend {

// assume that all live2d functions are used on a single thread
static Type sBackend = BACKEND_NONE;

bool IsVulkan() {
    return sBackend == BACKEND_VULKAN;
}

bool IsOpenGL() {
    return sBackend == BACKEND_OPENGL;
}

void SetBackend(Type backend) {
    sBackend = backend;
}

Type GetBackend() {
    return sBackend;
}
}   // namespace Backend
}   // namespace Common
}   // namespace Live2D