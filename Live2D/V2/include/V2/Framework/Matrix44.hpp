#pragma once

#include <array>

namespace Live2D {
namespace V2 {

class Matrix44 {
public:
    Matrix44();

    void identity();
    void setMatrix(const float m[16]);
    void multTranslate(float x, float y, float z);
    void multScale(float sx, float sy, float sz);
    std::array<float, 16>& getArray() { return mMatrix; }

    static Matrix44 mul(const Matrix44& a, const Matrix44& b);

private:
    std::array<float, 16> mMatrix;
};

}   // namespace V2
}   // namespace Live2D