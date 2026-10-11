#pragma once

/**
 * @brief ModelFactory.hpp
 * @note 统一模型创建入口：按模型 json 自动识别 v2/v3 并实例化对应实现。
 *
 * 头文件级工厂（header-only）：依赖 V2/V3 的 Model 定义，
 * 仅供同时链接 V2 + V3 的使用方包含（如 tests/cpp、Wrapper）。
 */

#include "IModel.hpp"
#include "ModelVersion.hpp"
#include "V2/Model.hpp"
#include "V3/Model.hpp"

namespace Live2D {

// 按模型 json 版本实例化对应实现（不加载；调用方再调 LoadModelJson，并负责释放）。
// 版本探测失败（非模型 json / moc 缺失等）返回 nullptr。
inline IModel* CreateModel(const char* modelJsonPath) {
    switch (DetectModelVersion(modelJsonPath)) {
        case 2:
            return new V2::Model();
        case 3:
            return new V3::Model();
        default:
            return nullptr;
    }
}

}   // namespace Live2D
