#version 450
// V2 mask program — fragment stage（采样 256x256 裁剪遮罩 s_texture1）
// 由 GLRenderer.cpp 内嵌 GLSL 1.20 sFragMask 移植
layout(location=0) in vec2 v_texCoord;
layout(location=1) in vec4 v_clipPos;
layout(location=0) out vec4 outColor;

layout(std140, set=0, binding=0) uniform UBO {
    mat4 u_mvpMatrix;
    mat4 u_clipMatrix;
    vec4 u_baseColor;
    vec4 u_screenColor;
    vec4 u_multiplyColor;
    vec4 u_channelFlag;
    int u_maskFlag;
} ubo;

layout(set=0, binding=1) uniform sampler2D s_texture0;
layout(set=0, binding=2) uniform sampler2D s_texture1;

void main() {
    vec4 col_formask = texture(s_texture0, v_texCoord);
    col_formask.rgb = col_formask.rgb * col_formask.a;
    col_formask.rgb = col_formask.rgb * ubo.u_multiplyColor.rgb;
    col_formask.rgb = col_formask.rgb + ubo.u_screenColor.rgb - (col_formask.rgb * ubo.u_screenColor.rgb);
    col_formask = col_formask * ubo.u_baseColor;

    vec4 clipMask = texture(s_texture1, v_clipPos.xy / v_clipPos.w) * ubo.u_channelFlag;
    float maskVal = clipMask.r + clipMask.g + clipMask.b + clipMask.a;

    outColor = col_formask * maskVal;
}
