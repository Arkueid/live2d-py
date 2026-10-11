#version 450
// V2 normal program — vertex stage
// 由 GLRenderer.cpp 内嵌 GLSL 1.20 sVertNormal 移植（UV 翻转逻辑保留）
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
    gl_Position = ubo.u_mvpMatrix * vec4(a_position, 0.0, 1.0);
    v_clipPos = gl_Position;
    v_texCoord = a_texCoord;
    v_texCoord.y = 1.0 - v_texCoord.y;
}
