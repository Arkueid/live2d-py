#pragma once

namespace Live2D {
namespace V2 {

class TargetPoint {
public:
    TargetPoint() = default;

    void Set(float x, float y) {
        mTargetX = x;
        mTargetY = y;
    }
    float GetX() const { return mX; }
    float GetY() const { return mY; }

    // 时间由外部传入（时钟只由 Model 管理）: delta_time_weight = deltaSec * FRAME_RATE
    void Update(float deltaSec);

private:
    // Python l2d_target_point.py 物理核心（速度/加速度限制）
    void UpdateByWeight(float deltaTimeWeight);

    float mX = 0, mY = 0;
    float mTargetX = 0, mTargetY = 0;
    float mFaceVX = 0, mFaceVY = 0;
    static constexpr float sEpsilon = 0.01f;
};

}   // namespace V2
}   // namespace Live2D
