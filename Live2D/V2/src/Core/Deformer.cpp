#include "Deformer.hpp"
#include "BinaryReader.hpp"
#include "DEF.hpp"
#include "PivotManager.hpp"
#include "Moc.hpp"
#include "UtInterpolate.hpp"

namespace Live2D {
namespace V2 {

void Deformer::Read(BinaryReader& br) {
    mId = br.ReadObject<const Id*>();
    mTargetId = br.ReadObject<const Id*>();
}

void Deformer::ReadOpacity(BinaryReader& br) {
    if (br.GetFormatVersion() >= LIVE2D_FORMAT_VERSION_V2_10_SDK2) {
        mPivotOpacities = br.ReadFloat32Array();
    }
}

void Deformer::InterpolateOpacity(Moc* moc, PivotManager* pivotMgr, bool& ret) {
    if (mPivotOpacities.empty()) {
        SetInterpolatedOpacity(1.0f);
    } else {
        SetInterpolatedOpacity(
            UtInterpolate::InterpolateFloat(moc, pivotMgr, ret, mPivotOpacities));
    }
}

bool Deformer::NeedTransform() const {
    return mTargetId != nullptr && *mTargetId != Id::DST_BASE_ID();
}

}   // namespace V2
}   // namespace Live2D