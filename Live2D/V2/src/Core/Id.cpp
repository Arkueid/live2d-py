#include "Id.hpp"

namespace Live2D {
namespace V2 {

std::unordered_map<std::string, std::unique_ptr<Id>> Id::sInstances;

const Id& Id::DST_BASE_ID() {
    return Id::GetID("DST_BASE");
}

const Id& Id::GetID(const std::string& idStr) {
    auto it = sInstances.find(idStr);
    if (it != sInstances.end()) {
        return *it->second;
    }
    auto id = std::make_unique<Id>(idStr);
    const Id& ref = *id;
    sInstances[idStr] = std::move(id);
    return ref;
}

void Id::ReleaseStored() {
    sInstances.clear();
}

}   // namespace V2
}   // namespace Live2D