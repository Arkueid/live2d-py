#pragma once

#include <array>
#include <vector>

namespace Live2D {
namespace V2 {

// 一组共享同一 mask 索引集合的 clipped drawable 及其遮罩布局状态
class ClipContext {
public:
    ClipContext() = default;

    std::vector<int> mClippedDrawIndexList;
    std::vector<int> mClippingMaskDrawIndexList;
    bool mIsUsing = false;
    int mLayoutChannelNo = 0;
    std::array<float, 4> mLayoutBounds = {0, 0, 1, 1};
    std::array<float, 4> mAllClippedDrawRect = {0, 0, 0, 0};
    std::array<float, 16> mMatrixForMask{};
    std::array<float, 16> mMatrixForDraw{};
};

}   // namespace V2
}   // namespace Live2D
