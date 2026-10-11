#pragma once

#include <memory>

namespace Live2D {
namespace V2 {

class ISerializable;

class ObjectFactory {
public:
    static std::unique_ptr<ISerializable> create(int clsNo);
};

}   // namespace V2
}   // namespace Live2D