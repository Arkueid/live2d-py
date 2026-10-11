#pragma once
#include <string>
namespace Live2D {
namespace V2 {
struct ExpressionParam {
    std::string mId;
    float mValue = 0;
    float mDefValue = 0;
    int mBlendType = 1;
};
}   // namespace V2
}   // namespace Live2D