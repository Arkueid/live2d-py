#pragma once
#include <memory>

#include "ClipContext.hpp"
#include <array>
#include <vector>

namespace Live2D {
namespace V2 {

class Moc;
class VkRenderer;

/**
 * ClippingManagerVulkan — 裁剪遮罩管理（Vulkan 后端）
 * 裁剪数学与 ClippingManagerOpenGL 完全一致（channel 打包 / layout 划分 / 矩阵推导）。
 * 与 GL 版的差异：FBO 换成 offscreen image，遮罩 pass 的渲染区间由调用方
 * （VkRenderer::preDraw）开关，本类只负责在其中录制遮罩源 draw call。
 */
class ClippingManagerVulkan {
public:
    explicit ClippingManagerVulkan(VkRenderer& renderer);
    ~ClippingManagerVulkan();

    void Init(Moc* moc);

    /**
     * @brief 计算裁剪布局并录制遮罩源绘制。
     * @return 是否有任何激活的裁剪（false 时调用方可跳过 mask pass）
     */
    bool SetupClip(Moc* moc);

    /// drawable 对应的 ClipContext（无裁剪返回 nullptr），供渲染器逐 drawable 查询
    ClipContext* GetClipForDrawable(int index) const { return mDrawableClipMap[index]; }

private:
    void CalcClippedDrawTotalBounds(Moc* moc, ClipContext* clip);
    void SetupLayoutBounds(int count);

    VkRenderer& mRenderer;
    std::vector<std::unique_ptr<ClipContext>> mClipContextList;
    // drawable index -> ClipContext（无裁剪为 nullptr），init 阶段建立
    std::vector<ClipContext*> mDrawableClipMap;
    std::vector<std::array<float, 4>> mChannelColors;
};

}   // namespace V2
}   // namespace Live2D
