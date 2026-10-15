#pragma once

#include "ISerializable.hpp"
#include "Id.hpp"

namespace Live2D {
namespace V2 {

/**
 * Parameter — 参数静态定义（id + min/max/default），
 * 对标 V3 Core 层 csmParameter（Framework 经 CubismModel::GetParameter* 暴露）。
 */
class Parameter final : public ISerializable {
public:
    Parameter() = default;

    void Read(class BinaryReader& br) override;

    float GetMinValue() const { return mMinValue; }
    float GetMaxValue() const { return mMaxValue; }
    float GetDefaultValue() const { return mDefaultValue; }
    const Id* GetId() const { return mParamId; }

private:
    float mMinValue = 0.0f;
    float mMaxValue = 0.0f;
    float mDefaultValue = 0.0f;
    const Id* mParamId = nullptr;
};

}   // namespace V2
}   // namespace Live2D
