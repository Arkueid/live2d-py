#include "MatrixManager.hpp"
#include <algorithm>
#include <cmath>
#include <cstring>

namespace Live2D {
namespace Common {

namespace {
// 4x4 矩阵乘法 r = a * b（column-major）
void mul(float r[16], const float a[16], const float b[16]) {
    float t[16] = {};
    for (int i = 0; i < 4; i++)
        for (int j = 0; j < 4; j++)
            t[i + j * 4] = a[i] * b[j * 4] + a[i + 4] * b[j * 4 + 1] + a[i + 8] * b[j * 4 + 2] +
                           a[i + 12] * b[j * 4 + 3];
    std::memcpy(r, t, sizeof(t));
}
}   // namespace

void MatrixManager::onResize(int w, int h) {
    mWinW = w;
    mWinH = h;
}
void MatrixManager::setScale(float s) {
    mScaleX = s;
    mScaleY = s;
}
void MatrixManager::setScaleX(float sx) {
    mScaleX = sx;
}
void MatrixManager::setScaleY(float sy) {
    mScaleY = sy;
}
void MatrixManager::setOffset(float x, float y) {
    mOffsetX = x;
    mOffsetY = y;
}
void MatrixManager::rotate(float deg) {
    mRotationDeg = deg;
}
void MatrixManager::setFit(const Fit& fit) {
    mFit = fit;
}

void MatrixManager::screenToScene(float* x, float* y) const {
    float w = (float)mWinW;
    float h = (float)mWinH;
    *x = (*x - w * 0.5f) * 2.0f / h;
    *y = (*y - h * 0.5f) * -2.0f / h;
}

void MatrixManager::buildModelMatrix(const Fit& fit, float out[16]) const {
    float cw = fit.canvasW > 0 ? fit.canvasW : 1.0f;
    float ch = fit.canvasH > 0 ? fit.canvasH : 1.0f;

    // contain-fit：场景坐标短边恒为 2、长边随窗口比例扩展，
    // base 取宽/高两方向约束的最小值——完整显示前提下的最大填充
    float aspect = mWinH > 0 ? (float)mWinW / (float)mWinH : 1.0f;
    float sceneW = 2.0f * (aspect > 1.0f ? aspect : 1.0f);
    float sceneH = 2.0f * (aspect > 1.0f ? 1.0f : 1.0f / aspect);
    float base = std::min(sceneW / cw, sceneH / ch);

    float sx = base;
    float sy = fit.yDown ? -base : base;
    float tx = fit.centered ? -cw * sx * 0.5f : 0.0f;
    float ty = fit.centered ? -ch * sy * 0.5f : 0.0f;

    std::memset(out, 0, 16 * sizeof(float));
    out[0] = sx;
    out[5] = sy;
    out[10] = 1.0f;
    out[15] = 1.0f;
    out[12] = tx;
    out[13] = ty;
}

const float* MatrixManager::getMvp() const {
    // 与上次计算所用状态完全一致时直接返回缓存
    if (mCacheValid && mScaleX == mCachedScaleX && mScaleY == mCachedScaleY &&
        mOffsetX == mCachedOffsetX && mOffsetY == mCachedOffsetY &&
        mRotationDeg == mCachedRotation && mWinW == mCachedWinW && mWinH == mCachedWinH &&
        mFit.canvasW == mCachedFit.canvasW && mFit.canvasH == mCachedFit.canvasH &&
        mFit.yDown == mCachedFit.yDown && mFit.centered == mCachedFit.centered) {
        return mMvpCache;
    }
    mCacheValid = true;
    mCachedScaleX = mScaleX;
    mCachedScaleY = mScaleY;
    mCachedOffsetX = mOffsetX;
    mCachedOffsetY = mOffsetY;
    mCachedRotation = mRotationDeg;
    mCachedWinW = mWinW;
    mCachedWinH = mWinH;
    mCachedFit = mFit;

    // 1. 投影矩阵：窗口纵横修正 + 用户缩放/平移
    float proj[16] = {};
    proj[0] = proj[5] = proj[10] = proj[15] = 1.0f;
    if (mWinH > mWinW) {
        // 竖屏
        proj[5] = (float)mWinW / (float)mWinH;
    } else {
        // 横屏
        proj[0] = (float)mWinH / (float)mWinW;
    }
    proj[0] *= mScaleX;
    proj[5] *= mScaleY;
    proj[12] = mOffsetX;
    proj[13] = mOffsetY;

    // 2. 模型矩阵：旋转作用于投影之前（P * (R * M)），避免非方形窗口下剪切变形
    float mm[16];
    buildModelMatrix(mFit, mm);
    if (mRotationDeg != 0.0f) {
        float r = mRotationDeg * 3.14159265f / 180.0f;
        float rot[16] = {};
        rot[0] = std::cos(r);
        rot[4] = std::sin(r);
        rot[1] = -std::sin(r);
        rot[5] = std::cos(r);
        rot[10] = 1.0f;
        rot[15] = 1.0f;
        float rm[16];
        mul(rm, rot, mm);
        std::memcpy(mm, rm, sizeof(mm));
    }

    // 3. mvp = P * (R * M)
    mul(mMvpCache, proj, mm);
    return mMvpCache;
}

void MatrixManager::invertTransform(float* x, float* y, const float mvp[16]) {
    // getMvp 生成的矩阵 z 轴恒为直通（mvp[8..11] = 0,0,1,0），
    // x/y 构成二维仿射变换，对仿射块求逆即可
    float det = mvp[0] * mvp[5] - mvp[4] * mvp[1];
    if (det == 0.0f)
        return;
    float tx = *x - mvp[12];
    float ty = *y - mvp[13];
    *x = (mvp[5] * tx - mvp[4] * ty) / det;
    *y = (-mvp[1] * tx + mvp[0] * ty) / det;
}

}   // namespace Common
}   // namespace Live2D
