#pragma once
#include "Id.hpp"
#include <vector>


namespace Live2D {
namespace V2 {

struct Motion {
    Id* mParamId = nullptr;
    Id* mSecondaryId = nullptr;
    Id* mVisibleSecondaryId = nullptr;
    std::vector<float> mValues;
};

}   // namespace V2
}   // namespace Live2D