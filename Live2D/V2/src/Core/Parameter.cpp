#include "Parameter.hpp"
#include "BinaryReader.hpp"

namespace Live2D {
namespace V2 {

void Parameter::Read(BinaryReader& br) {
    mMinValue = br.ReadFloat32();
    mMaxValue = br.ReadFloat32();
    mDefaultValue = br.ReadFloat32();
    mParamId = br.ReadObject<const Id*>();
}

}   // namespace V2
}   // namespace Live2D
