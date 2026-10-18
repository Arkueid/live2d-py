#pragma once
#include "AMotion.hpp"
#include "Id.hpp"
#include <cstdint>
#include <vector>


namespace Live2D {
namespace V2 {

class Moc;

class Motion : public AMotion {
    struct MotionData {
        Id* mParamId = nullptr;
        Id* mSecondaryId = nullptr;
        std::vector<float> mValues;
    };

public:
    explicit Motion(Id::IdPool& idPool);
    void UpdateParam(Moc* moc, float timeSec, float weight) override;
    float GetDurationSec() const override;
    bool IsLoop() const override;
    static Motion* Load(const std::vector<uint8_t>& data, Id::IdPool& idPool);
    std::vector<MotionData> mMotions;
    int mFps = 30;
    float mDurationMs = 0;
    float mLoopDurationMs = 0;
    bool mLoop = false;
    bool mLoopFadeIn = true;
    Id::IdPool& mIdPool;
    Id* mVisibleId = nullptr;
};

}   // namespace V2
}   // namespace Live2D