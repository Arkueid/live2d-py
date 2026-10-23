#version 120
// V2 OpenGL normal program — fragment stage (u_maskFlag 同时承担 mask 渲染通路)
// 由 GLRenderer.cpp 内嵌 GLSL 1.20 (sFragNormal) 拆分而来，内容保持一致。
#ifdef GL_ES
precision mediump float;
#endif
varying vec2 v_texCoord;
varying vec4 v_clipPos;
uniform sampler2D s_texture0;
uniform vec4 u_channelFlag;
uniform vec4 u_baseColor;
uniform bool u_maskFlag;
uniform vec4 u_screenColor;
uniform vec4 u_multiplyColor;
void main(){
    vec4 smpColor;
    if(u_maskFlag){
        float isInside =
            step(u_baseColor.x, v_clipPos.x/v_clipPos.w)
          * step(u_baseColor.y, v_clipPos.y/v_clipPos.w)
          * step(v_clipPos.x/v_clipPos.w, u_baseColor.z)
          * step(v_clipPos.y/v_clipPos.w, u_baseColor.w);
        smpColor = u_channelFlag * texture2D(s_texture0, v_texCoord).a * isInside;
    }else{
        smpColor = texture2D(s_texture0, v_texCoord);
        smpColor.rgb = smpColor.rgb * smpColor.a;
        smpColor.rgb = smpColor.rgb * u_multiplyColor.rgb;
        smpColor.rgb = smpColor.rgb + u_screenColor.rgb - (smpColor.rgb * u_screenColor.rgb);
        smpColor = smpColor * u_baseColor;
    }
    gl_FragColor = smpColor;
}
