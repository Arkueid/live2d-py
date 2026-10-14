/**
 * IRenderer — V2 渲染器抽象接口
 * GLRenderer（OpenGL/EGL）与 VkRenderer（Vulkan）共同实现此接口，
 * 与 V3 的 CubismRenderer_OpenGLES2 / CubismRenderer_Vulkan 并列结构一致。
 */

#pragma once
#include <array>
#include <cstdint>
#include <vector>

namespace Live2D {
namespace V2 {

class ModelContext;

class IRenderer
{
public:
    virtual ~IRenderer() = default;

    /// 是否为 Vulkan 后端
    virtual bool isVulkan() const = 0;

    /**
     * @brief 绘制前的处理（裁剪遮罩 pass 等）
     */
    virtual void preDraw(ModelContext* context) = 0;

    /**
     * @brief 绘制所有 drawable
     */
    virtual void draw(ModelContext* context) = 0;

    /**
     * @brief 标记裁剪遮罩内容已变化需重渲（仅 Vulkan 后端使用，GL 默认每帧重渲）
     */
    virtual void setMaskDirty() {}

    /**
     * @brief 绘制单个 drawable（mesh）
     */
    virtual void drawTexture(int texNo, const std::array<float, 4>& screenColor,
                             const std::vector<int16_t>& indices,
                             const std::vector<float>& vertices,
                             const std::vector<float>& uvs,
                             float opacity, int compositionType,
                             const std::array<float, 4>& multiplyColor) = 0;

    virtual void setMatrix(const float m[16]) = 0;
    virtual void setClipMatrix(const float m[16]) = 0;
    virtual void setClipChannel(int channel) = 0;
    virtual void setClipDrawMode(bool on) = 0;
    virtual void setClipMaskMode(bool on) = 0;
    virtual void setCulling(bool cull) = 0;
    virtual void resize(int w, int h) = 0;
};

}   // namespace V2
}   // namespace Live2D
