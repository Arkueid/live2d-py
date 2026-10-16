#include "ObjectFactory.hpp"
#include "Log.hpp"
#include "AffineEnt.hpp"
#include "Deformer.hpp"
#include "RotationDeformer.hpp"
#include "WarpDeformer.hpp"
#include "Drawable.hpp"
#include "ISerializable.hpp"
#include "Moc.hpp"
#include "Parameter.hpp"
#include "Part.hpp"
#include "PivotManager.hpp"

namespace Live2D {
namespace V2 {

using namespace Live2D::Common::Log;

std::unique_ptr<ISerializable> ObjectFactory::Create(int clsNo, Id::IdPool& idPool) {
    if (clsNo < 100) {
        switch (clsNo) {
            case 65:
                return std::make_unique<WarpDeformer>();
            case 66:
                return std::make_unique<PivotManager>();
            case 67:
                return std::make_unique<PivotManager::Pivot>();
            case 68:
                return std::make_unique<RotationDeformer>();
            case 69:
                return std::make_unique<AffineEnt>();
            case 70:
                return std::make_unique<Drawable>();
            default:
                break;
        }
    } else if (clsNo < 150) {
        switch (clsNo) {
            case 131:
                return std::make_unique<Parameter>();
            case 133:
                return std::make_unique<Part>();
            case 136:
                return std::make_unique<Moc>(idPool);
            case 142:
                // Avatar（旧换装特性）已移除；moc 中出现视为不支持的数据
                LOGE("Unsupported class ID (Avatar removed): %d", clsNo);
                break;
            default:
                LOGE("Unknown class ID: %d", clsNo);
                break;
        }
    }

    return nullptr;
}
}   // namespace V2
}   // namespace Live2D
