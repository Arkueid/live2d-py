#version 120
// V2 OpenGL normal program — vertex stage
// 由 GLRenderer.cpp 内嵌 GLSL 1.20 (sVertNormal) 拆分而来，内容保持一致。
attribute vec2 a_position;
attribute vec2 a_texCoord;
varying vec2 v_texCoord;
varying vec4 v_clipPos;
uniform mat4 u_mvpMatrix;
void main(){
    gl_Position = u_mvpMatrix * vec4(a_position, 0.0, 1.0);
    v_clipPos = gl_Position;
    v_texCoord = a_texCoord;
    v_texCoord.y = 1.0 - v_texCoord.y;
}
