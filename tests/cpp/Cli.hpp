#pragma once

#include <string>

#include "GraphicsBackend.hpp"

/**
 * 命令行选项（解析后结果）。
 */
struct AppOptions {
    Live2D::Common::Backend::Type backend = Live2D::Common::Backend::BACKEND_NONE;
    bool smoke = false;      // --smoke：无窗口的 API 回归测试
    bool smokeAll = false;   // --smoke all：对 ModelCatalog 中全部模型逐个回归
    std::string modelPath;   // 解析后的绝对路径（smokeAll 时为空）
};

/**
 * 解析命令行。返回 false 表示参数非法（已向 stderr 打印用法）。
 */
bool ParseOptions(int argc, char* argv[], AppOptions& out);

void PrintUsage(const char* exePath);
