#pragma once
#include <memory>

#include "ClipContext.hpp"
#include <array>
#include <vector>

#include <vulkan/vulkan.h>

namespace Live2D {
namespace V2 {

class ModelContext;
class VkRenderer;
class IDrawData;
class MeshContext;

/**
 * ClippingManagerVulkan — 裁剪遮罩管理（Vulkan 后端）
 * 裁剪数学与 ClippingManagerOpenGL 完全一致（channel 打包 / layout 划分 / 矩阵推导）。
 * 与 GL 版的差异：FBO 换成 offscreen image，遮罩 pass 的渲染命令由调用方
 * （VkRenderer::preDraw）提供 command buffer，本类只负责在其中录制遮罩源 draw call。
 */
class ClippingManagerVulkan {
public:
    explicit ClippingManagerVulkan(VkRenderer& renderer);
    ~ClippingManagerVulkan();

    void init(ModelContext* mc, const std::vector<IDrawData*>& drawDataList,
              const std::vector<MeshContext*>& drawContextList);

    /**
     * @brief 计算裁剪布局并在 mask command buffer 中录制遮罩源绘制。
     * @return 是否有任何激活的裁剪（false 时调用方可跳过 mask pass）
     */
    bool setupClip(ModelContext* mc, VkCommandBuffer cmd);

    void calcClippedDrawTotalBounds(ModelContext* mc, ClipContext* clip);
    void setupLayoutBounds(int count);

private:
    VkRenderer& mRenderer;
    std::vector<std::unique_ptr<ClipContext>> mClipContextList;
    std::vector<std::array<float, 4>> mChannelColors;
};

}   // namespace V2
}   // namespace Live2D
