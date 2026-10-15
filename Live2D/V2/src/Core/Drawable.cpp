#include "Drawable.hpp"
#include "Log.hpp"
#include "BinaryReader.hpp"
#include "DEF.hpp"
#include "PivotManager.hpp"
#include "Deformer.hpp"
#include "Moc.hpp"
#include "UtInterpolate.hpp"
#include <string>

namespace Live2D {
namespace V2 {

int Drawable::sTotalMinOrder = Drawable::DEFAULT_ORDER;
int Drawable::sTotalMaxOrder = Drawable::DEFAULT_ORDER;

Drawable::~Drawable() = default;

void Drawable::SetDrawOrders(const std::vector<int>& orders) {
    for (int order : orders) {
        if (order < sTotalMinOrder)
            sTotalMinOrder = order;
        else if (order > sTotalMaxOrder)
            sTotalMaxOrder = order;
    }
}

void Drawable::Read(BinaryReader& br) {
    mId = br.ReadObject<const Id*>();
    mTargetId = br.ReadObject<const Id*>();
    mPivotMgr.reset(br.ReadObject<PivotManager*>());
    mAverageDrawOrder = br.ReadInt32();
    mPivotDrawOrders = br.ReadInt32Array();
    SetDrawOrders(mPivotDrawOrders);
    mPivotOpacities = br.ReadFloat32Array();

    if (br.GetFormatVersion() >= LIVE2D_FORMAT_VERSION_AVAILABLE) {
        const Id* clipId = br.ReadObject<const Id*>();
        if (clipId && !clipId->Str().empty()) {
            const std::string& s = clipId->Str();
            size_t start = 0, end;
            while ((end = s.find(',', start)) != std::string::npos) {
                mClipIDList.push_back(s.substr(start, end - start));
                start = end + 1;
            }
            mClipIDList.push_back(s.substr(start));
        }
    }

    mTextureNo = br.ReadInt32();
    mPointCount = br.ReadInt32();
    mPolygonCount = br.ReadInt32();

    auto obj = br.ReadObject<std::vector<int32_t>>();
    mIndexArray.resize(mPolygonCount * 3);
    for (int i = mPolygonCount * 3 - 1; i >= 0; i--) {
        mIndexArray[i] = static_cast<int16_t>(obj[i]);
    }

    mPivotPoints = br.ReadObject<std::vector<std::vector<float>>>();
    mUvs = br.ReadObject<std::vector<float>>();

    if (br.GetFormatVersion() >= LIVE2D_FORMAT_VERSION_V2_8_TEX_OPTION) {
        mOptionFlag = br.ReadInt32();
        if (mOptionFlag != 0) {
            if ((mOptionFlag & 1) != 0) {
                (void)br.ReadInt32();
                LOGE("Drawable read: not handled option flag");
            }
            if ((mOptionFlag & MASK_COLOR_COMPOSITION) != 0) {
                mColorCompositionType = (mOptionFlag & MASK_COLOR_COMPOSITION) >> 1;
            } else {
                mColorCompositionType = COLOR_COMPOSITION_NORMAL;
            }
            if ((mOptionFlag & 32) != 0) {
                mCulling = false;
            }
        }
    } else {
        mOptionFlag = 0;
    }
}

void Drawable::InitRuntimeState() {
    int vertexCount = mPointCount * VERTEX_STEP;
    bool needXform = NeedTransform();
    mInterpolatedPoints.resize(vertexCount);
    if (needXform)
        mTransformedPoints.resize(vertexCount);

    if (VERTEX_TYPE == VERTEX_TYPE_OFFSET0_STEP2) {
        if (REVERSE_TEXTURE_T) {
            for (int i = mPointCount - 1; i >= 0; i--) {
                int uvOffset = i << 1;
                mUvs[uvOffset + 1] = 1.0f - mUvs[uvOffset + 1];
            }
        }
    } else if (VERTEX_TYPE == VERTEX_TYPE_OFFSET2_STEP5) {
        for (int i = mPointCount - 1; i >= 0; i--) {
            int uvOffset = i << 1;
            int vOffset = i * VERTEX_STEP;
            float uvX = mUvs[uvOffset];
            float uvY = mUvs[uvOffset + 1];
            mInterpolatedPoints[vOffset] = uvX;
            mInterpolatedPoints[vOffset + 1] = uvY;
            mInterpolatedPoints[vOffset + 4] = 0;
            if (needXform) {
                mTransformedPoints[vOffset] = uvX;
                mTransformedPoints[vOffset + 1] = uvY;
                mTransformedPoints[vOffset + 4] = 0;
            }
        }
    }
}

void Drawable::SetupInterpolate(Moc* moc) {
    // drawable 级：插值 drawOrder 与 opacity
    mParamOutside = false;
    mInterpolatedDrawOrder =
        UtInterpolate::InterpolateInt(moc, mPivotMgr.get(), mParamOutside, mPivotDrawOrders);
    // Match Python: skip opacity interpolation if outside param
    if (mParamOutside)
        return;
    mInterpolatedOpacity =
        UtInterpolate::InterpolateFloat(moc, mPivotMgr.get(), mParamOutside, mPivotOpacities);
    if (mParamOutside)
        return;

    // 网格顶点插值
    bool paramOutside = false;
    UtInterpolate::InterpolatePoints(moc,
                                     mPivotMgr.get(),
                                     paramOutside,
                                     mPointCount,
                                     mPivotPoints,
                                     mInterpolatedPoints,
                                     VERTEX_OFFSET,
                                     VERTEX_STEP);
    if (paramOutside)
        mParamOutside = true;
}

void Drawable::SetupTransform(Moc* moc) {
    if (mParamOutside)
        return;

    if (NeedTransform()) {
        auto* targetId = GetTargetId();
        if (mTmpDeformerIndex == DEFORMER_INDEX_NOT_INIT)
            mTmpDeformerIndex = moc->GetDeformerIndex(targetId);

        if (mTmpDeformerIndex < 0) {
            LOGE("deformer not found: %s\n", targetId->Str().c_str());
        } else {
            auto* deformer = moc->GetDeformer(mTmpDeformerIndex);
            if (deformer && !deformer->IsOutsideParam()) {
                deformer->TransformPoints(mInterpolatedPoints,
                                          mTransformedPoints,
                                          mPointCount,
                                          VERTEX_OFFSET,
                                          VERTEX_STEP);
                mAvailable = true;
            } else {
                mAvailable = false;
            }
            mBaseOpacity = deformer->GetTotalOpacity();
        }
    }
}

}   // namespace V2
}   // namespace Live2D
