#pragma once
#include "AMotion.hpp"
#include "Motion.hpp"
#include <cstdint>
#include <string>
#include <vector>
namespace Live2D {
namespace V2 {
class BinaryReader;
class Moc;
class Live2DMotion : public AMotion {
public:
    Live2DMotion();
    void UpdateParam(Moc* moc, float timeSec, float weight) override;
    float GetDurationSec() const override;
    bool IsLoop() const override;
    bool IsFinished() const override { return mFinished; }
    void Reset() override { mFinished = false; }
    static Live2DMotion* Load(const std::vector<uint8_t>& data);
    std::vector<Motion> mMotions;
    int mFps = 30;
    float mDurationMs = 0;
    float mLoopDurationMs = 0;
    bool mLoop = false;
    bool mLoopFadeIn = true;
    bool mFinished = false;
};
}   // namespace V2
}   // namespace Live2D