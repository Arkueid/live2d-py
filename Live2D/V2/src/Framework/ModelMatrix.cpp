#include "ModelMatrix.hpp"
#include <cstring>

namespace Live2D {
namespace V2 {

ModelMatrix::ModelMatrix(float w, float h)
    : mWidth(w)
    , mHeight(h) {
    identity();
}

void ModelMatrix::identity() {
    mMat.fill(0);
    mMat[0] = mMat[5] = mMat[10] = mMat[15] = 1.0f;
}

void ModelMatrix::setWidth(float w) {
    float sx = w / mWidth;
    float sy = -sx;
    scale(sx, sy);
}

void ModelMatrix::setHeight(float h) {
    float sx = h / mHeight;
    float sy = -sx;
    scale(sx, sy);
}

void ModelMatrix::setCenterPosition(float x, float y) {
    float w = mWidth * getScaleX();
    float h = mHeight * getScaleY();
    translate(x - w * 0.5f, y - h * 0.5f);
}

void ModelMatrix::translate(float x, float y) {
    // Match Python: absolute assignment, not relative accumulation
    mMat[12] = x;
    mMat[13] = y;
    mMat[14] = 0;
    mMat[15] = 1;
}

void ModelMatrix::scale(float sx, float sy) {
    // Match Python: assignment, NOT multiplication
    mMat[0] = sx;
    mMat[1] = 0;
    mMat[2] = 0;
    mMat[3] = 0;
    mMat[4] = 0;
    mMat[5] = sy;
    mMat[6] = 0;
    mMat[7] = 0;
}

}   // namespace V2
}   // namespace Live2D