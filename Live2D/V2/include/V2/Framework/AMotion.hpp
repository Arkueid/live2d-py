#pragma once
#include <functional>
#include <string>
namespace Live2D {
namespace V2 {
class Moc;
class AMotion {
public:
    virtual ~AMotion() = default;
    virtual void UpdateParam(Moc* moc, float timeSec, float weight) = 0;
    virtual float GetDurationSec() const = 0;
    virtual bool IsLoop() const = 0;
    // 播完状态归 entry 层管理（对齐 V3：ACubismMotion 无 finished 概念，
    // 结束判定由 MotionManager 按 per-entry elapsed/duration 计算）
    void SetFadeIn(float sec) { mFadeInSec = sec; }
    void SetFadeOut(float sec) { mFadeOutSec = sec; }
    float mFadeInSec = 1.0f, mFadeOutSec = 1.0f;
    float mWeight = 1.0f;   // Default full weight (fade not yet implemented)

    // per-motion 回调（对齐 V3 ACubismMotion 的 Began/Finished handler）：
    // group/no 等上下文由挂载方 lambda 捕获；onStart 入队时触发，
    // onFinish 仅在自然播完（非 loop 到 duration / loop 回卷）时触发；
    // 被抢占 fadeOut 结束与 StopAllMotions 均静默移除不触发
    std::function<void()> mOnStart;
    std::function<void()> mOnFinish;
};
}   // namespace V2
}   // namespace Live2D