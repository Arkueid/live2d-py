#pragma once
#include "AMotion.hpp"
#include "ExpressionParam.hpp"
#include "ModelContext.hpp"
#include <cstdint>
#include <vector>
namespace Live2D {
namespace V2 {
class ExpressionMotion : public AMotion {
public:
    ExpressionMotion();
    void updateParam(ModelContext* model, float timeSec, float weight) override;
    float getDurationSec() const override { return 1.0f; }
    bool isLoop() const override { return false; }
    bool isFinished() const override { return mFinished; }
    void reset() override { mFinished = false; }
    static ExpressionMotion* load(const std::vector<uint8_t>& data);
    std::vector<ExpressionParam> mParams;

private:
    bool mFinished = false;
};
}   // namespace V2
}   // namespace Live2D
