#pragma once

#include <string>

/** 单个模型的 API 回归（无窗口）。返回 0 表示全部检查通过。 */
int RunSmokeModel(const std::string& modelPath);

/** 对 ModelCatalog 中的全部模型逐个执行 RunSmokeModel，返回失败数非零时 1。 */
int RunSmokeAll();
