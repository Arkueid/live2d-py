#include "Id.hpp"


namespace Live2D {
namespace V2 {

Id::Id(const std::string& idStr) : mId(idStr) {}

Id::IdPool::IdPool() {
    mIds.insert_or_assign("DST_BASE", Id::DST_BASE_ID());
}

Id::IdPool::~IdPool() {
    mIds.erase("DST_BASE");
    for (auto& it : mIds) {
        delete it.second;
    }
    mIds.clear();
}

Id* Id::DST_BASE_ID() {
    static Id mDSTBaseID("DST_BASE");
    return &mDSTBaseID;
}

Id* Id::IdPool::Resolve(const std::string& idStr) {
    auto [it, inserted] = mIds.try_emplace(idStr, nullptr);
    if (inserted) {
        it->second = new Id(idStr);
    }
    return it->second;
}

}   // namespace V2
}   // namespace Live2D