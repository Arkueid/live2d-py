#pragma once

namespace Live2D {
namespace Common {

// V2/V3 统一的模型矩阵管理。
// 内部仅使用 float[16]，不依赖任何 SDK 矩阵类型。
// mvp 组成：P(窗口纵横修正 + 用户缩放/平移) * R(用户旋转) * M(画布适配)
class MatrixManager {
public:
    // 画布适配参数：
    // canvasW/H: 模型画布尺寸（逻辑单位）
    // yDown:     画布 y 轴向下（V2），适配时 y 取负
    // centered:  适配后将画布中心对齐到原点（V2）
    struct Fit {
        float canvasW;
        float canvasH;
        bool yDown;
        bool centered;
    };

    void onResize(int w, int h);
    void setScale(float s);
    void setScaleX(float sx);
    void setScaleY(float sy);
    void setOffset(float x, float y);
    void rotate(float deg);
    // 设置画布适配参数（模型加载时调用一次；整个模型生命周期内不变）
    void setFit(const Fit& fit);

    int getWidth() const { return mWinW; }
    int getHeight() const { return mWinH; }

    // 像素坐标 → 场景坐标（y 向上，纵向范围 [-1,1]，横向按窗口比例）
    void screenToScene(float* x, float* y) const;

    // 仅计算画布适配矩阵 M
    void buildModelMatrix(const Fit& fit, float out[16]) const;
    // 返回 mvp = P * R * M 的缓存（输入与上次不一致时自动重算，否则直接返回缓存）
    const float* getMvp() const;

    // mvp 的逆变换作用于 (x,y)：ndc → 模型画布坐标（旋转≠0 时也正确）
    static void invertTransform(float* x, float* y, const float mvp[16]);

private:
    float mScaleX = 1.0f, mScaleY = 1.0f;
    float mOffsetX = 0.0f, mOffsetY = 0.0f;
    float mRotationDeg = 0.0f;
    int mWinW = 600, mWinH = 600;
    Fit mFit = {0.0f, 0.0f, false, false};

    // mvp 缓存及其输入快照（getMvp 内维护：与当前状态一致则直接返回缓存。
    // mutable：缓存是备忘录实现细节，对外保持逻辑只读）
    mutable float mMvpCache[16] = {};
    mutable bool mCacheValid = false;
    mutable Fit mCachedFit = {0.0f, 0.0f, false, false};
    mutable float mCachedScaleX = 0.0f, mCachedScaleY = 0.0f;
    mutable float mCachedOffsetX = 0.0f, mCachedOffsetY = 0.0f;
    mutable float mCachedRotation = 0.0f;
    mutable int mCachedWinW = 0, mCachedWinH = 0;
};

}   // namespace Common
}   // namespace Live2D
