layout(binding = 0) uniform UBO
{
    mat4 u_matrix;
    mat4 u_clipMatrix;
    vec4 u_baseColor;
    vec4 u_multiplyColor;
    vec4 u_screenColor;
    vec4 u_channelFlag;
}ubo;

layout(binding = 1) uniform sampler2D s_texture0;
layout(binding = 2) uniform sampler2D s_texture1;
