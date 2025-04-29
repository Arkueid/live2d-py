#version 460
#extension GL_GOOGLE_include_directive : enable
#include "common.glsl"

layout(location = 0) in vec2 v_texCoord;
layout(location = 0) out vec4 outColor;

void main()
{
    vec4 texColor = texture(s_texture0 , v_texCoord);
    texColor.rgb = texColor.rgb * ubo.u_multiplyColor.rgb;
    texColor.rgb = (texColor.rgb + ubo.u_screenColor.rgb * texColor.a) - (texColor.rgb * ubo.u_screenColor.rgb);
    outColor = texColor * ubo.u_baseColor;
}
