#pragma once

#include <memory>
#include "Id.hpp"

namespace Live2D {
namespace V2 {

class ISerializable;

class ObjectFactory {
public:
    static std::unique_ptr<ISerializable> Create(int clsNo, Id::IdPool& idPool);
};

}   // namespace V2
}   // namespace Live2D