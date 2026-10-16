#pragma once
#include "Id.hpp"

namespace Live2D {
namespace V2 {
class Moc;
class EyeBlink {
public:
    EyeBlink(Id::IdPool& idPool);
    void SetInterval(int ms) { mBlinkIntervalMs = ms; }
    void SetEyeMotion(int closeMs, int closedMs, int openMs);
    // 时间由外部传入（时钟只由 Model 管理）: deltaMs 为本次时间步长（毫秒），内部累计 elapsed
    void UpdateParam(Moc* moc, float deltaMs);
    float mBlinkIntervalMs = 5000;

private:
    enum State { FIRST, INTERVAL, CLOSING, CLOSED, OPENING };
    State mState = FIRST;
    float mClosingMs = 150, mClosedMs = 80, mOpeningMs = 220;
    float mCurrentTime = 0, mNextBlinkTime = 0, mStateStartTime = 0;
    bool mCloseIfZero = true;
    Id* mEyeLId;
    Id* mEyeRId;
};
}   // namespace V2
}   // namespace Live2D