#include "AffineEnt.hpp"
#include "Core/BinaryReader.hpp"
#include "Core/DEF.hpp"

namespace Live2D {
namespace V2 {

void AffineEnt::Read(BinaryReader& br) {
    mOriginX = br.ReadFloat32();
    mOriginY = br.ReadFloat32();
    mScaleX = br.ReadFloat32();
    mScaleY = br.ReadFloat32();
    mRotationDeg = br.ReadFloat32();
    if (br.GetFormatVersion() >= LIVE2D_FORMAT_VERSION_V2_10_SDK2) {
        mReflectX = br.ReadBoolean();
        mReflectY = br.ReadBoolean();
    }
}

}   // namespace V2
}   // namespace Live2D