#pragma once

namespace Live2D {
namespace Common {
namespace Backend {

enum Type {
    BACKEND_OPENGL,
    BACKEND_VULKAN,
    BACKEND_NONE,
};

bool IsVulkan();
bool IsOpenGL();
void SetBackend(Type backend);
Type GetBackend();
}   // namespace Backend
}   // namespace Common

}   // namespace Live2D