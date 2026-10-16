#pragma once
#include <memory>

#include "ISerializable.hpp"
#include "Id.hpp"
#include <vector>

namespace Live2D {
namespace V2 {

class PivotManager;
class Moc;

/**
 * Drawable — drawable 数据与运行时状态一体（原 IDrawData + Mesh 合并，
 * V2 仅有 mesh 一种 draw data 实现）。对标 V3 CubismModel 的 drawable 概念。
 */
class Drawable final : public ISerializable {
public:
    static constexpr int DEFORMER_INDEX_NOT_INIT = -2;
    static constexpr int DEFAULT_ORDER = 500;
    static constexpr int MASK_COLOR_COMPOSITION = 30;
    static constexpr int COLOR_COMPOSITION_NORMAL = 0;
    static constexpr int COLOR_COMPOSITION_SCREEN = 1;
    static constexpr int COLOR_COMPOSITION_MULTIPLY = 2;
    static int sTotalMinOrder;
    static int sTotalMaxOrder;

    Drawable() = default;
    ~Drawable() override;

    void Read(class BinaryReader& br) override;

    void SetupInterpolate(Moc* moc);
    void SetupTransform(Moc* moc);

    /// 初始化运行时状态（顶点缓冲分配与 UV 修正，Moc::init 阶段调用一次）
    void InitRuntimeState();

    const Id* GetId() const { return mId; }
    void SetId(const Id* value) { mId = value; }
    const Id* GetTargetId() const { return mTargetId; }
    void SetTargetId(const Id* value) { mTargetId = value; }
    bool NeedTransform() const { return mTargetId != nullptr && mTargetId != Id::DST_BASE_ID(); }

    float GetOpacity() const { return mInterpolatedOpacity; }
    int GetDrawOrder() const { return mInterpolatedDrawOrder; }

    static void SetDrawOrders(const std::vector<int>& orders);
    static int GetTotalMinOrder() { return sTotalMinOrder; }
    static int GetTotalMaxOrder() { return sTotalMaxOrder; }

    const std::vector<Id*>& GetClipIDList() const { return mClipIDList; }

    void SetTextureNo(int texNo) { mTextureNo = texNo; }
    int GetTextureNo() const { return mTextureNo; }
    int GetColorCompositionType() const { return mColorCompositionType; }
    bool GetCulling() const { return mCulling; }
    const std::vector<float>& GetUvs() const { return mUvs; }
    int GetNumPoints() const { return mPointCount; }
    const std::vector<int16_t>& GetIndexArray() const { return mIndexArray; }

    // ---- 运行时状态 ----
    std::vector<float> mInterpolatedPoints;
    std::vector<float> mTransformedPoints;
    float mBaseOpacity = 1.0f;
    float mInterpolatedOpacity = 1.0f;
    int mInterpolatedDrawOrder = 0;
    int mPartsIndex = 0;
    int mTmpDeformerIndex = DEFORMER_INDEX_NOT_INIT;
    bool mAvailable = true;
    bool mParamOutside = false;

private:
    const Id* mId = nullptr;
    const Id* mTargetId = nullptr;
    std::unique_ptr<PivotManager> mPivotMgr;
    int mAverageDrawOrder = 0;
    std::vector<int> mPivotDrawOrders;
    std::vector<float> mPivotOpacities;
    std::vector<Id*> mClipIDList;

    int mTextureNo = -1;
    int mPointCount = 0;
    int mPolygonCount = 0;
    int mOptionFlag = 0;
    std::vector<int16_t> mIndexArray;
    std::vector<std::vector<float>> mPivotPoints;
    std::vector<float> mUvs;
    int mColorCompositionType = COLOR_COMPOSITION_NORMAL;
    bool mCulling = true;
};

}   // namespace V2
}   // namespace Live2D
