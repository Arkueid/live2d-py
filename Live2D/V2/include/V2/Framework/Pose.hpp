#pragma once
#include <cstdint>
#include <string>
#include <vector>
namespace Live2D {
namespace V2 {
class Moc;
struct PartData {
    int partsIndex = -1;
    int paramIndex = -1;
    std::string id;
    std::vector<PartData> link;
    void InitIndex(Moc* moc);
};
struct PosePartGroup {
    std::vector<PartData> parts;
};
class Pose {
public:
    Pose();
    // 时间由外部传入（时钟只由 Model 管理）: dtSec 为本次时间步长（秒）
    void UpdateParam(Moc* moc, float dtSec);
    void InitParam(Moc* moc);
    static Pose* Load(const std::vector<uint8_t>& data);

public:
    std::vector<PosePartGroup> mMGroups;

private:

};
}   // namespace V2
}   // namespace Live2D