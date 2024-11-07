#version 460
#extension GL_GOOGLE_include_directive : enable
#include "common.glsl"

layout(location = 0) in vec2 v_texCoord;
layout(location = 1) in vec4 v_clipPos;

layout(location = 0) out vec4 outColor;

void main()
{
    vec4 texColor = texture(s_texture0, v_texCoord);
    texColor.rgb = texColor.rgb * ubo.u_multiplyColor.rgb;
    texColor.rgb = (texColor.rgb + ubo.u_screenColor.rgb * texColor.a) - (texColor.rgb * ubo.u_screenColor.rgb);
    vec4 col_formask = texColor * ubo.u_baseColor;
    vec4 clipMask = (1.0 - texture(s_texture1, v_clipPos.xy / v_clipPos.w)) * ubo.u_channelFlag;
    float maskVal = clipMask.r + clipMask.g + clipMask.b + clipMask.a;
    col_formask = col_formask * maskVal;
    outColor = col_formask;
}
