#pragma once
#include <array>

namespace Live2D {
namespace V2 {

// 语义对齐 Python v2 的 L2DModelMatrix（translate/scale 均为绝对赋值）
class ModelMatrix {
public:
    ModelMatrix() { identity(); }
    ModelMatrix(float w, float h);

    void identity();
    void setWidth(float w);
    void setHeight(float h);
    void setCenterPosition(float x, float y);
    void translate(float x, float y);
    void scale(float sx, float sy);
    std::array<float, 16> getArray() const { return mMat; }
    float getScaleX() const { return mMat[0]; }
    float getScaleY() const { return mMat[5]; }
    float invertTransformX(float src) const { return (src - mMat[12]) / mMat[0]; }
    float invertTransformY(float src) const { return (src - mMat[13]) / mMat[5]; }

    float mWidth = 0, mHeight = 0;

private:
    std::array<float, 16> mMat{};
};

}   // namespace V2
}   // namespace Live2D
