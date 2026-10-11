#pragma once

namespace Live2D {

/**
 * @brief 初始化 Cubism 框架（幂等，重复调用直接返回）。
 *
 * 封装框架启动所需的全部样板：设置 shader 目录、配置 Option
 * （日志/文件加载回调）、StartUp、Initialize。
 * Python 与 C++ 调用方统一通过此入口完成初始化。
 *
 * @param shaderDir  shader 资源目录（FrameworkShaders 所在目录的父路径）
 * @param logLevel   Cubism 框架日志级别，对应 Csm::CubismFramework::Option::LogLevel
 *                   （0=Verbose, 1=Debug, 2=Info, 3=Warning, 4=Error, 5=Off；默认 Warning）
 */
void Init(const char* shaderDir, int logLevel = 3);

/**
 * @brief 释放 Cubism 框架资源（与 Init 配对）。
 */
void Dispose();

}   // namespace Live2D
