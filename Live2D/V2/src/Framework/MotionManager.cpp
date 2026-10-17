#include "MotionManager.hpp"
#include <cmath>
namespace Live2D {
namespace V2 {
MotionManager::MotionManager() = default;
bool MotionManager::ReserveMotion(int priority) {
    if (priority < mReservePriority)
        return false;
    if (priority < mCurrentPriority)
        return false;
    mReservePriority = priority;
    return true;
}
int MotionManager::StartMotion(AMotion* motion, bool autoPriority) {
    // Fade out existing motions, matching Python v2: shorter end time wins
    for (auto& e : mMotions) {
        if (e.mFadeOut > 0) {
            float newEnd = e.mElapsedMs + e.mFadeOut * 1000.0f;
            if (e.mFadeOutEndElapsedMs < 0 || newEnd < e.mFadeOutEndElapsedMs)
                e.mFadeOutEndElapsedMs = newEnd;
        }
    }
    motion->Reset();   // Reset finished flag for re-used motion objects
    mMotions.push_back({motion, motion->mFadeInSec, motion->mFadeOutSec});
    return (int)mMotions.size() - 1;
}
int MotionManager::StartMotionPrio(AMotion* motion, int priority) {
    if (priority == mReservePriority)
        mReservePriority = 0;
    mCurrentPriority = priority;
    return StartMotion(motion, false);
}
// Easing: 0.5 - 0.5*cos(x*pi), clamped [0,1]
static float easeSine(float x) {
    if (x <= 0)
        return 0;
    if (x >= 1)
        return 1;
    return 0.5f - 0.5f * cosf(x * 3.14159265f);
}

bool MotionManager::UpdateParam(Moc* moc, float dtMs) {
    bool updated = false;
    for (size_t i = 0; i < mMotions.size();) {
        auto& e = mMotions[i];
        if (!e.mStarted) {
            e.mStarted = true;
        }
        e.mElapsedMs += dtMs;
        float elapsed = e.mElapsedMs / 1000.0f;

        // Fade-in weight
        float fadeIn = 1.0f;
        if (e.mFadeIn > 0) {
            e.mFadeInElapsedMs += dtMs;
            fadeIn = easeSine(e.mFadeInElapsedMs / (e.mFadeIn * 1000.0f));
        }
        // Fade-out weight
        float fadeOut = 1.0f;
        if (e.mFadeOut > 0 && e.mFadeOutEndElapsedMs >= 0) {
            float remaining =
                (e.mFadeOutEndElapsedMs - e.mElapsedMs) / (e.mFadeOut * 1000.0f);
            if (remaining <= 0) {
                e.mFinished = true;
                fadeOut = 0;
            } else {
                fadeOut = easeSine(remaining);
            }
        }

        float weight = e.mMotion->mWeight * fadeIn * fadeOut;
        if (weight < 0)
            weight = 0;
        if (weight > 1)
            weight = 1;

        e.mMotion->UpdateParam(moc, elapsed, weight);
        updated = true;
        if (e.mFinished || e.mMotion->IsFinished()) {
            mMotions.erase(mMotions.begin() + i);
        } else {
            i++;
        }
    }
    if (mMotions.empty())
        mCurrentPriority = 0;
    return updated;
}
bool MotionManager::IsFinished() const {
    return mMotions.empty();
}
void MotionManager::StopAllMotions() {
    mMotions.clear();
}
}   // namespace V2
}   // namespace Live2D