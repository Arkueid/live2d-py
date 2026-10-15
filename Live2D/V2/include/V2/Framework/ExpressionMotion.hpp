#pragma once
#include "AMotion.hpp"
#include "ExpressionParam.hpp"
#include "Moc.hpp"
#include <cstdint>
#include <vector>
namespace Live2D {
namespace V2 {
class ExpressionMotion : public AMotion {
public:
    ExpressionMotion();
    void UpdateParam(Moc* model, float timeSec, float weight) override;
    float GetDurationSec() const override { return 1.0f; }
    bool IsLoop() const override { return false; }
    bool IsFinished() const override { return mFinished; }
    void Reset() override { mFinished = false; }
    static ExpressionMotion* Load(const std::vector<uint8_t>& data);
    std::vector<ExpressionParam> mParams;

private:
    bool mFinished = false;
};
}   // namespace V2
}   // namespace Live2D
