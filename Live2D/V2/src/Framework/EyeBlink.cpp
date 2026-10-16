#include "EyeBlink.hpp"
#include "Moc.hpp"
#include <cstdlib>


namespace Live2D {
namespace V2 {
EyeBlink::EyeBlink(Id::IdPool& idPool) {
    mEyeLId = idPool.Resolve("PARAM_EYE_L_OPEN");
    mEyeRId = idPool.Resolve("PARAM_EYE_R_OPEN");
}

void EyeBlink::SetEyeMotion(int closeMs, int closedMs, int openMs) {
    mClosingMs = static_cast<float>(closeMs);
    mClosedMs = static_cast<float>(closedMs);
    mOpeningMs = static_cast<float>(openMs);
}
void EyeBlink::UpdateParam(Moc* moc, float deltaMs) {
    mCurrentTime += deltaMs;
    float now = mCurrentTime;

    switch (mState) {
        case FIRST:
            mNextBlinkTime = now + static_cast<float>(rand() % (2 * (int)mBlinkIntervalMs));
            mState = INTERVAL;
            break;
        case INTERVAL:
            if (now >= mNextBlinkTime) {
                mState = CLOSING;
                mStateStartTime = now;
            }
            break;
        case CLOSING: {
            float elapsed = now - mStateStartTime;
            float v = 1.0f - (elapsed / mClosingMs);
            if (v <= 0) {
                v = 0;
                mState = CLOSED;
                mStateStartTime = now;
            }
            moc->SetParamValue(moc->GetParamIndex(mEyeLId), v);
            moc->SetParamValue(moc->GetParamIndex(mEyeRId), v);
            break;
        }
        case CLOSED: {
            moc->SetParamValue(moc->GetParamIndex(mEyeLId), 0);
            moc->SetParamValue(moc->GetParamIndex(mEyeRId), 0);
            float elapsed = now - mStateStartTime;
            if (elapsed >= mClosedMs) {
                mState = OPENING;
                mStateStartTime = now;
            }
            break;
        }
        case OPENING: {
            float elapsed = now - mStateStartTime;
            float v = elapsed / mOpeningMs;
            if (v >= 1.0f) {
                v = 1.0f;
                mState = INTERVAL;
                mNextBlinkTime =
                    now + mBlinkIntervalMs + static_cast<float>(rand() % (int)mBlinkIntervalMs);
            }
            moc->SetParamValue(moc->GetParamIndex(mEyeLId), v);
            moc->SetParamValue(moc->GetParamIndex(mEyeRId), v);
            break;
        }
    }
}
}   // namespace V2
}   // namespace Live2D
