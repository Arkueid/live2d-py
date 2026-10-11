#pragma once

namespace Live2D {

// 探测模型 json 对应的 moc 版本：返回 2 / 3，失败返回 -1。
// v2: 顶层 "model" 字段 + moc 魔数 "moc"（3 字节）
// v3: "FileReferences.Moc" 字段 + moc3 魔数 "MOC3"（4 字节）
int DetectModelVersion(const char* modelJsonPath);

}   // namespace Live2D
