#version 450
// V2 normal program — fragment stage（兼做 mask 渲染 pass：u_maskFlag 分支）
// 由 GLRenderer.cpp 内嵌 GLSL 1.20 sFragNormal 移植
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

void main() {
    vec4 smpColor;
    if (ubo.u_maskFlag != 0) {
        float isInside =
            step(ubo.u_baseColor.x, v_clipPos.x / v_clipPos.w)
          * step(ubo.u_baseColor.y, v_clipPos.y / v_clipPos.w)
          * step(v_clipPos.x / v_clipPos.w, ubo.u_baseColor.z)
          * step(v_clipPos.y / v_clipPos.w, ubo.u_baseColor.w);
        smpColor = ubo.u_channelFlag * texture(s_texture0, v_texCoord).a * isInside;
    } else {
        smpColor = texture(s_texture0, v_texCoord);
        smpColor.rgb = smpColor.rgb * smpColor.a;
        smpColor.rgb = smpColor.rgb * ubo.u_multiplyColor.rgb;
        smpColor.rgb = smpColor.rgb + ubo.u_screenColor.rgb - (smpColor.rgb * ubo.u_screenColor.rgb);
        smpColor = smpColor * ubo.u_baseColor;
    }
    outColor = smpColor;
}
