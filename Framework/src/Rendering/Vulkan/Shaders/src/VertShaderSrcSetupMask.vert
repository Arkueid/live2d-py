#version 460
#extension GL_GOOGLE_include_directive : enable
#include "common.glsl"

layout(location = 0) in vec4 a_position;
layout(location = 1) in vec2 a_texCoord;

layout(location = 0) out vec2 v_texCoord;
layout(location = 1) out vec4 v_myPos;

void main()
{
    vec4 pos = ubo.u_clipMatrix * a_position;
    pos.y = -pos.y;
    gl_Position = pos;
    v_myPos = ubo.u_clipMatrix * a_position;
    v_texCoord = a_texCoord;
    v_texCoord.y = 1.0 - v_texCoord.y;
}
