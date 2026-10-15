#pragma once

#include <array>

namespace Live2D {
namespace V2 {

class Matrix44 {
public:
    Matrix44();

    void Identity();
    void SetMatrix(const float m[16]);
    void MultTranslate(float x, float y, float z);
    void MultScale(float sx, float sy, float sz);
    std::array<float, 16>& GetArray() { return mMatrix; }

    static Matrix44 Mul(const Matrix44& a, const Matrix44& b);

private:
    std::array<float, 16> mMatrix;
};

}   // namespace V2
}   // namespace Live2D