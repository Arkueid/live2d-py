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
    virtual bool IsFinished() const { return false; }
    virtual void Reset() {}
    void SetFadeIn(float sec) { mFadeInSec = sec; }
    void SetFadeOut(float sec) { mFadeOutSec = sec; }
    float mFadeInSec = 1.0f, mFadeOutSec = 1.0f;
    float mWeight = 1.0f;   // Default full weight (fade not yet implemented)
    std::function<void(std::string, int)> mOnStart;
    std::function<void(std::string, int)> mOnFinish;
};
}   // namespace V2
}   // namespace Live2D