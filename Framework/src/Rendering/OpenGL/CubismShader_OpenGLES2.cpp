/**
 * Copyright(c) Live2D Inc. All rights reserved.
 *
 * Use of this source code is governed by the Live2D Open Software license
 * that can be found at https://www.live2d.com/eula/live2d-open-software-license-agreement_en.html.
 */

#include "CubismShader_OpenGLES2.hpp"
#include <float.h>
#include "Type/csmRectF.hpp"

#ifdef CSM_TARGET_WIN_GL
#include <Windows.h>
#endif

#define CSM_FRAGMENT_SHADER_FP_PRECISION_HIGH "highp"
#define CSM_FRAGMENT_SHADER_FP_PRECISION_MID "mediump"
#define CSM_FRAGMENT_SHADER_FP_PRECISION_LOW "lowp"

#define CSM_FRAGMENT_SHADER_FP_PRECISION CSM_FRAGMENT_SHADER_FP_PRECISION_HIGH

//------------ LIVE2D NAMESPACE ------------
namespace Live2D { namespace Cubism { namespace Framework { namespace Rendering {

/*********************************************************************************************************************
*                                       CubismShader_OpenGLES2
********************************************************************************************************************/
namespace {
    const csmInt32 ShaderCount = 19; ///< シェーダの数 = マスク生成用 + (通常 + 加算 + 乗算) * (マスク無 + マスク有 + マスク有反転 + マスク無の乗算済アルファ対応版 + マスク有の乗算済アルファ対応版 + マスク有反転の乗算済アルファ対応版)
    CubismShader_OpenGLES2* s_instance;
}

enum ShaderNames
{
    // SetupMask
    ShaderNames_SetupMask,

    //Normal
    ShaderNames_Normal,
    ShaderNames_NormalMasked,
    ShaderNames_NormalMaskedInverted,
    ShaderNames_NormalPremultipliedAlpha,
    ShaderNames_NormalMaskedPremultipliedAlpha,
    ShaderNames_NormalMaskedInvertedPremultipliedAlpha,

    //Add
    ShaderNames_Add,
    ShaderNames_AddMasked,
    ShaderNames_AddMaskedInverted,
    ShaderNames_AddPremultipliedAlpha,
    ShaderNames_AddMaskedPremultipliedAlpha,
    ShaderNames_AddMaskedPremultipliedAlphaInverted,

    //Mult
    ShaderNames_Mult,
    ShaderNames_MultMasked,
    ShaderNames_MultMaskedInverted,
    ShaderNames_MultPremultipliedAlpha,
    ShaderNames_MultMaskedPremultipliedAlpha,
    ShaderNames_MultMaskedPremultipliedAlphaInverted,
};

// SetupMask
static const csmChar* VertShaderSrcSetupMask =
#if defined(CSM_TARGET_IPHONE_ES2) || defined(CSM_TARGET_ANDROID_ES2)
        "#version 100\n"
#else
        "#version 120\n"
#endif
        "attribute vec4 a_position;"
        "attribute vec2 a_texCoord;"
        "varying vec2 v_texCoord;"
        "varying vec4 v_myPos;"
        "uniform mat4 u_clipMatrix;"
        "void main()"
        "{"
        "gl_Position = u_clipMatrix * a_position;"
        "v_myPos = u_clipMatrix * a_position;"
        "v_texCoord = a_texCoord;"
        "v_texCoord.y = 1.0 - v_texCoord.y;"
        "}";
static const csmChar* FragShaderSrcSetupMask =
#if defined(CSM_TARGET_IPHONE_ES2) || defined(CSM_TARGET_ANDROID_ES2)
        "#version 100\n"
        "precision " CSM_FRAGMENT_SHADER_FP_PRECISION " float;"
#else
        "#version 120\n"
#endif
        "varying vec2 v_texCoord;"
        "varying vec4 v_myPos;"
        "uniform sampler2D s_texture0;"
        "uniform vec4 u_channelFlag;"
        "uniform vec4 u_baseColor;"
        "void main()"
        "{"
        "float isInside = "
        "  step(u_baseColor.x, v_myPos.x/v_myPos.w)"
        "* step(u_baseColor.y, v_myPos.y/v_myPos.w)"
        "* step(v_myPos.x/v_myPos.w, u_baseColor.z)"
        "* step(v_myPos.y/v_myPos.w, u_baseColor.w);"

        "gl_FragColor = u_channelFlag * texture2D(s_texture0 , v_texCoord).a * isInside;"
        "}";
#if defined(CSM_TARGET_ANDROID_ES2)
static const csmChar* FragShaderSrcSetupMaskTegra =
        "#version 100\n"
        "#extension GL_NV_shader_framebuffer_fetch : enable\n"
        "precision " CSM_FRAGMENT_SHADER_FP_PRECISION " float;"
        "varying vec2 v_texCoord;"
        "varying vec4 v_myPos;"
        "uniform sampler2D s_texture0;"
        "uniform vec4 u_channelFlag;"
        "uniform vec4 u_baseColor;"
        "void main()"
        "{"
        "float isInside = "
        "  step(u_baseColor.x, v_myPos.x/v_myPos.w)"
        "* step(u_baseColor.y, v_myPos.y/v_myPos.w)"
        "* step(v_myPos.x/v_myPos.w, u_baseColor.z)"
        "* step(v_myPos.y/v_myPos.w, u_baseColor.w);"

        "gl_FragColor = u_channelFlag * texture2D(s_texture0 , v_texCoord).a * isInside;"
        "}";
#endif

//----- バーテックスシェーダプログラム -----
// Normal & Add & Mult 共通
static const csmChar* VertShaderSrc =
#if defined(CSM_TARGET_IPHONE_ES2) || defined(CSM_TARGET_ANDROID_ES2)
        "#version 100\n"
#else
        "#version 120\n"
#endif
        "attribute vec4 a_position;" //v.vertex
        "attribute vec2 a_texCoord;" //v.texcoord
        "varying vec2 v_texCoord;" //v2f.texcoord
        "uniform mat4 u_matrix;"
        "void main()"
        "{"
        "gl_Position = u_matrix * a_position;"
        "v_texCoord = a_texCoord;"
        "v_texCoord.y = 1.0 - v_texCoord.y;"
        "}";

// Normal & Add & Mult 共通（クリッピングされたものの描画用）
static const csmChar* VertShaderSrcMasked =
#if defined(CSM_TARGET_IPHONE_ES2) || defined(CSM_TARGET_ANDROID_ES2)
        "#version 100\n"
#else
        "#version 120\n"
#endif
        "attribute vec4 a_position;"
        "attribute vec2 a_texCoord;"
        "varying vec2 v_texCoord;"
        "varying vec4 v_clipPos;"
        "uniform mat4 u_matrix;"
        "uniform mat4 u_clipMatrix;"
        "void main()"
        "{"
        "gl_Position = u_matrix * a_position;"
        "v_clipPos = u_clipMatrix * a_position;"
        "v_texCoord = a_texCoord;"
        "v_texCoord.y = 1.0 - v_texCoord.y;"
        "}";

//----- フラグメントシェーダプログラム -----
// Normal & Add & Mult 共通
static const csmChar* FragShaderSrc =
#if defined(CSM_TARGET_IPHONE_ES2) || defined(CSM_TARGET_ANDROID_ES2)
        "#version 100\n"
        "precision " CSM_FRAGMENT_SHADER_FP_PRECISION " float;"
#else
        "#version 120\n"
#endif
        "varying vec2 v_texCoord;" //v2f.texcoord
        "uniform sampler2D s_texture0;" //_MainTex
        "uniform vec4 u_baseColor;" //v2f.color
        "uniform vec4 u_multiplyColor;"
        "uniform vec4 u_screenColor;"
        "void main()"
        "{"
        "vec4 texColor = texture2D(s_texture0 , v_texCoord);"
        "texColor.rgb = texColor.rgb * u_multiplyColor.rgb;"
        "texColor.rgb = texColor.rgb + u_screenColor.rgb - (texColor.rgb * u_screenColor.rgb);"
        "vec4 color = texColor * u_baseColor;"
        "gl_FragColor = vec4(color.rgb * color.a,  color.a);"
        "}";
#if defined(CSM_TARGET_ANDROID_ES2)
static const csmChar* FragShaderSrcTegra =
        "#version 100\n"
        "#extension GL_NV_shader_framebuffer_fetch : enable\n"
        "precision " CSM_FRAGMENT_SHADER_FP_PRECISION " float;"
        "varying vec2 v_texCoord;" //v2f.texcoord
        "uniform sampler2D s_texture0;" //_MainTex
        "uniform vec4 u_baseColor;" //v2f.color
        "uniform vec4 u_multiplyColor;"
        "uniform vec4 u_screenColor;"
        "void main()"
        "{"
        "vec4 texColor = texture2D(s_texture0 , v_texCoord);"
        "texColor.rgb = texColor.rgb * u_multiplyColor.rgb;"
        "texColor.rgb = texColor.rgb + u_screenColor.rgb - (texColor.rgb * u_screenColor.rgb);"
        "vec4 color = texColor * u_baseColor;"
        "gl_FragColor = vec4(color.rgb * color.a,  color.a);"
        "}";
#endif

// Normal & Add & Mult 共通 （PremultipliedAlpha）
static const csmChar* FragShaderSrcPremultipliedAlpha =
#if defined(CSM_TARGET_IPHONE_ES2) || defined(CSM_TARGET_ANDROID_ES2)
        "#version 100\n"
        "precision " CSM_FRAGMENT_SHADER_FP_PRECISION " float;"
#else
        "#version 120\n"
#endif
        "varying vec2 v_texCoord;" //v2f.texcoord
        "uniform sampler2D s_texture0;" //_MainTex
        "uniform vec4 u_baseColor;" //v2f.color
        "uniform vec4 u_multiplyColor;"
        "uniform vec4 u_screenColor;"
        "void main()"
        "{"
        "vec4 texColor = texture2D(s_texture0 , v_texCoord);"
        "texColor.rgb = texColor.rgb * u_multiplyColor.rgb;"
        "texColor.rgb = (texColor.rgb + u_screenColor.rgb * texColor.a) - (texColor.rgb * u_screenColor.rgb);"
        "gl_FragColor = texColor * u_baseColor;"
        "}";
#if defined(CSM_TARGET_ANDROID_ES2)
static const csmChar* FragShaderSrcPremultipliedAlphaTegra =
        "#version 100\n"
        "#extension GL_NV_shader_framebuffer_fetch : enable\n"
        "precision " CSM_FRAGMENT_SHADER_FP_PRECISION " float;"
        "varying vec2 v_texCoord;" //v2f.texcoord
        "uniform sampler2D s_texture0;" //_MainTex
        "uniform vec4 u_baseColor;" //v2f.color
        "uniform vec4 u_multiplyColor;"
        "uniform vec4 u_screenColor;"
        "void main()"
        "{"
        "vec4 texColor = texture2D(s_texture0 , v_texCoord);"
        "texColor.rgb = texColor.rgb * u_multiplyColor.rgb;"
        "texColor.rgb = (texColor.rgb + u_screenColor.rgb * texColor.a) - (texColor.rgb * u_screenColor.rgb);"
        "gl_FragColor = texColor * u_baseColor;"
        "}";
#endif

// Normal & Add & Mult 共通（クリッピングされたものの描画用）
static const csmChar* FragShaderSrcMask =
#if defined(CSM_TARGET_IPHONE_ES2) || defined(CSM_TARGET_ANDROID_ES2)
        "#version 100\n"
        "precision " CSM_FRAGMENT_SHADER_FP_PRECISION " float;"
#else
        "#version 120\n"
#endif
        "varying vec2 v_texCoord;"
        "varying vec4 v_clipPos;"
        "uniform sampler2D s_texture0;"
        "uniform sampler2D s_texture1;"
        "uniform vec4 u_channelFlag;"
        "uniform vec4 u_baseColor;"
        "uniform vec4 u_multiplyColor;"
        "uniform vec4 u_screenColor;"
        "void main()"
        "{"
        "vec4 texColor = texture2D(s_texture0 , v_texCoord);"
        "texColor.rgb = texColor.rgb * u_multiplyColor.rgb;"
        "texColor.rgb = texColor.rgb + u_screenColor.rgb - (texColor.rgb * u_screenColor.rgb);"
        "vec4 col_formask = texColor * u_baseColor;"
        "col_formask.rgb = col_formask.rgb  * col_formask.a ;"
        "vec4 clipMask = (1.0 - texture2D(s_texture1, v_clipPos.xy / v_clipPos.w)) * u_channelFlag;"
        "float maskVal = clipMask.r + clipMask.g + clipMask.b + clipMask.a;"
        "col_formask = col_formask * maskVal;"
        "gl_FragColor = col_formask;"
        "}";
#if defined(CSM_TARGET_ANDROID_ES2)
static const csmChar* FragShaderSrcMaskTegra =
        "#version 100\n"
        "#extension GL_NV_shader_framebuffer_fetch : enable\n"
        "precision " CSM_FRAGMENT_SHADER_FP_PRECISION " float;"
        "varying vec2 v_texCoord;"
        "varying vec4 v_clipPos;"
        "uniform sampler2D s_texture0;"
        "uniform sampler2D s_texture1;"
        "uniform vec4 u_channelFlag;"
        "uniform vec4 u_baseColor;"
        "uniform vec4 u_multiplyColor;"
        "uniform vec4 u_screenColor;"
        "void main()"
        "{"
        "vec4 texColor = texture2D(s_texture0 , v_texCoord);"
        "texColor.rgb = texColor.rgb * u_multiplyColor.rgb;"
        "texColor.rgb = texColor.rgb + u_screenColor.rgb - (texColor.rgb * u_screenColor.rgb);"
        "vec4 col_formask = texColor * u_baseColor;"
        "col_formask.rgb = col_formask.rgb  * col_formask.a ;"
        "vec4 clipMask = (1.0 - texture2D(s_texture1, v_clipPos.xy / v_clipPos.w)) * u_channelFlag;"
        "float maskVal = clipMask.r + clipMask.g + clipMask.b + clipMask.a;"
        "col_formask = col_formask * maskVal;"
        "gl_FragColor = col_formask;"
        "}";
#endif

// Normal & Add & Mult 共通（クリッピングされて反転使用の描画用）
static const csmChar* FragShaderSrcMaskInverted =
#if defined(CSM_TARGET_IPHONE_ES2) || defined(CSM_TARGET_ANDROID_ES2)
        "#version 100\n"
        "precision " CSM_FRAGMENT_SHADER_FP_PRECISION " float;"
#else
        "#version 120\n"
#endif
        "varying vec2 v_texCoord;"
        "varying vec4 v_clipPos;"
        "uniform sampler2D s_texture0;"
        "uniform sampler2D s_texture1;"
        "uniform vec4 u_channelFlag;"
        "uniform vec4 u_baseColor;"
        "uniform vec4 u_multiplyColor;"
        "uniform vec4 u_screenColor;"
        "void main()"
        "{"
        "vec4 texColor = texture2D(s_texture0 , v_texCoord);"
        "texColor.rgb = texColor.rgb * u_multiplyColor.rgb;"
        "texColor.rgb = texColor.rgb + u_screenColor.rgb - (texColor.rgb * u_screenColor.rgb);"
        "vec4 col_formask = texColor * u_baseColor;"
        "col_formask.rgb = col_formask.rgb  * col_formask.a ;"
        "vec4 clipMask = (1.0 - texture2D(s_texture1, v_clipPos.xy / v_clipPos.w)) * u_channelFlag;"
        "float maskVal = clipMask.r + clipMask.g + clipMask.b + clipMask.a;"
        "col_formask = col_formask * (1.0 - maskVal);"
        "gl_FragColor = col_formask;"
        "}";
#if defined(CSM_TARGET_ANDROID_ES2)
static const csmChar* FragShaderSrcMaskInvertedTegra =
        "#version 100\n"
        "#extension GL_NV_shader_framebuffer_fetch : enable\n"
        "precision " CSM_FRAGMENT_SHADER_FP_PRECISION " float;"
        "varying vec2 v_texCoord;"
        "varying vec4 v_clipPos;"
        "uniform sampler2D s_texture0;"
        "uniform sampler2D s_texture1;"
        "uniform vec4 u_channelFlag;"
        "uniform vec4 u_baseColor;"
        "uniform vec4 u_multiplyColor;"
        "uniform vec4 u_screenColor;"
        "void main()"
        "{"
        "vec4 texColor = texture2D(s_texture0 , v_texCoord);"
        "texColor.rgb = texColor.rgb * u_multiplyColor.rgb;"
        "texColor.rgb = texColor.rgb + u_screenColor.rgb - (texColor.rgb * u_screenColor.rgb);"
        "vec4 col_formask = texColor * u_baseColor;"
        "col_formask.rgb = col_formask.rgb  * col_formask.a ;"
        "vec4 clipMask = (1.0 - texture2D(s_texture1, v_clipPos.xy / v_clipPos.w)) * u_channelFlag;"
        "float maskVal = clipMask.r + clipMask.g + clipMask.b + clipMask.a;"
        "col_formask = col_formask * (1.0 - maskVal);"
        "gl_FragColor = col_formask;"
        "}";
#endif

// Normal & Add & Mult 共通（クリッピングされたものの描画用、PremultipliedAlphaの場合）
static const csmChar* FragShaderSrcMaskPremultipliedAlpha =
#if defined(CSM_TARGET_IPHONE_ES2) || defined(CSM_TARGET_ANDROID_ES2)
        "#version 100\n"
        "precision " CSM_FRAGMENT_SHADER_FP_PRECISION " float;"
#else
        "#version 120\n"
#endif
        "varying vec2 v_texCoord;"
        "varying vec4 v_clipPos;"
        "uniform sampler2D s_texture0;"
        "uniform sampler2D s_texture1;"
        "uniform vec4 u_channelFlag;"
        "uniform vec4 u_baseColor;"
        "uniform vec4 u_multiplyColor;"
        "uniform vec4 u_screenColor;"
        "void main()"
        "{"
        "vec4 texColor = texture2D(s_texture0 , v_texCoord);"
        "texColor.rgb = texColor.rgb * u_multiplyColor.rgb;"
        "texColor.rgb = (texColor.rgb + u_screenColor.rgb * texColor.a) - (texColor.rgb * u_screenColor.rgb);"
        "vec4 col_formask = texColor * u_baseColor;"
        "vec4 clipMask = (1.0 - texture2D(s_texture1, v_clipPos.xy / v_clipPos.w)) * u_channelFlag;"
        "float maskVal = clipMask.r + clipMask.g + clipMask.b + clipMask.a;"
        "col_formask = col_formask * maskVal;"
        "gl_FragColor = col_formask;"
        "}";
#if defined(CSM_TARGET_ANDROID_ES2)
static const csmChar* FragShaderSrcMaskPremultipliedAlphaTegra =
        "#version 100\n"
        "#extension GL_NV_shader_framebuffer_fetch : enable\n"
        "precision " CSM_FRAGMENT_SHADER_FP_PRECISION " float;"
        "varying vec2 v_texCoord;"
        "varying vec4 v_clipPos;"
        "uniform sampler2D s_texture0;"
        "uniform sampler2D s_texture1;"
        "uniform vec4 u_channelFlag;"
        "uniform vec4 u_baseColor;"
        "uniform vec4 u_multiplyColor;"
        "uniform vec4 u_screenColor;"
        "void main()"
        "{"
        "vec4 texColor = texture2D(s_texture0 , v_texCoord);"
        "texColor.rgb = texColor.rgb * u_multiplyColor.rgb;"
        "texColor.rgb = (texColor.rgb + u_screenColor.rgb * texColor.a) - (texColor.rgb * u_screenColor.rgb);"
        "vec4 col_formask = texColor * u_baseColor;"
        "vec4 clipMask = (1.0 - texture2D(s_texture1, v_clipPos.xy / v_clipPos.w)) * u_channelFlag;"
        "float maskVal = clipMask.r + clipMask.g + clipMask.b + clipMask.a;"
        "col_formask = col_formask * maskVal;"
        "gl_FragColor = col_formask;"
        "}";
#endif

// Normal & Add & Mult 共通（クリッピングされて反転使用の描画用、PremultipliedAlphaの場合）
static const csmChar* FragShaderSrcMaskInvertedPremultipliedAlpha =
#if defined(CSM_TARGET_IPHONE_ES2) || defined(CSM_TARGET_ANDROID_ES2)
        "#version 100\n"
        "precision " CSM_FRAGMENT_SHADER_FP_PRECISION " float;"
#else
        "#version 120\n"
#endif
        "varying vec2 v_texCoord;"
        "varying vec4 v_clipPos;"
        "uniform sampler2D s_texture0;"
        "uniform sampler2D s_texture1;"
        "uniform vec4 u_channelFlag;"
        "uniform vec4 u_baseColor;"
        "uniform vec4 u_multiplyColor;"
        "uniform vec4 u_screenColor;"
        "void main()"
        "{"
        "vec4 texColor = texture2D(s_texture0 , v_texCoord);"
        "texColor.rgb = texColor.rgb * u_multiplyColor.rgb;"
        "texColor.rgb = (texColor.rgb + u_screenColor.rgb * texColor.a) - (texColor.rgb * u_screenColor.rgb);"
        "vec4 col_formask = texColor * u_baseColor;"
        "vec4 clipMask = (1.0 - texture2D(s_texture1, v_clipPos.xy / v_clipPos.w)) * u_channelFlag;"
        "float maskVal = clipMask.r + clipMask.g + clipMask.b + clipMask.a;"
        "col_formask = col_formask * (1.0 - maskVal);"
        "gl_FragColor = col_formask;"
        "}";
#if defined(CSM_TARGET_ANDROID_ES2)
static const csmChar* FragShaderSrcMaskInvertedPremultipliedAlphaTegra =
        "#version 100\n"
        "#extension GL_NV_shader_framebuffer_fetch : enable\n"
        "precision " CSM_FRAGMENT_SHADER_FP_PRECISION " float;"
        "varying vec2 v_texCoord;"
        "varying vec4 v_clipPos;"
        "uniform sampler2D s_texture0;"
        "uniform sampler2D s_texture1;"
        "uniform vec4 u_channelFlag;"
        "uniform vec4 u_baseColor;"
        "uniform vec4 u_multiplyColor;"
        "uniform vec4 u_screenColor;"
        "void main()"
        "{"
        "vec4 texColor = texture2D(s_texture0 , v_texCoord);"
        "texColor.rgb = texColor.rgb * u_multiplyColor.rgb;"
        "texColor.rgb = (texColor.rgb + u_screenColor.rgb * texColor.a) - (texColor.rgb * u_screenColor.rgb);"
        "vec4 col_formask = texColor * u_baseColor;"
        "vec4 clipMask = (1.0 - texture2D(s_texture1, v_clipPos.xy / v_clipPos.w)) * u_channelFlag;"
        "float maskVal = clipMask.r + clipMask.g + clipMask.b + clipMask.a;"
        "col_formask = col_formask * (1.0 - maskVal);"
        "gl_FragColor = col_formask;"
        "}";
#endif

void CubismShader_OpenGLES2::ReleaseShaderProgram()
{
    for (csmUint32 i = 0; i < _shaderSets.GetSize(); i++)
    {
        if (_shaderSets[i]->ShaderProgram)
        {
            glDeleteProgram(_shaderSets[i]->ShaderProgram);
            _shaderSets[i]->ShaderProgram = 0;
            CSM_DELETE(_shaderSets[i]);
        }
    }
}

CubismShader_OpenGLES2::CubismShader_OpenGLES2()
{ }

CubismShader_OpenGLES2::~CubismShader_OpenGLES2()
{
    ReleaseShaderProgram();
}

CubismShader_OpenGLES2* CubismShader_OpenGLES2::GetInstance()
{
    if (s_instance == NULL)
    {
        s_instance = CSM_NEW CubismShader_OpenGLES2();
    }
    return s_instance;
}

void CubismShader_OpenGLES2::DeleteInstance()
{
    if (s_instance)
    {
        CSM_DELETE_SELF(CubismShader_OpenGLES2, s_instance);
        s_instance = NULL;
    }
}

#ifdef CSM_TARGET_ANDROID_ES2
csmBool CubismShader_OpenGLES2::s_extMode = false;
csmBool CubismShader_OpenGLES2::s_extPAMode = false;
void CubismShader_OpenGLES2::SetExtShaderMode(csmBool extMode, csmBool extPAMode) {
    s_extMode = extMode;
    s_extPAMode = extPAMode;
}
#endif

void CubismShader_OpenGLES2::GenerateShaders()
{
    for (csmInt32 i = 0; i < ShaderCount; i++)
    {
        _shaderSets.PushBack(CSM_NEW CubismShaderSet());
    }

#ifdef CSM_TARGET_ANDROID_ES2
    if (s_extMode)
    {
        _shaderSets[0]->ShaderProgram = LoadShaderProgram(VertShaderSrcSetupMask, FragShaderSrcSetupMaskTegra);

        _shaderSets[1]->ShaderProgram = LoadShaderProgram(VertShaderSrc, FragShaderSrcTegra);
        _shaderSets[2]->ShaderProgram = LoadShaderProgram(VertShaderSrcMasked, FragShaderSrcMaskTegra);
        _shaderSets[3]->ShaderProgram = LoadShaderProgram(VertShaderSrcMasked, FragShaderSrcMaskInvertedTegra);
        _shaderSets[4]->ShaderProgram = LoadShaderProgram(VertShaderSrc, FragShaderSrcPremultipliedAlphaTegra);
        _shaderSets[5]->ShaderProgram = LoadShaderProgram(VertShaderSrcMasked, FragShaderSrcMaskPremultipliedAlphaTegra);
        _shaderSets[6]->ShaderProgram = LoadShaderProgram(VertShaderSrcMasked, FragShaderSrcMaskInvertedPremultipliedAlphaTegra);
    }
    else
    {
        _shaderSets[0]->ShaderProgram = LoadShaderProgram(VertShaderSrcSetupMask, FragShaderSrcSetupMask);

        _shaderSets[1]->ShaderProgram = LoadShaderProgram(VertShaderSrc, FragShaderSrc);
        _shaderSets[2]->ShaderProgram = LoadShaderProgram(VertShaderSrcMasked, FragShaderSrcMask);
        _shaderSets[3]->ShaderProgram = LoadShaderProgram(VertShaderSrcMasked, FragShaderSrcMaskInverted);
        _shaderSets[4]->ShaderProgram = LoadShaderProgram(VertShaderSrc, FragShaderSrcPremultipliedAlpha);
        _shaderSets[5]->ShaderProgram = LoadShaderProgram(VertShaderSrcMasked, FragShaderSrcMaskPremultipliedAlpha);
        _shaderSets[6]->ShaderProgram = LoadShaderProgram(VertShaderSrcMasked, FragShaderSrcMaskInvertedPremultipliedAlpha);
    }

    // 加算も通常と同じシェーダーを利用する
    _shaderSets[7]->ShaderProgram = _shaderSets[1]->ShaderProgram;
    _shaderSets[8]->ShaderProgram = _shaderSets[2]->ShaderProgram;
    _shaderSets[9]->ShaderProgram = _shaderSets[3]->ShaderProgram;
    _shaderSets[10]->ShaderProgram = _shaderSets[4]->ShaderProgram;
    _shaderSets[11]->ShaderProgram = _shaderSets[5]->ShaderProgram;
    _shaderSets[12]->ShaderProgram = _shaderSets[6]->ShaderProgram;

    // 乗算も通常と同じシェーダーを利用する
    _shaderSets[13]->ShaderProgram = _shaderSets[1]->ShaderProgram;
    _shaderSets[14]->ShaderProgram = _shaderSets[2]->ShaderProgram;
    _shaderSets[15]->ShaderProgram = _shaderSets[3]->ShaderProgram;
    _shaderSets[16]->ShaderProgram = _shaderSets[4]->ShaderProgram;
    _shaderSets[17]->ShaderProgram = _shaderSets[5]->ShaderProgram;
    _shaderSets[18]->ShaderProgram = _shaderSets[6]->ShaderProgram;

#else

    _shaderSets[0]->ShaderProgram = LoadShaderProgram(VertShaderSrcSetupMask, FragShaderSrcSetupMask);

    _shaderSets[1]->ShaderProgram = LoadShaderProgram(VertShaderSrc, FragShaderSrc);
    _shaderSets[2]->ShaderProgram = LoadShaderProgram(VertShaderSrcMasked, FragShaderSrcMask);
    _shaderSets[3]->ShaderProgram = LoadShaderProgram(VertShaderSrcMasked, FragShaderSrcMaskInverted);
    _shaderSets[4]->ShaderProgram = LoadShaderProgram(VertShaderSrc, FragShaderSrcPremultipliedAlpha);
    _shaderSets[5]->ShaderProgram = LoadShaderProgram(VertShaderSrcMasked, FragShaderSrcMaskPremultipliedAlpha);
    _shaderSets[6]->ShaderProgram = LoadShaderProgram(VertShaderSrcMasked, FragShaderSrcMaskInvertedPremultipliedAlpha);

    // 加算も通常と同じシェーダーを利用する
    _shaderSets[7]->ShaderProgram = _shaderSets[1]->ShaderProgram;
    _shaderSets[8]->ShaderProgram = _shaderSets[2]->ShaderProgram;
    _shaderSets[9]->ShaderProgram = _shaderSets[3]->ShaderProgram;
    _shaderSets[10]->ShaderProgram = _shaderSets[4]->ShaderProgram;
    _shaderSets[11]->ShaderProgram = _shaderSets[5]->ShaderProgram;
    _shaderSets[12]->ShaderProgram = _shaderSets[6]->ShaderProgram;

    // 乗算も通常と同じシェーダーを利用する
    _shaderSets[13]->ShaderProgram = _shaderSets[1]->ShaderProgram;
    _shaderSets[14]->ShaderProgram = _shaderSets[2]->ShaderProgram;
    _shaderSets[15]->ShaderProgram = _shaderSets[3]->ShaderProgram;
    _shaderSets[16]->ShaderProgram = _shaderSets[4]->ShaderProgram;
    _shaderSets[17]->ShaderProgram = _shaderSets[5]->ShaderProgram;
    _shaderSets[18]->ShaderProgram = _shaderSets[6]->ShaderProgram;
#endif

    // SetupMask
    _shaderSets[0]->AttributePositionLocation = glGetAttribLocation(_shaderSets[0]->ShaderProgram, "a_position");
    _shaderSets[0]->AttributeTexCoordLocation = glGetAttribLocation(_shaderSets[0]->ShaderProgram, "a_texCoord");
    _shaderSets[0]->SamplerTexture0Location = glGetUniformLocation(_shaderSets[0]->ShaderProgram, "s_texture0");
    _shaderSets[0]->UniformClipMatrixLocation = glGetUniformLocation(_shaderSets[0]->ShaderProgram, "u_clipMatrix");
    _shaderSets[0]->UnifromChannelFlagLocation = glGetUniformLocation(_shaderSets[0]->ShaderProgram, "u_channelFlag");
    _shaderSets[0]->UniformBaseColorLocation = glGetUniformLocation(_shaderSets[0]->ShaderProgram, "u_baseColor");
    _shaderSets[0]->UniformMultiplyColorLocation = glGetUniformLocation(_shaderSets[0]->ShaderProgram, "u_multiplyColor");
    _shaderSets[0]->UniformScreenColorLocation = glGetUniformLocation(_shaderSets[0]->ShaderProgram, "u_screenColor");

    // 通常
    _shaderSets[1]->AttributePositionLocation = glGetAttribLocation(_shaderSets[1]->ShaderProgram, "a_position");
    _shaderSets[1]->AttributeTexCoordLocation = glGetAttribLocation(_shaderSets[1]->ShaderProgram, "a_texCoord");
    _shaderSets[1]->SamplerTexture0Location = glGetUniformLocation(_shaderSets[1]->ShaderProgram, "s_texture0");
    _shaderSets[1]->UniformMatrixLocation = glGetUniformLocation(_shaderSets[1]->ShaderProgram, "u_matrix");
    _shaderSets[1]->UniformBaseColorLocation = glGetUniformLocation(_shaderSets[1]->ShaderProgram, "u_baseColor");
    _shaderSets[1]->UniformMultiplyColorLocation = glGetUniformLocation(_shaderSets[1]->ShaderProgram, "u_multiplyColor");
    _shaderSets[1]->UniformScreenColorLocation = glGetUniformLocation(_shaderSets[1]->ShaderProgram, "u_screenColor");

    // 通常（クリッピング）
    _shaderSets[2]->AttributePositionLocation = glGetAttribLocation(_shaderSets[2]->ShaderProgram, "a_position");
    _shaderSets[2]->AttributeTexCoordLocation = glGetAttribLocation(_shaderSets[2]->ShaderProgram, "a_texCoord");
    _shaderSets[2]->SamplerTexture0Location = glGetUniformLocation(_shaderSets[2]->ShaderProgram, "s_texture0");
    _shaderSets[2]->SamplerTexture1Location = glGetUniformLocation(_shaderSets[2]->ShaderProgram, "s_texture1");
    _shaderSets[2]->UniformMatrixLocation = glGetUniformLocation(_shaderSets[2]->ShaderProgram, "u_matrix");
    _shaderSets[2]->UniformClipMatrixLocation = glGetUniformLocation(_shaderSets[2]->ShaderProgram, "u_clipMatrix");
    _shaderSets[2]->UnifromChannelFlagLocation = glGetUniformLocation(_shaderSets[2]->ShaderProgram, "u_channelFlag");
    _shaderSets[2]->UniformBaseColorLocation = glGetUniformLocation(_shaderSets[2]->ShaderProgram, "u_baseColor");
    _shaderSets[2]->UniformMultiplyColorLocation = glGetUniformLocation(_shaderSets[2]->ShaderProgram, "u_multiplyColor");
    _shaderSets[2]->UniformScreenColorLocation = glGetUniformLocation(_shaderSets[2]->ShaderProgram, "u_screenColor");

    // 通常（クリッピング・反転）
    _shaderSets[3]->AttributePositionLocation = glGetAttribLocation(_shaderSets[3]->ShaderProgram, "a_position");
    _shaderSets[3]->AttributeTexCoordLocation = glGetAttribLocation(_shaderSets[3]->ShaderProgram, "a_texCoord");
    _shaderSets[3]->SamplerTexture0Location = glGetUniformLocation(_shaderSets[3]->ShaderProgram, "s_texture0");
    _shaderSets[3]->SamplerTexture1Location = glGetUniformLocation(_shaderSets[3]->ShaderProgram, "s_texture1");
    _shaderSets[3]->UniformMatrixLocation = glGetUniformLocation(_shaderSets[3]->ShaderProgram, "u_matrix");
    _shaderSets[3]->UniformClipMatrixLocation = glGetUniformLocation(_shaderSets[3]->ShaderProgram, "u_clipMatrix");
    _shaderSets[3]->UnifromChannelFlagLocation = glGetUniformLocation(_shaderSets[3]->ShaderProgram, "u_channelFlag");
    _shaderSets[3]->UniformBaseColorLocation = glGetUniformLocation(_shaderSets[3]->ShaderProgram, "u_baseColor");
    _shaderSets[3]->UniformMultiplyColorLocation = glGetUniformLocation(_shaderSets[3]->ShaderProgram, "u_multiplyColor");
    _shaderSets[3]->UniformScreenColorLocation = glGetUniformLocation(_shaderSets[3]->ShaderProgram, "u_screenColor");

    // 通常（PremultipliedAlpha）
    _shaderSets[4]->AttributePositionLocation = glGetAttribLocation(_shaderSets[4]->ShaderProgram, "a_position");
    _shaderSets[4]->AttributeTexCoordLocation = glGetAttribLocation(_shaderSets[4]->ShaderProgram, "a_texCoord");
    _shaderSets[4]->SamplerTexture0Location = glGetUniformLocation(_shaderSets[4]->ShaderProgram, "s_texture0");
    _shaderSets[4]->UniformMatrixLocation = glGetUniformLocation(_shaderSets[4]->ShaderProgram, "u_matrix");
    _shaderSets[4]->UniformBaseColorLocation = glGetUniformLocation(_shaderSets[4]->ShaderProgram, "u_baseColor");
    _shaderSets[4]->UniformMultiplyColorLocation = glGetUniformLocation(_shaderSets[4]->ShaderProgram, "u_multiplyColor");
    _shaderSets[4]->UniformScreenColorLocation = glGetUniformLocation(_shaderSets[4]->ShaderProgram, "u_screenColor");

    // 通常（クリッピング、PremultipliedAlpha）
    _shaderSets[5]->AttributePositionLocation = glGetAttribLocation(_shaderSets[5]->ShaderProgram, "a_position");
    _shaderSets[5]->AttributeTexCoordLocation = glGetAttribLocation(_shaderSets[5]->ShaderProgram, "a_texCoord");
    _shaderSets[5]->SamplerTexture0Location = glGetUniformLocation(_shaderSets[5]->ShaderProgram, "s_texture0");
    _shaderSets[5]->SamplerTexture1Location = glGetUniformLocation(_shaderSets[5]->ShaderProgram, "s_texture1");
    _shaderSets[5]->UniformMatrixLocation = glGetUniformLocation(_shaderSets[5]->ShaderProgram, "u_matrix");
    _shaderSets[5]->UniformClipMatrixLocation = glGetUniformLocation(_shaderSets[5]->ShaderProgram, "u_clipMatrix");
    _shaderSets[5]->UnifromChannelFlagLocation = glGetUniformLocation(_shaderSets[5]->ShaderProgram, "u_channelFlag");
    _shaderSets[5]->UniformBaseColorLocation = glGetUniformLocation(_shaderSets[5]->ShaderProgram, "u_baseColor");
    _shaderSets[5]->UniformMultiplyColorLocation = glGetUniformLocation(_shaderSets[5]->ShaderProgram, "u_multiplyColor");
    _shaderSets[5]->UniformScreenColorLocation = glGetUniformLocation(_shaderSets[5]->ShaderProgram, "u_screenColor");

    // 通常（クリッピング・反転、PremultipliedAlpha）
    _shaderSets[6]->AttributePositionLocation = glGetAttribLocation(_shaderSets[6]->ShaderProgram, "a_position");
    _shaderSets[6]->AttributeTexCoordLocation = glGetAttribLocation(_shaderSets[6]->ShaderProgram, "a_texCoord");
    _shaderSets[6]->SamplerTexture0Location = glGetUniformLocation(_shaderSets[6]->ShaderProgram, "s_texture0");
    _shaderSets[6]->SamplerTexture1Location = glGetUniformLocation(_shaderSets[6]->ShaderProgram, "s_texture1");
    _shaderSets[6]->UniformMatrixLocation = glGetUniformLocation(_shaderSets[6]->ShaderProgram, "u_matrix");
    _shaderSets[6]->UniformClipMatrixLocation = glGetUniformLocation(_shaderSets[6]->ShaderProgram, "u_clipMatrix");
    _shaderSets[6]->UnifromChannelFlagLocation = glGetUniformLocation(_shaderSets[6]->ShaderProgram, "u_channelFlag");
    _shaderSets[6]->UniformBaseColorLocation = glGetUniformLocation(_shaderSets[6]->ShaderProgram, "u_baseColor");
    _shaderSets[6]->UniformMultiplyColorLocation = glGetUniformLocation(_shaderSets[6]->ShaderProgram, "u_multiplyColor");
    _shaderSets[6]->UniformScreenColorLocation = glGetUniformLocation(_shaderSets[6]->ShaderProgram, "u_screenColor");

    // 加算
    _shaderSets[7]->AttributePositionLocation = glGetAttribLocation(_shaderSets[7]->ShaderProgram, "a_position");
    _shaderSets[7]->AttributeTexCoordLocation = glGetAttribLocation(_shaderSets[7]->ShaderProgram, "a_texCoord");
    _shaderSets[7]->SamplerTexture0Location = glGetUniformLocation(_shaderSets[7]->ShaderProgram, "s_texture0");
    _shaderSets[7]->UniformMatrixLocation = glGetUniformLocation(_shaderSets[7]->ShaderProgram, "u_matrix");
    _shaderSets[7]->UniformBaseColorLocation = glGetUniformLocation(_shaderSets[7]->ShaderProgram, "u_baseColor");
    _shaderSets[7]->UniformMultiplyColorLocation = glGetUniformLocation(_shaderSets[7]->ShaderProgram, "u_multiplyColor");
    _shaderSets[7]->UniformScreenColorLocation = glGetUniformLocation(_shaderSets[7]->ShaderProgram, "u_screenColor");

    // 加算（クリッピング）
    _shaderSets[8]->AttributePositionLocation = glGetAttribLocation(_shaderSets[8]->ShaderProgram, "a_position");
    _shaderSets[8]->AttributeTexCoordLocation = glGetAttribLocation(_shaderSets[8]->ShaderProgram, "a_texCoord");
    _shaderSets[8]->SamplerTexture0Location = glGetUniformLocation(_shaderSets[8]->ShaderProgram, "s_texture0");
    _shaderSets[8]->SamplerTexture1Location = glGetUniformLocation(_shaderSets[8]->ShaderProgram, "s_texture1");
    _shaderSets[8]->UniformMatrixLocation = glGetUniformLocation(_shaderSets[8]->ShaderProgram, "u_matrix");
    _shaderSets[8]->UniformClipMatrixLocation = glGetUniformLocation(_shaderSets[8]->ShaderProgram, "u_clipMatrix");
    _shaderSets[8]->UnifromChannelFlagLocation = glGetUniformLocation(_shaderSets[8]->ShaderProgram, "u_channelFlag");
    _shaderSets[8]->UniformBaseColorLocation = glGetUniformLocation(_shaderSets[8]->ShaderProgram, "u_baseColor");
    _shaderSets[8]->UniformMultiplyColorLocation = glGetUniformLocation(_shaderSets[8]->ShaderProgram, "u_multiplyColor");
    _shaderSets[8]->UniformScreenColorLocation = glGetUniformLocation(_shaderSets[8]->ShaderProgram, "u_screenColor");

    // 加算（クリッピング・反転）
    _shaderSets[9]->AttributePositionLocation = glGetAttribLocation(_shaderSets[9]->ShaderProgram, "a_position");
    _shaderSets[9]->AttributeTexCoordLocation = glGetAttribLocation(_shaderSets[9]->ShaderProgram, "a_texCoord");
    _shaderSets[9]->SamplerTexture0Location = glGetUniformLocation(_shaderSets[9]->ShaderProgram, "s_texture0");
    _shaderSets[9]->SamplerTexture1Location = glGetUniformLocation(_shaderSets[9]->ShaderProgram, "s_texture1");
    _shaderSets[9]->UniformMatrixLocation = glGetUniformLocation(_shaderSets[9]->ShaderProgram, "u_matrix");
    _shaderSets[9]->UniformClipMatrixLocation = glGetUniformLocation(_shaderSets[9]->ShaderProgram, "u_clipMatrix");
    _shaderSets[9]->UnifromChannelFlagLocation = glGetUniformLocation(_shaderSets[9]->ShaderProgram, "u_channelFlag");
    _shaderSets[9]->UniformBaseColorLocation = glGetUniformLocation(_shaderSets[9]->ShaderProgram, "u_baseColor");
    _shaderSets[9]->UniformMultiplyColorLocation = glGetUniformLocation(_shaderSets[9]->ShaderProgram, "u_multiplyColor");
    _shaderSets[9]->UniformScreenColorLocation = glGetUniformLocation(_shaderSets[9]->ShaderProgram, "u_screenColor");

    // 加算（PremultipliedAlpha）
    _shaderSets[10]->AttributePositionLocation = glGetAttribLocation(_shaderSets[10]->ShaderProgram, "a_position");
    _shaderSets[10]->AttributeTexCoordLocation = glGetAttribLocation(_shaderSets[10]->ShaderProgram, "a_texCoord");
    _shaderSets[10]->SamplerTexture0Location = glGetUniformLocation(_shaderSets[10]->ShaderProgram, "s_texture0");
    _shaderSets[10]->UniformMatrixLocation = glGetUniformLocation(_shaderSets[10]->ShaderProgram, "u_matrix");
    _shaderSets[10]->UniformBaseColorLocation = glGetUniformLocation(_shaderSets[10]->ShaderProgram, "u_baseColor");
    _shaderSets[10]->UniformMultiplyColorLocation = glGetUniformLocation(_shaderSets[10]->ShaderProgram, "u_multiplyColor");
    _shaderSets[10]->UniformScreenColorLocation = glGetUniformLocation(_shaderSets[10]->ShaderProgram, "u_screenColor");

    // 加算（クリッピング、PremultipliedAlpha）
    _shaderSets[11]->AttributePositionLocation = glGetAttribLocation(_shaderSets[11]->ShaderProgram, "a_position");
    _shaderSets[11]->AttributeTexCoordLocation = glGetAttribLocation(_shaderSets[11]->ShaderProgram, "a_texCoord");
    _shaderSets[11]->SamplerTexture0Location = glGetUniformLocation(_shaderSets[11]->ShaderProgram, "s_texture0");
    _shaderSets[11]->SamplerTexture1Location = glGetUniformLocation(_shaderSets[11]->ShaderProgram, "s_texture1");
    _shaderSets[11]->UniformMatrixLocation = glGetUniformLocation(_shaderSets[11]->ShaderProgram, "u_matrix");
    _shaderSets[11]->UniformClipMatrixLocation = glGetUniformLocation(_shaderSets[11]->ShaderProgram, "u_clipMatrix");
    _shaderSets[11]->UnifromChannelFlagLocation = glGetUniformLocation(_shaderSets[11]->ShaderProgram, "u_channelFlag");
    _shaderSets[11]->UniformBaseColorLocation = glGetUniformLocation(_shaderSets[11]->ShaderProgram, "u_baseColor");
    _shaderSets[11]->UniformMultiplyColorLocation = glGetUniformLocation(_shaderSets[11]->ShaderProgram, "u_multiplyColor");
    _shaderSets[11]->UniformScreenColorLocation = glGetUniformLocation(_shaderSets[11]->ShaderProgram, "u_screenColor");

    // 加算（クリッピング・反転、PremultipliedAlpha）
    _shaderSets[12]->AttributePositionLocation = glGetAttribLocation(_shaderSets[12]->ShaderProgram, "a_position");
    _shaderSets[12]->AttributeTexCoordLocation = glGetAttribLocation(_shaderSets[12]->ShaderProgram, "a_texCoord");
    _shaderSets[12]->SamplerTexture0Location = glGetUniformLocation(_shaderSets[12]->ShaderProgram, "s_texture0");
    _shaderSets[12]->SamplerTexture1Location = glGetUniformLocation(_shaderSets[12]->ShaderProgram, "s_texture1");
    _shaderSets[12]->UniformMatrixLocation = glGetUniformLocation(_shaderSets[12]->ShaderProgram, "u_matrix");
    _shaderSets[12]->UniformClipMatrixLocation = glGetUniformLocation(_shaderSets[12]->ShaderProgram, "u_clipMatrix");
    _shaderSets[12]->UnifromChannelFlagLocation = glGetUniformLocation(_shaderSets[12]->ShaderProgram, "u_channelFlag");
    _shaderSets[12]->UniformBaseColorLocation = glGetUniformLocation(_shaderSets[12]->ShaderProgram, "u_baseColor");
    _shaderSets[12]->UniformMultiplyColorLocation = glGetUniformLocation(_shaderSets[12]->ShaderProgram, "u_multiplyColor");
    _shaderSets[12]->UniformScreenColorLocation = glGetUniformLocation(_shaderSets[12]->ShaderProgram, "u_screenColor");

    // 乗算
    _shaderSets[13]->AttributePositionLocation = glGetAttribLocation(_shaderSets[13]->ShaderProgram, "a_position");
    _shaderSets[13]->AttributeTexCoordLocation = glGetAttribLocation(_shaderSets[13]->ShaderProgram, "a_texCoord");
    _shaderSets[13]->SamplerTexture0Location = glGetUniformLocation(_shaderSets[13]->ShaderProgram, "s_texture0");
    _shaderSets[13]->UniformMatrixLocation = glGetUniformLocation(_shaderSets[13]->ShaderProgram, "u_matrix");
    _shaderSets[13]->UniformBaseColorLocation = glGetUniformLocation(_shaderSets[13]->ShaderProgram, "u_baseColor");
    _shaderSets[13]->UniformMultiplyColorLocation = glGetUniformLocation(_shaderSets[13]->ShaderProgram, "u_multiplyColor");
    _shaderSets[13]->UniformScreenColorLocation = glGetUniformLocation(_shaderSets[13]->ShaderProgram, "u_screenColor");

    // 乗算（クリッピング）
    _shaderSets[14]->AttributePositionLocation = glGetAttribLocation(_shaderSets[14]->ShaderProgram, "a_position");
    _shaderSets[14]->AttributeTexCoordLocation = glGetAttribLocation(_shaderSets[14]->ShaderProgram, "a_texCoord");
    _shaderSets[14]->SamplerTexture0Location = glGetUniformLocation(_shaderSets[14]->ShaderProgram, "s_texture0");
    _shaderSets[14]->SamplerTexture1Location = glGetUniformLocation(_shaderSets[14]->ShaderProgram, "s_texture1");
    _shaderSets[14]->UniformMatrixLocation = glGetUniformLocation(_shaderSets[14]->ShaderProgram, "u_matrix");
    _shaderSets[14]->UniformClipMatrixLocation = glGetUniformLocation(_shaderSets[14]->ShaderProgram, "u_clipMatrix");
    _shaderSets[14]->UnifromChannelFlagLocation = glGetUniformLocation(_shaderSets[14]->ShaderProgram, "u_channelFlag");
    _shaderSets[14]->UniformBaseColorLocation = glGetUniformLocation(_shaderSets[14]->ShaderProgram, "u_baseColor");
    _shaderSets[14]->UniformMultiplyColorLocation = glGetUniformLocation(_shaderSets[14]->ShaderProgram, "u_multiplyColor");
    _shaderSets[14]->UniformScreenColorLocation = glGetUniformLocation(_shaderSets[14]->ShaderProgram, "u_screenColor");

    // 乗算（クリッピング・反転）
    _shaderSets[15]->AttributePositionLocation = glGetAttribLocation(_shaderSets[15]->ShaderProgram, "a_position");
    _shaderSets[15]->AttributeTexCoordLocation = glGetAttribLocation(_shaderSets[15]->ShaderProgram, "a_texCoord");
    _shaderSets[15]->SamplerTexture0Location = glGetUniformLocation(_shaderSets[15]->ShaderProgram, "s_texture0");
    _shaderSets[15]->SamplerTexture1Location = glGetUniformLocation(_shaderSets[15]->ShaderProgram, "s_texture1");
    _shaderSets[15]->UniformMatrixLocation = glGetUniformLocation(_shaderSets[15]->ShaderProgram, "u_matrix");
    _shaderSets[15]->UniformClipMatrixLocation = glGetUniformLocation(_shaderSets[15]->ShaderProgram, "u_clipMatrix");
    _shaderSets[15]->UnifromChannelFlagLocation = glGetUniformLocation(_shaderSets[15]->ShaderProgram, "u_channelFlag");
    _shaderSets[15]->UniformBaseColorLocation = glGetUniformLocation(_shaderSets[15]->ShaderProgram, "u_baseColor");
    _shaderSets[15]->UniformMultiplyColorLocation = glGetUniformLocation(_shaderSets[15]->ShaderProgram, "u_multiplyColor");
    _shaderSets[15]->UniformScreenColorLocation = glGetUniformLocation(_shaderSets[15]->ShaderProgram, "u_screenColor");

    // 乗算（PremultipliedAlpha）
    _shaderSets[16]->AttributePositionLocation = glGetAttribLocation(_shaderSets[16]->ShaderProgram, "a_position");
    _shaderSets[16]->AttributeTexCoordLocation = glGetAttribLocation(_shaderSets[16]->ShaderProgram, "a_texCoord");
    _shaderSets[16]->SamplerTexture0Location = glGetUniformLocation(_shaderSets[16]->ShaderProgram, "s_texture0");
    _shaderSets[16]->UniformMatrixLocation = glGetUniformLocation(_shaderSets[16]->ShaderProgram, "u_matrix");
    _shaderSets[16]->UniformBaseColorLocation = glGetUniformLocation(_shaderSets[16]->ShaderProgram, "u_baseColor");
    _shaderSets[16]->UniformMultiplyColorLocation = glGetUniformLocation(_shaderSets[16]->ShaderProgram, "u_multiplyColor");
    _shaderSets[16]->UniformScreenColorLocation = glGetUniformLocation(_shaderSets[16]->ShaderProgram, "u_screenColor");

    // 乗算（クリッピング、PremultipliedAlpha）
    _shaderSets[17]->AttributePositionLocation = glGetAttribLocation(_shaderSets[17]->ShaderProgram, "a_position");
    _shaderSets[17]->AttributeTexCoordLocation = glGetAttribLocation(_shaderSets[17]->ShaderProgram, "a_texCoord");
    _shaderSets[17]->SamplerTexture0Location = glGetUniformLocation(_shaderSets[17]->ShaderProgram, "s_texture0");
    _shaderSets[17]->SamplerTexture1Location = glGetUniformLocation(_shaderSets[17]->ShaderProgram, "s_texture1");
    _shaderSets[17]->UniformMatrixLocation = glGetUniformLocation(_shaderSets[17]->ShaderProgram, "u_matrix");
    _shaderSets[17]->UniformClipMatrixLocation = glGetUniformLocation(_shaderSets[17]->ShaderProgram, "u_clipMatrix");
    _shaderSets[17]->UnifromChannelFlagLocation = glGetUniformLocation(_shaderSets[17]->ShaderProgram, "u_channelFlag");
    _shaderSets[17]->UniformBaseColorLocation = glGetUniformLocation(_shaderSets[17]->ShaderProgram, "u_baseColor");
    _shaderSets[17]->UniformMultiplyColorLocation = glGetUniformLocation(_shaderSets[17]->ShaderProgram, "u_multiplyColor");
    _shaderSets[17]->UniformScreenColorLocation = glGetUniformLocation(_shaderSets[17]->ShaderProgram, "u_screenColor");

    // 乗算（クリッピング・反転、PremultipliedAlpha）
    _shaderSets[18]->AttributePositionLocation = glGetAttribLocation(_shaderSets[18]->ShaderProgram, "a_position");
    _shaderSets[18]->AttributeTexCoordLocation = glGetAttribLocation(_shaderSets[18]->ShaderProgram, "a_texCoord");
    _shaderSets[18]->SamplerTexture0Location = glGetUniformLocation(_shaderSets[18]->ShaderProgram, "s_texture0");
    _shaderSets[18]->SamplerTexture1Location = glGetUniformLocation(_shaderSets[18]->ShaderProgram, "s_texture1");
    _shaderSets[18]->UniformMatrixLocation = glGetUniformLocation(_shaderSets[18]->ShaderProgram, "u_matrix");
    _shaderSets[18]->UniformClipMatrixLocation = glGetUniformLocation(_shaderSets[18]->ShaderProgram, "u_clipMatrix");
    _shaderSets[18]->UnifromChannelFlagLocation = glGetUniformLocation(_shaderSets[18]->ShaderProgram, "u_channelFlag");
    _shaderSets[18]->UniformBaseColorLocation = glGetUniformLocation(_shaderSets[18]->ShaderProgram, "u_baseColor");
    _shaderSets[18]->UniformMultiplyColorLocation = glGetUniformLocation(_shaderSets[18]->ShaderProgram, "u_multiplyColor");
    _shaderSets[18]->UniformScreenColorLocation = glGetUniformLocation(_shaderSets[18]->ShaderProgram, "u_screenColor");
}

void CubismShader_OpenGLES2::SetupShaderProgramForDraw(CubismRenderer_OpenGLES2* renderer, const CubismModel& model, const csmInt32 index)
{
    if (_shaderSets.GetSize() == 0)
    {
        GenerateShaders();
    }

    // Blending
    csmInt32 SRC_COLOR;
    csmInt32 DST_COLOR;
    csmInt32 SRC_ALPHA;
    csmInt32 DST_ALPHA;

    // _shaderSets用のオフセット計算
    const csmBool masked = renderer->GetClippingContextBufferForDraw() != NULL;  // この描画オブジェクトはマスク対象か
    const csmBool invertedMask = model.GetDrawableInvertedMask(index);
    const csmBool isPremultipliedAlpha = renderer->IsPremultipliedAlpha();
    const csmInt32 offset = (masked ? ( invertedMask ? 2 : 1 ) : 0) + (isPremultipliedAlpha ? 3 : 0);

    // シェーダーセット
    CubismShaderSet* shaderSet;
    switch (model.GetDrawableBlendMode(index))
    {
    case CubismRenderer::CubismBlendMode_Normal:
    default:
        shaderSet = _shaderSets[ShaderNames_Normal + offset];
        SRC_COLOR = GL_ONE;
        DST_COLOR = GL_ONE_MINUS_SRC_ALPHA;
        SRC_ALPHA = GL_ONE;
        DST_ALPHA = GL_ONE_MINUS_SRC_ALPHA;
        break;

    case CubismRenderer::CubismBlendMode_Additive:
        shaderSet = _shaderSets[ShaderNames_Add + offset];
        SRC_COLOR = GL_ONE;
        DST_COLOR = GL_ONE;
        SRC_ALPHA = GL_ZERO;
        DST_ALPHA = GL_ONE;
        break;

    case CubismRenderer::CubismBlendMode_Multiplicative:
        shaderSet = _shaderSets[ShaderNames_Mult + offset];
        SRC_COLOR = GL_DST_COLOR;
        DST_COLOR = GL_ONE_MINUS_SRC_ALPHA;
        SRC_ALPHA = GL_ZERO;
        DST_ALPHA = GL_ONE;
        break;
    }

    glUseProgram(shaderSet->ShaderProgram);

    //テクスチャ設定
    SetupTexture(renderer, model, index, shaderSet);

    // 頂点属性設定
    SetVertexAttributes(model, index, shaderSet);

    if (masked)
    {
        glActiveTexture(GL_TEXTURE1);

        // frameBufferに書かれたテクスチャ
        GLuint tex = renderer->GetMaskBuffer(renderer->GetClippingContextBufferForDraw()->_bufferIndex)->GetColorBuffer();

        glBindTexture(GL_TEXTURE_2D, tex);
        glUniform1i(shaderSet->SamplerTexture1Location, 1);

        // View座標をClippingContextの座標に変換するための行列を設定
        glUniformMatrix4fv(shaderSet->UniformClipMatrixLocation, 1, 0, renderer->GetClippingContextBufferForDraw()->_matrixForDraw.GetArray());

        // 使用するカラーチャンネルを設定
        SetColorChannelUniformVariables(shaderSet, renderer->GetClippingContextBufferForDraw());
    }

    //座標変換
    glUniformMatrix4fv(shaderSet->UniformMatrixLocation, 1, 0, renderer->GetMvpMatrix().GetArray()); //

    // ユニフォーム変数設定
    CubismRenderer::CubismTextureColor baseColor = renderer->GetModelColorWithOpacity(model.GetDrawableOpacity(index));
    CubismRenderer::CubismTextureColor multiplyColor = model.GetMultiplyColor(index);
    CubismRenderer::CubismTextureColor screenColor = model.GetScreenColor(index);
    SetColorUniformVariables(renderer, model, index, shaderSet, baseColor, multiplyColor, screenColor);

    glBlendFuncSeparate(SRC_COLOR, DST_COLOR, SRC_ALPHA, DST_ALPHA);
}

void CubismShader_OpenGLES2::SetupShaderProgramForMask(CubismRenderer_OpenGLES2* renderer, const CubismModel& model, const csmInt32 index)
{
    if (_shaderSets.GetSize() == 0)
    {
        GenerateShaders();
    }

    // Blending
    csmInt32 SRC_COLOR = GL_ZERO;
    csmInt32 DST_COLOR = GL_ONE_MINUS_SRC_COLOR;
    csmInt32 SRC_ALPHA = GL_ZERO;
    csmInt32 DST_ALPHA = GL_ONE_MINUS_SRC_ALPHA;

    CubismShaderSet* shaderSet = _shaderSets[ShaderNames_SetupMask];
    glUseProgram(shaderSet->ShaderProgram);

    //テクスチャ設定
    SetupTexture(renderer, model, index, shaderSet);

    // 頂点属性設定
    SetVertexAttributes(model, index, shaderSet);

    // 使用するカラーチャンネルを設定
    SetColorChannelUniformVariables(shaderSet, renderer->GetClippingContextBufferForMask());

    glUniformMatrix4fv(shaderSet->UniformClipMatrixLocation, 1, GL_FALSE, renderer->GetClippingContextBufferForMask()->_matrixForMask.GetArray());

    // ユニフォーム変数設定
    csmRectF* rect = renderer->GetClippingContextBufferForMask()->_layoutBounds;
    CubismRenderer::CubismTextureColor baseColor = {rect->X * 2.0f - 1.0f, rect->Y * 2.0f - 1.0f, rect->GetRight() * 2.0f - 1.0f, rect->GetBottom() * 2.0f - 1.0f};
    CubismRenderer::CubismTextureColor multiplyColor = model.GetMultiplyColor(index);
    CubismRenderer::CubismTextureColor screenColor = model.GetScreenColor(index);
    SetColorUniformVariables(renderer, model, index, shaderSet, baseColor, multiplyColor, screenColor);

    glBlendFuncSeparate(SRC_COLOR, DST_COLOR, SRC_ALPHA, DST_ALPHA);
}

csmBool CubismShader_OpenGLES2::CompileShaderSource(GLuint* outShader, GLenum shaderType, const csmChar* shaderSource)
{
    GLint status;
    const GLchar* source = shaderSource;

    *outShader = glCreateShader(shaderType);
    glShaderSource(*outShader, 1, &source, NULL);
    glCompileShader(*outShader);

    GLint logLength;
    glGetShaderiv(*outShader, GL_INFO_LOG_LENGTH, &logLength);
    if (logLength > 0)
    {
        GLchar* log = reinterpret_cast<GLchar*>(CSM_MALLOC(logLength));
        glGetShaderInfoLog(*outShader, logLength, &logLength, log);
        CubismLogError("Shader compile log: %s", log);
        CSM_FREE(log);
    }

    glGetShaderiv(*outShader, GL_COMPILE_STATUS, &status);
    if (status == GL_FALSE)
    {
        glDeleteShader(*outShader);
        return false;
    }

    return true;
}

csmBool CubismShader_OpenGLES2::LinkProgram(GLuint shaderProgram)
{
    GLint status;
    glLinkProgram(shaderProgram);

    GLint logLength;
    glGetProgramiv(shaderProgram, GL_INFO_LOG_LENGTH, &logLength);
    if (logLength > 0)
    {
        GLchar* log = reinterpret_cast<GLchar*>(CSM_MALLOC(logLength));
        glGetProgramInfoLog(shaderProgram, logLength, &logLength, log);
        CubismLogError("Program link log: %s", log);
        CSM_FREE(log);
    }

    glGetProgramiv(shaderProgram, GL_LINK_STATUS, &status);
    if (status == GL_FALSE)
    {
        return false;
    }

    return true;
}

csmBool CubismShader_OpenGLES2::ValidateProgram(GLuint shaderProgram)
{
    GLint logLength, status;

    glValidateProgram(shaderProgram);
    glGetProgramiv(shaderProgram, GL_INFO_LOG_LENGTH, &logLength);
    if (logLength > 0)
    {
        GLchar* log = reinterpret_cast<GLchar*>(CSM_MALLOC(logLength));
        glGetProgramInfoLog(shaderProgram, logLength, &logLength, log);
        CubismLogError("Validate program log: %s", log);
        CSM_FREE(log);
    }

    glGetProgramiv(shaderProgram, GL_VALIDATE_STATUS, &status);
    if (status == GL_FALSE)
    {
        return false;
    }

    return true;
}

GLuint CubismShader_OpenGLES2::LoadShaderProgram(const csmChar* vertShaderSrc, const csmChar* fragShaderSrc)
{
    GLuint vertShader, fragShader;

    // Create shader program.
    GLuint shaderProgram = glCreateProgram();

    if (!CompileShaderSource(&vertShader, GL_VERTEX_SHADER, vertShaderSrc))
    {
        CubismLogError("Vertex shader compile error!");
        return 0;
    }

    // Create and compile fragment shader.
    if (!CompileShaderSource(&fragShader, GL_FRAGMENT_SHADER, fragShaderSrc))
    {
        CubismLogError("Fragment shader compile error!");
        return 0;
    }

    // Attach vertex shader to program.
    glAttachShader(shaderProgram, vertShader);

    // Attach fragment shader to program.
    glAttachShader(shaderProgram, fragShader);

    // Link program.
    if (!LinkProgram(shaderProgram))
    {
        CubismLogError("Failed to link program: %d", shaderProgram);

        if (vertShader)
        {
            glDeleteShader(vertShader);
            vertShader = 0;
        }
        if (fragShader)
        {
            glDeleteShader(fragShader);
            fragShader = 0;
        }
        if (shaderProgram)
        {
            glDeleteProgram(shaderProgram);
            shaderProgram = 0;
        }

        return 0;
    }

    // Release vertex and fragment shaders.
    if (vertShader)
    {
        glDetachShader(shaderProgram, vertShader);
        glDeleteShader(vertShader);
    }

    if (fragShader)
    {
        glDetachShader(shaderProgram, fragShader);
        glDeleteShader(fragShader);
    }

    return shaderProgram;
}

void CubismShader_OpenGLES2::SetVertexAttributes(const CubismModel& model, const csmInt32 index, CubismShaderSet* shaderSet)
{
    // 頂点位置属性の設定
    const csmFloat32* vertexArray = model.GetDrawableVertices(index);
    glEnableVertexAttribArray(shaderSet->AttributePositionLocation);
    glVertexAttribPointer(shaderSet->AttributePositionLocation, 2, GL_FLOAT, GL_FALSE, sizeof(csmFloat32) * 2, vertexArray);

    // テクスチャ座標属性の設定
    const csmFloat32* uvArray = reinterpret_cast<const csmFloat32*>(model.GetDrawableVertexUvs(index));
    glEnableVertexAttribArray(shaderSet->AttributeTexCoordLocation);
    glVertexAttribPointer(shaderSet->AttributeTexCoordLocation, 2, GL_FLOAT, GL_FALSE, sizeof(csmFloat32) * 2, uvArray);
}

void CubismShader_OpenGLES2::SetupTexture(CubismRenderer_OpenGLES2* renderer, const CubismModel& model, const csmInt32 index, CubismShaderSet* shaderSet)
{
    const csmInt32 textureIndex = model.GetDrawableTextureIndex(index);
    const GLuint textureId = renderer->GetBindedTextureId(textureIndex);
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, textureId);
    glUniform1i(shaderSet->SamplerTexture0Location, 0);
}

void CubismShader_OpenGLES2::SetColorUniformVariables(CubismRenderer_OpenGLES2* renderer, const CubismModel& model, const csmInt32 index, CubismShaderSet* shaderSet,
                                                      CubismRenderer::CubismTextureColor& baseColor, CubismRenderer::CubismTextureColor& multiplyColor, CubismRenderer::CubismTextureColor& screenColor)
{
    glUniform4f(shaderSet->UniformBaseColorLocation, baseColor.R, baseColor.G, baseColor.B, baseColor.A);
    glUniform4f(shaderSet->UniformMultiplyColorLocation, multiplyColor.R, multiplyColor.G, multiplyColor.B, multiplyColor.A);
    glUniform4f(shaderSet->UniformScreenColorLocation, screenColor.R, screenColor.G, screenColor.B, screenColor.A);
}

void CubismShader_OpenGLES2::SetColorChannelUniformVariables(CubismShaderSet* shaderSet, CubismClippingContext_OpenGLES2* contextBuffer)
{
    const csmInt32 channelIndex = contextBuffer->_layoutChannelIndex;
    CubismRenderer::CubismTextureColor* colorChannel = contextBuffer->GetClippingManager()->GetChannelFlagAsColor(channelIndex);
    glUniform4f(shaderSet->UnifromChannelFlagLocation, colorChannel->R, colorChannel->G, colorChannel->B, colorChannel->A);
}

}}}}

//------------ LIVE2D NAMESPACE ------------
