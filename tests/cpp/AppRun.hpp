#pragma once

#include <string>

/**
 * GLFW 窗口 + 后端初始化（GL / Vulkan） + 模型加载 + 主循环 + 清理。
 * 后端由调用方事先写入 Common::Backend（main 里完成）。
 */
int RunInteractive(const std::string& modelPath);
