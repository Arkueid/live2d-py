#pragma once
#include <memory>

#include "ClipContext.hpp"
#include <array>
#include <vector>

namespace Live2D {
namespace V2 {

class Moc;
class GLRenderer;

class ClippingManagerOpenGL {
public:
    explicit ClippingManagerOpenGL(GLRenderer& renderer);
    ~ClippingManagerOpenGL();

    void Init(Moc* moc);
    void SetupClip(Moc* moc);

    /// drawable 对应的 ClipContext（无裁剪返回 nullptr），供渲染器逐 drawable 查询
    ClipContext* GetClipForDrawable(int index) const { return mDrawableClipMap[index]; }

private:
    void CalcClippedDrawTotalBounds(Moc* moc, ClipContext* clip);
    void SetupLayoutBounds(int count);

    GLRenderer& mRenderer;
    std::vector<std::unique_ptr<ClipContext>> mClipContextList;
    // drawable index -> ClipContext（无裁剪为 nullptr），init 阶段建立
    std::vector<ClipContext*> mDrawableClipMap;
    std::vector<std::array<float, 4>> mChannelColors;
};

}   // namespace V2
}   // namespace Live2D
