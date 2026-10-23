#version 120
// V2 OpenGL mask program — fragment stage
// 由 GLRenderer.cpp 内嵌 GLSL 1.20 (sFragMask) 拆分而来，内容保持一致。
#ifdef GL_ES
precision mediump float;
#endif
varying vec2 v_texCoord;
varying vec4 v_clipPos;
uniform sampler2D s_texture0;
uniform sampler2D s_texture1;
uniform vec4 u_channelFlag;
uniform vec4 u_baseColor;
uniform vec4 u_screenColor;
uniform vec4 u_multiplyColor;
void main(){
    vec4 col_formask = texture2D(s_texture0, v_texCoord);
    col_formask.rgb = col_formask.rgb * col_formask.a;
    col_formask.rgb = col_formask.rgb * u_multiplyColor.rgb;
    col_formask.rgb = col_formask.rgb + u_screenColor.rgb - (col_formask.rgb * u_screenColor.rgb);
    col_formask = col_formask * u_baseColor;
    vec4 clipMask = texture2D(s_texture1, v_clipPos.xy / v_clipPos.w) * u_channelFlag;
    float maskVal = clipMask.r + clipMask.g + clipMask.b + clipMask.a;
    col_formask = col_formask * maskVal;
    gl_FragColor = col_formask;
}
