#version 450
// V2 mask program — vertex stage（裁剪绘制：v_clipPos = clipMatrix * pos）
// 由 GLRenderer.cpp 内嵌 GLSL 1.20 sVertMask 移植
layout(location=0) in vec2 a_position;
layout(location=1) in vec2 a_texCoord;
layout(location=0) out vec2 v_texCoord;
layout(location=1) out vec4 v_clipPos;

layout(std140, set=0, binding=0) uniform UBO {
    mat4 u_mvpMatrix;
    mat4 u_clipMatrix;
    vec4 u_baseColor;
    vec4 u_screenColor;
    vec4 u_multiplyColor;
    vec4 u_channelFlag;
    int u_maskFlag;
} ubo;

void main() {
    vec4 pos = vec4(a_position, 0.0, 1.0);
    gl_Position = ubo.u_mvpMatrix * pos;
    v_clipPos = ubo.u_clipMatrix * pos;
    // mask pass 使用负高度 viewport，写入的 mask image 相对 GL 垂直翻转；
    // 采样坐标（clipMatrix 映射到 [0,1]）需同步翻转 v 分量（w=1，仿射矩阵）
    v_clipPos.y = 1.0 - v_clipPos.y;
    v_texCoord = a_texCoord;
    v_texCoord.y = 1.0 - v_texCoord.y;
}
