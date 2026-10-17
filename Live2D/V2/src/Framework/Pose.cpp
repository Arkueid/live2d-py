#include "Pose.hpp"
#include "Id.hpp"
#include "Log.hpp"
#include "Moc.hpp"
#include "nlohmann/json.hpp"
#include <algorithm>
#include <string>

namespace Live2D {
namespace V2 {

using json = nlohmann::json;

Pose::Pose() = default;

void PartTarget::InitIndex(Moc* moc) {
    partIndex = moc->GetPartIndex(id);
    for (auto& l : link)
        l.InitIndex(moc);
}

static void normalizeGroup(Moc* moc, PosePartGroup& group, float deltaSec) {
    int visibleIdx = -1;
    float visibleOpacity = 1.0f;
    float clearSec = 0.5f;
    float phi = 0.5f;
    float maxBack = 0.15f;

    for (int i = 0; i < (int)group.parts.size(); i++) {
        if (moc->GetVisibleFlag(group.parts[i].id) != 0.0f) {
            if (visibleIdx >= 0)
                break;
            visibleIdx = i;
            int pIdx = group.parts[i].partIndex;
            if (pIdx >= 0) {
                visibleOpacity = moc->GetPartsOpacity(pIdx);
                visibleOpacity += deltaSec / clearSec;
                if (visibleOpacity > 1.0f)
                    visibleOpacity = 1.0f;
            }
        }
    }

    if (visibleIdx < 0) {
        visibleIdx = 0;
        visibleOpacity = 1.0f;
    }

    for (int i = 0; i < (int)group.parts.size(); i++) {
        int pIdx = group.parts[i].partIndex;
        if (pIdx < 0)
            continue;
        if (i == visibleIdx) {
            moc->SetPartsOpacity(pIdx, visibleOpacity);
        } else {
            float cur = moc->GetPartsOpacity(pIdx);
            float a1;
            if (visibleOpacity < phi)
                a1 = visibleOpacity * (phi - 1.0f) / phi + 1.0f;
            else
                a1 = (1.0f - visibleOpacity) * phi / (1.0f - phi);

            float backOp = (1.0f - a1) * (1.0f - visibleOpacity);
            if (backOp > maxBack)
                a1 = 1.0f - maxBack / (1.0f - visibleOpacity);
            cur = std::min(cur, a1);
            moc->SetPartsOpacity(pIdx, cur);
        }
    }
}

static void copyOpacityOtherParts(Moc* moc, PosePartGroup& group) {
    for (auto& p : group.parts) {
        if (p.partIndex < 0)
            continue;
        float op = moc->GetPartsOpacity(p.partIndex);
        for (auto& lp : p.link) {
            if (lp.partIndex < 0)
                continue;
            moc->SetPartsOpacity(lp.partIndex, op);
        }
    }
}

void Pose::InitParam(Moc* moc) {
    // Python: initParam runs once per model instance (l2d_pose.py:27-42),
    // not per frame — the per-frame opacity hard-set was wiping the fade.
    for (auto& g : mMGroups) {
        for (auto& p : g.parts) {
            p.InitIndex(moc);
            if (p.partIndex < 0)
                continue;
            bool v = (moc->GetVisibleFlag(p.id) != 0.0f);
            moc->SetPartsOpacity(p.partIndex, v ? 1.0f : 0.0f);
        }
    }
}

void Pose::UpdateParam(Moc* moc, float dtSec) {
    for (auto& g : mMGroups) {
        normalizeGroup(moc, g, dtSec);
        copyOpacityOtherParts(moc, g);
    }
}

Pose* Pose::Load(const std::vector<uint8_t>& data, Id::IdPool& idPool) {
    auto* pose = new Pose();
    if (data.empty())
        return pose;
    // Python: pm.jsonParseFromBytes == json.loads (l2d_pose.py:106-132)
    json root = json::parse(std::string((const char*)data.data(), data.size()));
    auto visible = root.find("parts_visible");
    if (visible == root.end() || !visible->is_array())
        return pose;

    for (const auto& poseInfo : *visible) {
        auto groups = poseInfo.find("group");
        if (groups == poseInfo.end() || !groups->is_array())
            continue;
        PosePartGroup group;
        for (const auto& partsInfo : *groups) {
            if (!partsInfo.is_object())
                continue;
            PartTarget parts;
            parts.id = idPool.Resolve(partsInfo.value("id", std::string()));
            // Python: get("link") — missing or null => no links
            auto link = partsInfo.find("link");
            if (link != partsInfo.end() && link->is_array()) {
                for (const auto& linkId : *link) {
                    if (!linkId.is_string())
                        continue;
                    PartTarget linkParts;
                    linkParts.id = idPool.Resolve(linkId.get<std::string>());
                    parts.link.push_back(std::move(linkParts));
                }
            }
            group.parts.push_back(std::move(parts));
        }
        pose->mMGroups.push_back(std::move(group));
    }
    return pose;
}

}   // namespace V2
}   // namespace Live2D