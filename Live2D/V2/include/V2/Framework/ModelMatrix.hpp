#pragma once
#include <array>

namespace Live2D {
namespace V2 {

// 语义对齐 Python v2 的 L2DModelMatrix（translate/scale 均为绝对赋值）
class ModelMatrix {
public:
    ModelMatrix() { Identity(); }
    ModelMatrix(float w, float h);

    void Identity();
    void SetWidth(float w);
    void SetHeight(float h);
    void SetCenterPosition(float x, float y);
    void Translate(float x, float y);
    void Scale(float sx, float sy);
    std::array<float, 16> GetArray() const { return mMat; }
    float GetScaleX() const { return mMat[0]; }
    float GetScaleY() const { return mMat[5]; }
    float InvertTransformX(float src) const { return (src - mMat[12]) / mMat[0]; }
    float InvertTransformY(float src) const { return (src - mMat[13]) / mMat[5]; }

    float mWidth = 0, mHeight = 0;

private:
    std::array<float, 16> mMat{};
};

}   // namespace V2
}   // namespace Live2D
