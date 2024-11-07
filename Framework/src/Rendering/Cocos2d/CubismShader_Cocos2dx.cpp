/**
 * Copyright(c) Live2D Inc. All rights reserved.
 *
 * Use of this source code is governed by the Live2D Open Software license
 * that can be found at https://www.live2d.com/eula/live2d-open-software-license-agreement_en.html.
 */

#include "CubismShader_Cocos2dx.hpp"
#include <float.h>
#include "renderer/backend/Device.h"

#ifdef CSM_TARGET_WIN_GL
#include <Windows.h>
#endif

//------------ LIVE2D NAMESPACE ------------
namespace Live2D { namespace Cubism { namespace Framework { namespace Rendering {

/*********************************************************************************************************************
*                                       CubismShader_Cocos2dx
********************************************************************************************************************/
namespace {
    const csmInt32 ShaderCount = 19; ///< シェーダの数 = マスク生成用 + (通常 + 加算 + 乗算) * (マスク無 + マスク有 + マスク有反転 + マスク無の乗算済アルファ対応版 + マスク有の乗算済アルファ対応版 + マスク有反転の乗算済アルファ対応版)
    CubismShader_Cocos2dx* s_instance;
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

void CubismShader_Cocos2dx::ReleaseShaderProgram()
{
    for (csmUint32 i = 0; i < _shaderSets.GetSize(); i++)
    {
        if (_shaderSets[i]->ShaderProgram)
        {
            CSM_DELETE(_shaderSets[i]);
        }
    }
}

// SetupMask
static const csmChar* VertShaderSrcSetupMask =
#if defined(CC_PLATFORM_MOBILE)
#else
    "#version 120\n"
#endif
    "attribute vec2 a_position;"
    "attribute vec2 a_texCoord;"
    "varying vec2 v_texCoord;"
    "varying vec4 v_myPos;"
    "uniform mat4 u_clipMatrix;"
    "void main()"
    "{"
    "vec4 pos = vec4(a_position.x, a_position.y, 0.0, 1.0);"
    "gl_Position = u_clipMatrix * pos;"
    "v_myPos = u_clipMatrix * pos;"
    "v_texCoord = a_texCoord;"
    "v_texCoord.y = 1.0 - v_texCoord.y;"
    "}";
static const csmChar* FragShaderSrcSetupMask =
#if defined(CC_PLATFORM_MOBILE)
#else
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
#if CC_TARGET_PLATFORM == CC_PLATFORM_ANDROID
static const csmChar* FragShaderSrcSetupMaskTegra =
    "#extension GL_NV_shader_framebuffer_fetch : enable\n"
    "precision mediump float;"
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
#if defined(CC_PLATFORM_MOBILE)
#else
    "#version 120\n"
#endif
    "attribute vec2 a_position;" //v.vertex
    "attribute vec2 a_texCoord;" //v.texcoord
    "varying vec2 v_texCoord;" //v2f.texcoord
    "uniform mat4 u_matrix;"
    "void main()"
    "{"
    "vec4 pos = vec4(a_position.x, a_position.y, 0.0, 1.0);"
    "gl_Position = u_matrix * pos;"
    "v_texCoord = a_texCoord;"
    "v_texCoord.y = 1.0 - v_texCoord.y;"
    "}";

// Normal & Add & Mult 共通（クリッピングされたものの描画用）
static const csmChar* VertShaderSrcMasked =
#if defined(CC_PLATFORM_MOBILE)
#else
    "#version 120\n"
#endif
    "attribute vec2 a_position;"
    "attribute vec2 a_texCoord;"
    "varying vec2 v_texCoord;"
    "varying vec4 v_clipPos;"
    "uniform mat4 u_matrix;"
    "uniform mat4 u_clipMatrix;"
    "void main()"
    "{"
    "vec4 pos = vec4(a_position.x, a_position.y, 0.0, 1.0);"
    "gl_Position = u_matrix * pos;"
    "v_clipPos = u_clipMatrix * pos;"
#if defined(CC_USE_METAL)
    "v_clipPos = vec4(v_clipPos.x, 1.0 - v_clipPos.y, v_clipPos.zw);"
#endif
    "v_texCoord = a_texCoord;"
    "v_texCoord.y = 1.0 - v_texCoord.y;"
    "}";

//----- フラグメントシェーダプログラム -----
// Normal & Add & Mult 共通
static const csmChar* FragShaderSrc =
#if defined(CC_PLATFORM_MOBILE)
#else
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
#if CC_TARGET_PLATFORM == CC_PLATFORM_ANDROID
static const csmChar* FragShaderSrcTegra =
    "#extension GL_NV_shader_framebuffer_fetch : enable\n"
    "precision mediump float;"
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
#if defined(CC_PLATFORM_MOBILE)
#else
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
#if CC_TARGET_PLATFORM == CC_PLATFORM_ANDROID
static const csmChar* FragShaderSrcPremultipliedAlphaTegra =
    "#extension GL_NV_shader_framebuffer_fetch : enable\n"
    "precision mediump float;"
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
#if defined(CC_PLATFORM_MOBILE)
#else
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
#if CC_TARGET_PLATFORM == CC_PLATFORM_ANDROID
static const csmChar* FragShaderSrcMaskTegra =
    "#extension GL_NV_shader_framebuffer_fetch : enable\n"
    "precision mediump float;"
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
#if defined(CC_PLATFORM_MOBILE)
#else
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
#if CC_TARGET_PLATFORM == CC_PLATFORM_ANDROID
static const csmChar* FragShaderSrcMaskInvertedTegra =
    "#extension GL_NV_shader_framebuffer_fetch : enable\n"
    "precision mediump float;"
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
#if defined(CC_PLATFORM_MOBILE)
#else
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
#if CC_TARGET_PLATFORM == CC_PLATFORM_ANDROID
static const csmChar* FragShaderSrcMaskPremultipliedAlphaTegra =
    "#extension GL_NV_shader_framebuffer_fetch : enable\n"
    "precision mediump float;"
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
#if defined(CC_PLATFORM_MOBILE)
#else
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
#if CC_TARGET_PLATFORM == CC_PLATFORM_ANDROID
static const csmChar* FragShaderSrcMaskInvertedPremultipliedAlphaTegra =
    "#extension GL_NV_shader_framebuffer_fetch : enable\n"
    "precision mediump float;"
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

CubismShader_Cocos2dx::CubismShader_Cocos2dx()
{ }

CubismShader_Cocos2dx::~CubismShader_Cocos2dx()
{
    ReleaseShaderProgram();
}

CubismShader_Cocos2dx* CubismShader_Cocos2dx::GetInstance()
{
    if (s_instance == NULL)
    {
        s_instance = CSM_NEW CubismShader_Cocos2dx();
    }
    return s_instance;
}

void CubismShader_Cocos2dx::DeleteInstance()
{
    if (s_instance)
    {
        CSM_DELETE_SELF(CubismShader_Cocos2dx, s_instance);
        s_instance = NULL;
    }
}

#ifdef CSM_TARGET_ANDROID_ES2
csmBool CubismShader_Cocos2dx::s_extMode = false;
csmBool CubismShader_Cocos2dx::s_extPAMode = false;
void CubismShader_Cocos2dx::SetExtShaderMode(csmBool extMode, csmBool extPAMode) {
    s_extMode = extMode;
    s_extPAMode = extPAMode;
}
#endif

void CubismShader_Cocos2dx::GenerateShaders()
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
    _shaderSets[0]->AttributePositionLocation = _shaderSets[0]->ShaderProgram->getAttributeLocation("a_position");
    _shaderSets[0]->AttributeTexCoordLocation = _shaderSets[0]->ShaderProgram->getAttributeLocation("a_texCoord");
    _shaderSets[0]->SamplerTexture0Location = _shaderSets[0]->ShaderProgram->getUniformLocation("s_texture0");
    _shaderSets[0]->UniformClipMatrixLocation = _shaderSets[0]->ShaderProgram->getUniformLocation("u_clipMatrix");
    _shaderSets[0]->UnifromChannelFlagLocation = _shaderSets[0]->ShaderProgram->getUniformLocation("u_channelFlag");
    _shaderSets[0]->UniformBaseColorLocation = _shaderSets[0]->ShaderProgram->getUniformLocation("u_baseColor");
    _shaderSets[0]->UniformMultiplyColorLocation = _shaderSets[0]->ShaderProgram->getUniformLocation("u_multiplyColor");
    _shaderSets[0]->UniformScreenColorLocation = _shaderSets[0]->ShaderProgram->getUniformLocation("u_screenColor");

    // 通常
    _shaderSets[1]->AttributePositionLocation = _shaderSets[1]->ShaderProgram->getAttributeLocation("a_position");
    _shaderSets[1]->AttributeTexCoordLocation = _shaderSets[1]->ShaderProgram->getAttributeLocation("a_texCoord");
    _shaderSets[1]->SamplerTexture0Location = _shaderSets[1]->ShaderProgram->getUniformLocation("s_texture0");
    _shaderSets[1]->UniformMatrixLocation = _shaderSets[1]->ShaderProgram->getUniformLocation("u_matrix");
    _shaderSets[1]->UniformBaseColorLocation = _shaderSets[1]->ShaderProgram->getUniformLocation("u_baseColor");
    _shaderSets[1]->UniformMultiplyColorLocation = _shaderSets[1]->ShaderProgram->getUniformLocation("u_multiplyColor");
    _shaderSets[1]->UniformScreenColorLocation = _shaderSets[1]->ShaderProgram->getUniformLocation("u_screenColor");

    // 通常（クリッピング）
    _shaderSets[2]->AttributePositionLocation = _shaderSets[2]->ShaderProgram->getAttributeLocation("a_position");
    _shaderSets[2]->AttributeTexCoordLocation = _shaderSets[2]->ShaderProgram->getAttributeLocation("a_texCoord");
    _shaderSets[2]->SamplerTexture0Location = _shaderSets[2]->ShaderProgram->getUniformLocation("s_texture0");
    _shaderSets[2]->SamplerTexture1Location = _shaderSets[2]->ShaderProgram->getUniformLocation("s_texture1");
    _shaderSets[2]->UniformMatrixLocation = _shaderSets[2]->ShaderProgram->getUniformLocation("u_matrix");
    _shaderSets[2]->UniformClipMatrixLocation = _shaderSets[2]->ShaderProgram->getUniformLocation("u_clipMatrix");
    _shaderSets[2]->UnifromChannelFlagLocation = _shaderSets[2]->ShaderProgram->getUniformLocation("u_channelFlag");
    _shaderSets[2]->UniformBaseColorLocation = _shaderSets[2]->ShaderProgram->getUniformLocation("u_baseColor");
    _shaderSets[2]->UniformMultiplyColorLocation = _shaderSets[2]->ShaderProgram->getUniformLocation("u_multiplyColor");
    _shaderSets[2]->UniformScreenColorLocation = _shaderSets[2]->ShaderProgram->getUniformLocation("u_screenColor");

    // 通常（クリッピング・反転）
    _shaderSets[3]->AttributePositionLocation = _shaderSets[3]->ShaderProgram->getAttributeLocation("a_position");
    _shaderSets[3]->AttributeTexCoordLocation = _shaderSets[3]->ShaderProgram->getAttributeLocation("a_texCoord");
    _shaderSets[3]->SamplerTexture0Location = _shaderSets[3]->ShaderProgram->getUniformLocation("s_texture0");
    _shaderSets[3]->SamplerTexture1Location = _shaderSets[3]->ShaderProgram->getUniformLocation("s_texture1");
    _shaderSets[3]->UniformMatrixLocation = _shaderSets[3]->ShaderProgram->getUniformLocation("u_matrix");
    _shaderSets[3]->UniformClipMatrixLocation = _shaderSets[3]->ShaderProgram->getUniformLocation("u_clipMatrix");
    _shaderSets[3]->UnifromChannelFlagLocation = _shaderSets[3]->ShaderProgram->getUniformLocation("u_channelFlag");
    _shaderSets[3]->UniformBaseColorLocation = _shaderSets[3]->ShaderProgram->getUniformLocation("u_baseColor");
    _shaderSets[3]->UniformMultiplyColorLocation = _shaderSets[3]->ShaderProgram->getUniformLocation("u_multiplyColor");
    _shaderSets[3]->UniformScreenColorLocation = _shaderSets[3]->ShaderProgram->getUniformLocation("u_screenColor");

    // 通常（PremultipliedAlpha）
    _shaderSets[4]->AttributePositionLocation = _shaderSets[4]->ShaderProgram->getAttributeLocation("a_position");
    _shaderSets[4]->AttributeTexCoordLocation = _shaderSets[4]->ShaderProgram->getAttributeLocation("a_texCoord");
    _shaderSets[4]->SamplerTexture0Location = _shaderSets[4]->ShaderProgram->getUniformLocation("s_texture0");
    _shaderSets[4]->UniformMatrixLocation = _shaderSets[4]->ShaderProgram->getUniformLocation("u_matrix");
    _shaderSets[4]->UniformBaseColorLocation = _shaderSets[4]->ShaderProgram->getUniformLocation("u_baseColor");
    _shaderSets[4]->UniformMultiplyColorLocation = _shaderSets[4]->ShaderProgram->getUniformLocation("u_multiplyColor");
    _shaderSets[4]->UniformScreenColorLocation = _shaderSets[4]->ShaderProgram->getUniformLocation("u_screenColor");

    // 通常（クリッピング、PremultipliedAlpha）
    _shaderSets[5]->AttributePositionLocation = _shaderSets[5]->ShaderProgram->getAttributeLocation("a_position");
    _shaderSets[5]->AttributeTexCoordLocation = _shaderSets[5]->ShaderProgram->getAttributeLocation("a_texCoord");
    _shaderSets[5]->SamplerTexture0Location = _shaderSets[5]->ShaderProgram->getUniformLocation("s_texture0");
    _shaderSets[5]->SamplerTexture1Location = _shaderSets[5]->ShaderProgram->getUniformLocation("s_texture1");
    _shaderSets[5]->UniformMatrixLocation = _shaderSets[5]->ShaderProgram->getUniformLocation("u_matrix");
    _shaderSets[5]->UniformClipMatrixLocation = _shaderSets[5]->ShaderProgram->getUniformLocation("u_clipMatrix");
    _shaderSets[5]->UnifromChannelFlagLocation = _shaderSets[5]->ShaderProgram->getUniformLocation("u_channelFlag");
    _shaderSets[5]->UniformBaseColorLocation = _shaderSets[5]->ShaderProgram->getUniformLocation("u_baseColor");
    _shaderSets[5]->UniformMultiplyColorLocation = _shaderSets[5]->ShaderProgram->getUniformLocation("u_multiplyColor");
    _shaderSets[5]->UniformScreenColorLocation = _shaderSets[5]->ShaderProgram->getUniformLocation("u_screenColor");

    // 通常（クリッピング・反転、PremultipliedAlpha）
    _shaderSets[6]->AttributePositionLocation = _shaderSets[6]->ShaderProgram->getAttributeLocation("a_position");
    _shaderSets[6]->AttributeTexCoordLocation = _shaderSets[6]->ShaderProgram->getAttributeLocation("a_texCoord");
    _shaderSets[6]->SamplerTexture0Location = _shaderSets[6]->ShaderProgram->getUniformLocation("s_texture0");
    _shaderSets[6]->SamplerTexture1Location = _shaderSets[6]->ShaderProgram->getUniformLocation("s_texture1");
    _shaderSets[6]->UniformMatrixLocation = _shaderSets[6]->ShaderProgram->getUniformLocation("u_matrix");
    _shaderSets[6]->UniformClipMatrixLocation = _shaderSets[6]->ShaderProgram->getUniformLocation("u_clipMatrix");
    _shaderSets[6]->UnifromChannelFlagLocation = _shaderSets[6]->ShaderProgram->getUniformLocation("u_channelFlag");
    _shaderSets[6]->UniformBaseColorLocation = _shaderSets[6]->ShaderProgram->getUniformLocation("u_baseColor");
    _shaderSets[6]->UniformMultiplyColorLocation = _shaderSets[6]->ShaderProgram->getUniformLocation("u_multiplyColor");
    _shaderSets[6]->UniformScreenColorLocation = _shaderSets[6]->ShaderProgram->getUniformLocation("u_screenColor");

    // 加算
    _shaderSets[7]->AttributePositionLocation = _shaderSets[7]->ShaderProgram->getAttributeLocation("a_position");
    _shaderSets[7]->AttributeTexCoordLocation = _shaderSets[7]->ShaderProgram->getAttributeLocation("a_texCoord");
    _shaderSets[7]->SamplerTexture0Location = _shaderSets[7]->ShaderProgram->getUniformLocation("s_texture0");
    _shaderSets[7]->UniformMatrixLocation = _shaderSets[7]->ShaderProgram->getUniformLocation("u_matrix");
    _shaderSets[7]->UniformBaseColorLocation = _shaderSets[7]->ShaderProgram->getUniformLocation("u_baseColor");
    _shaderSets[7]->UniformMultiplyColorLocation = _shaderSets[7]->ShaderProgram->getUniformLocation("u_multiplyColor");
    _shaderSets[7]->UniformScreenColorLocation = _shaderSets[7]->ShaderProgram->getUniformLocation("u_screenColor");

    // 加算（クリッピング）
    _shaderSets[8]->AttributePositionLocation = _shaderSets[8]->ShaderProgram->getAttributeLocation("a_position");
    _shaderSets[8]->AttributeTexCoordLocation = _shaderSets[8]->ShaderProgram->getAttributeLocation("a_texCoord");
    _shaderSets[8]->SamplerTexture0Location = _shaderSets[8]->ShaderProgram->getUniformLocation("s_texture0");
    _shaderSets[8]->SamplerTexture1Location = _shaderSets[8]->ShaderProgram->getUniformLocation("s_texture1");
    _shaderSets[8]->UniformMatrixLocation = _shaderSets[8]->ShaderProgram->getUniformLocation("u_matrix");
    _shaderSets[8]->UniformClipMatrixLocation = _shaderSets[8]->ShaderProgram->getUniformLocation("u_clipMatrix");
    _shaderSets[8]->UnifromChannelFlagLocation = _shaderSets[8]->ShaderProgram->getUniformLocation("u_channelFlag");
    _shaderSets[8]->UniformBaseColorLocation = _shaderSets[8]->ShaderProgram->getUniformLocation("u_baseColor");
    _shaderSets[8]->UniformMultiplyColorLocation = _shaderSets[8]->ShaderProgram->getUniformLocation("u_multiplyColor");
    _shaderSets[8]->UniformScreenColorLocation = _shaderSets[8]->ShaderProgram->getUniformLocation("u_screenColor");

    // 加算（クリッピング・反転）
    _shaderSets[9]->AttributePositionLocation = _shaderSets[9]->ShaderProgram->getAttributeLocation("a_position");
    _shaderSets[9]->AttributeTexCoordLocation = _shaderSets[9]->ShaderProgram->getAttributeLocation("a_texCoord");
    _shaderSets[9]->SamplerTexture0Location = _shaderSets[9]->ShaderProgram->getUniformLocation("s_texture0");
    _shaderSets[9]->SamplerTexture1Location = _shaderSets[9]->ShaderProgram->getUniformLocation("s_texture1");
    _shaderSets[9]->UniformMatrixLocation = _shaderSets[9]->ShaderProgram->getUniformLocation("u_matrix");
    _shaderSets[9]->UniformClipMatrixLocation = _shaderSets[9]->ShaderProgram->getUniformLocation("u_clipMatrix");
    _shaderSets[9]->UnifromChannelFlagLocation = _shaderSets[9]->ShaderProgram->getUniformLocation("u_channelFlag");
    _shaderSets[9]->UniformBaseColorLocation = _shaderSets[9]->ShaderProgram->getUniformLocation("u_baseColor");
    _shaderSets[9]->UniformMultiplyColorLocation = _shaderSets[9]->ShaderProgram->getUniformLocation("u_multiplyColor");
    _shaderSets[9]->UniformScreenColorLocation = _shaderSets[9]->ShaderProgram->getUniformLocation("u_screenColor");

    // 加算（PremultipliedAlpha）
    _shaderSets[10]->AttributePositionLocation = _shaderSets[10]->ShaderProgram->getAttributeLocation("a_position");
    _shaderSets[10]->AttributeTexCoordLocation = _shaderSets[10]->ShaderProgram->getAttributeLocation("a_texCoord");
    _shaderSets[10]->SamplerTexture0Location = _shaderSets[10]->ShaderProgram->getUniformLocation("s_texture0");
    _shaderSets[10]->UniformMatrixLocation = _shaderSets[10]->ShaderProgram->getUniformLocation("u_matrix");
    _shaderSets[10]->UniformBaseColorLocation = _shaderSets[10]->ShaderProgram->getUniformLocation("u_baseColor");
    _shaderSets[10]->UniformMultiplyColorLocation = _shaderSets[10]->ShaderProgram->getUniformLocation("u_multiplyColor");
    _shaderSets[10]->UniformScreenColorLocation = _shaderSets[10]->ShaderProgram->getUniformLocation("u_screenColor");

    // 加算（クリッピング、PremultipliedAlpha）
    _shaderSets[11]->AttributePositionLocation = _shaderSets[11]->ShaderProgram->getAttributeLocation("a_position");
    _shaderSets[11]->AttributeTexCoordLocation = _shaderSets[11]->ShaderProgram->getAttributeLocation("a_texCoord");
    _shaderSets[11]->SamplerTexture0Location = _shaderSets[11]->ShaderProgram->getUniformLocation("s_texture0");
    _shaderSets[11]->SamplerTexture1Location = _shaderSets[11]->ShaderProgram->getUniformLocation("s_texture1");
    _shaderSets[11]->UniformMatrixLocation = _shaderSets[11]->ShaderProgram->getUniformLocation("u_matrix");
    _shaderSets[11]->UniformClipMatrixLocation = _shaderSets[11]->ShaderProgram->getUniformLocation("u_clipMatrix");
    _shaderSets[11]->UnifromChannelFlagLocation = _shaderSets[11]->ShaderProgram->getUniformLocation("u_channelFlag");
    _shaderSets[11]->UniformBaseColorLocation = _shaderSets[11]->ShaderProgram->getUniformLocation("u_baseColor");
    _shaderSets[11]->UniformMultiplyColorLocation = _shaderSets[11]->ShaderProgram->getUniformLocation("u_multiplyColor");
    _shaderSets[11]->UniformScreenColorLocation = _shaderSets[11]->ShaderProgram->getUniformLocation("u_screenColor");

    // 加算（クリッピング・反転、PremultipliedAlpha）
    _shaderSets[12]->AttributePositionLocation = _shaderSets[12]->ShaderProgram->getAttributeLocation("a_position");
    _shaderSets[12]->AttributeTexCoordLocation = _shaderSets[12]->ShaderProgram->getAttributeLocation("a_texCoord");
    _shaderSets[12]->SamplerTexture0Location = _shaderSets[12]->ShaderProgram->getUniformLocation("s_texture0");
    _shaderSets[12]->SamplerTexture1Location = _shaderSets[12]->ShaderProgram->getUniformLocation("s_texture1");
    _shaderSets[12]->UniformMatrixLocation = _shaderSets[12]->ShaderProgram->getUniformLocation("u_matrix");
    _shaderSets[12]->UniformClipMatrixLocation = _shaderSets[12]->ShaderProgram->getUniformLocation("u_clipMatrix");
    _shaderSets[12]->UnifromChannelFlagLocation = _shaderSets[12]->ShaderProgram->getUniformLocation("u_channelFlag");
    _shaderSets[12]->UniformBaseColorLocation = _shaderSets[12]->ShaderProgram->getUniformLocation("u_baseColor");
    _shaderSets[12]->UniformMultiplyColorLocation = _shaderSets[12]->ShaderProgram->getUniformLocation("u_multiplyColor");
    _shaderSets[12]->UniformScreenColorLocation = _shaderSets[12]->ShaderProgram->getUniformLocation("u_screenColor");

    // 乗算
    _shaderSets[13]->AttributePositionLocation = _shaderSets[13]->ShaderProgram->getAttributeLocation("a_position");
    _shaderSets[13]->AttributeTexCoordLocation = _shaderSets[13]->ShaderProgram->getAttributeLocation("a_texCoord");
    _shaderSets[13]->SamplerTexture0Location = _shaderSets[13]->ShaderProgram->getUniformLocation("s_texture0");
    _shaderSets[13]->UniformMatrixLocation = _shaderSets[13]->ShaderProgram->getUniformLocation("u_matrix");
    _shaderSets[13]->UniformBaseColorLocation = _shaderSets[13]->ShaderProgram->getUniformLocation("u_baseColor");
    _shaderSets[13]->UniformMultiplyColorLocation = _shaderSets[13]->ShaderProgram->getUniformLocation("u_multiplyColor");
    _shaderSets[13]->UniformScreenColorLocation = _shaderSets[13]->ShaderProgram->getUniformLocation("u_screenColor");

    // 乗算（クリッピング）
    _shaderSets[14]->AttributePositionLocation = _shaderSets[14]->ShaderProgram->getAttributeLocation("a_position");
    _shaderSets[14]->AttributeTexCoordLocation = _shaderSets[14]->ShaderProgram->getAttributeLocation("a_texCoord");
    _shaderSets[14]->SamplerTexture0Location = _shaderSets[14]->ShaderProgram->getUniformLocation("s_texture0");
    _shaderSets[14]->SamplerTexture1Location = _shaderSets[14]->ShaderProgram->getUniformLocation("s_texture1");
    _shaderSets[14]->UniformMatrixLocation = _shaderSets[14]->ShaderProgram->getUniformLocation("u_matrix");
    _shaderSets[14]->UniformClipMatrixLocation = _shaderSets[14]->ShaderProgram->getUniformLocation("u_clipMatrix");
    _shaderSets[14]->UnifromChannelFlagLocation = _shaderSets[14]->ShaderProgram->getUniformLocation("u_channelFlag");
    _shaderSets[14]->UniformBaseColorLocation = _shaderSets[14]->ShaderProgram->getUniformLocation("u_baseColor");
    _shaderSets[14]->UniformMultiplyColorLocation = _shaderSets[14]->ShaderProgram->getUniformLocation("u_multiplyColor");
    _shaderSets[14]->UniformScreenColorLocation = _shaderSets[14]->ShaderProgram->getUniformLocation("u_screenColor");

    // 乗算（クリッピング・反転）
    _shaderSets[15]->AttributePositionLocation = _shaderSets[15]->ShaderProgram->getAttributeLocation("a_position");
    _shaderSets[15]->AttributeTexCoordLocation = _shaderSets[15]->ShaderProgram->getAttributeLocation("a_texCoord");
    _shaderSets[15]->SamplerTexture0Location = _shaderSets[15]->ShaderProgram->getUniformLocation("s_texture0");
    _shaderSets[15]->SamplerTexture1Location = _shaderSets[15]->ShaderProgram->getUniformLocation("s_texture1");
    _shaderSets[15]->UniformMatrixLocation = _shaderSets[15]->ShaderProgram->getUniformLocation("u_matrix");
    _shaderSets[15]->UniformClipMatrixLocation = _shaderSets[15]->ShaderProgram->getUniformLocation("u_clipMatrix");
    _shaderSets[15]->UnifromChannelFlagLocation = _shaderSets[15]->ShaderProgram->getUniformLocation("u_channelFlag");
    _shaderSets[15]->UniformBaseColorLocation = _shaderSets[15]->ShaderProgram->getUniformLocation("u_baseColor");
    _shaderSets[15]->UniformMultiplyColorLocation = _shaderSets[15]->ShaderProgram->getUniformLocation("u_multiplyColor");
    _shaderSets[15]->UniformScreenColorLocation = _shaderSets[15]->ShaderProgram->getUniformLocation("u_screenColor");

    // 乗算（PremultipliedAlpha）
    _shaderSets[16]->AttributePositionLocation = _shaderSets[16]->ShaderProgram->getAttributeLocation( "a_position");
    _shaderSets[16]->AttributeTexCoordLocation = _shaderSets[16]->ShaderProgram->getAttributeLocation("a_texCoord");
    _shaderSets[16]->SamplerTexture0Location = _shaderSets[16]->ShaderProgram->getUniformLocation("s_texture0");
    _shaderSets[16]->UniformMatrixLocation = _shaderSets[16]->ShaderProgram->getUniformLocation("u_matrix");
    _shaderSets[16]->UniformBaseColorLocation = _shaderSets[16]->ShaderProgram->getUniformLocation("u_baseColor");
    _shaderSets[16]->UniformMultiplyColorLocation = _shaderSets[16]->ShaderProgram->getUniformLocation("u_multiplyColor");
    _shaderSets[16]->UniformScreenColorLocation = _shaderSets[16]->ShaderProgram->getUniformLocation("u_screenColor");

    // 乗算（クリッピング、PremultipliedAlpha）
    _shaderSets[17]->AttributePositionLocation = _shaderSets[17]->ShaderProgram->getAttributeLocation("a_position");
    _shaderSets[17]->AttributeTexCoordLocation = _shaderSets[17]->ShaderProgram->getAttributeLocation("a_texCoord");
    _shaderSets[17]->SamplerTexture0Location = _shaderSets[17]->ShaderProgram->getUniformLocation("s_texture0");
    _shaderSets[17]->SamplerTexture1Location = _shaderSets[17]->ShaderProgram->getUniformLocation("s_texture1");
    _shaderSets[17]->UniformMatrixLocation = _shaderSets[17]->ShaderProgram->getUniformLocation("u_matrix");
    _shaderSets[17]->UniformClipMatrixLocation = _shaderSets[17]->ShaderProgram->getUniformLocation("u_clipMatrix");
    _shaderSets[17]->UnifromChannelFlagLocation = _shaderSets[17]->ShaderProgram->getUniformLocation("u_channelFlag");
    _shaderSets[17]->UniformBaseColorLocation = _shaderSets[17]->ShaderProgram->getUniformLocation("u_baseColor");
    _shaderSets[17]->UniformMultiplyColorLocation = _shaderSets[17]->ShaderProgram->getUniformLocation("u_multiplyColor");
    _shaderSets[17]->UniformScreenColorLocation = _shaderSets[17]->ShaderProgram->getUniformLocation("u_screenColor");

    // 乗算（クリッピング・反転、PremultipliedAlpha）
    _shaderSets[18]->AttributePositionLocation = _shaderSets[18]->ShaderProgram->getAttributeLocation("a_position");
    _shaderSets[18]->AttributeTexCoordLocation = _shaderSets[18]->ShaderProgram->getAttributeLocation("a_texCoord");
    _shaderSets[18]->SamplerTexture0Location = _shaderSets[18]->ShaderProgram->getUniformLocation("s_texture0");
    _shaderSets[18]->SamplerTexture1Location = _shaderSets[18]->ShaderProgram->getUniformLocation("s_texture1");
    _shaderSets[18]->UniformMatrixLocation = _shaderSets[18]->ShaderProgram->getUniformLocation("u_matrix");
    _shaderSets[18]->UniformClipMatrixLocation = _shaderSets[18]->ShaderProgram->getUniformLocation("u_clipMatrix");
    _shaderSets[18]->UnifromChannelFlagLocation = _shaderSets[18]->ShaderProgram->getUniformLocation("u_channelFlag");
    _shaderSets[18]->UniformBaseColorLocation = _shaderSets[18]->ShaderProgram->getUniformLocation("u_baseColor");
    _shaderSets[18]->UniformMultiplyColorLocation = _shaderSets[18]->ShaderProgram->getUniformLocation("u_multiplyColor");
    _shaderSets[18]->UniformScreenColorLocation = _shaderSets[18]->ShaderProgram->getUniformLocation("u_screenColor");
}

void CubismShader_Cocos2dx::SetupShaderProgramForMask(CubismCommandBuffer_Cocos2dx::DrawCommandBuffer::DrawCommand* drawCommand, CubismRenderer_Cocos2dx* renderer
                                                    ,const CubismModel& model, const csmInt32 index)
{
    if (_shaderSets.GetSize() == 0)
    {
        GenerateShaders();
    }

    CubismShaderSet* shaderSet = _shaderSets[ShaderNames_SetupMask];

    cocos2d::backend::BlendDescriptor* blendDescriptor = drawCommand->GetBlendDescriptor();
    blendDescriptor->sourceRGBBlendFactor = cocos2d::backend::BlendFactor::ZERO;
    blendDescriptor->destinationRGBBlendFactor = cocos2d::backend::BlendFactor::ONE_MINUS_SRC_COLOR;
    blendDescriptor->sourceAlphaBlendFactor = cocos2d::backend::BlendFactor::ZERO;
    blendDescriptor->destinationAlphaBlendFactor = cocos2d::backend::BlendFactor::ONE_MINUS_SRC_ALPHA;

    cocos2d::PipelineDescriptor* pipelineDescriptor = drawCommand->GetPipelineDescriptor();
    cocos2d::backend::ProgramState* programState = pipelineDescriptor->programState;
    if (!programState)
    {
        programState = new cocos2d::backend::ProgramState(shaderSet->ShaderProgram);
    }

    //テクスチャ設定
    SetupTexture(renderer, programState, model, index, shaderSet);

    // 頂点属性の設定
    SetVertexAttributes(programState, shaderSet);

    // カラーチャンネル
    CubismClippingContext_Cocos2dx* contextBuffer = renderer->GetClippingContextBufferForMask();
    SetColorChannel(renderer, programState, shaderSet, contextBuffer);

    //マスク用行列
    programState->setUniform(shaderSet->UniformClipMatrixLocation,
                             renderer->GetClippingContextBufferForMask()->_matrixForMask.GetArray(),
                             sizeof(float) * 16);

    // ユニフォーム変数設定
    csmRectF* rect = renderer->GetClippingContextBufferForMask()->_layoutBounds;
    CubismRenderer::CubismTextureColor baseColor = {rect->X * 2.0f - 1.0f, rect->Y * 2.0f - 1.0f, rect->GetRight() * 2.0f - 1.0f, rect->GetBottom() * 2.0f - 1.0f};
    CubismRenderer::CubismTextureColor multiplyColor = model.GetMultiplyColor(index);
    CubismRenderer::CubismTextureColor screenColor = model.GetScreenColor(index);
    SetColorUniformVariables(programState, shaderSet, baseColor, multiplyColor, screenColor);

    programState->getVertexLayout()->setLayout(sizeof(csmFloat32) * 4);
    blendDescriptor->blendEnabled = true;
    pipelineDescriptor->programState = programState;
}

void CubismShader_Cocos2dx::SetupShaderProgramForDraw(CubismCommandBuffer_Cocos2dx::DrawCommandBuffer::DrawCommand* drawCommand
                                                    , CubismRenderer_Cocos2dx* renderer, const CubismModel& model, const csmInt32 index)
{
    if (_shaderSets.GetSize() == 0)
    {
        GenerateShaders();
    }

    // _shaderSets用offsetの計算
    const csmBool masked = renderer->GetClippingContextBufferForDraw() != NULL;  // この描画オブジェクトはマスク対象か
    const csmBool invertedMask = model.GetDrawableInvertedMask(index);
    const csmBool isPremultipliedAlpha = renderer->IsPremultipliedAlpha();
    const csmInt32 offset = (masked ? ( invertedMask ? 2 : 1 ) : 0) + (isPremultipliedAlpha ? 3 : 0);

    CubismShaderSet* shaderSet;
    cocos2d::backend::BlendDescriptor* blendDescriptor = drawCommand->GetBlendDescriptor();
    switch (model.GetDrawableBlendMode(index))
    {
    case CubismRenderer::CubismBlendMode_Normal:
    default:
        shaderSet = _shaderSets[ShaderNames_Normal + offset];
        blendDescriptor->sourceRGBBlendFactor = cocos2d::backend::BlendFactor::ONE;
        blendDescriptor->destinationRGBBlendFactor = cocos2d::backend::BlendFactor::ONE_MINUS_SRC_ALPHA;
        blendDescriptor->sourceAlphaBlendFactor = cocos2d::backend::BlendFactor::ONE;
        blendDescriptor->destinationAlphaBlendFactor = cocos2d::backend::BlendFactor::ONE_MINUS_SRC_ALPHA;
        break;

    case CubismRenderer::CubismBlendMode_Additive:
        shaderSet = _shaderSets[ShaderNames_Add + offset];
        blendDescriptor->sourceRGBBlendFactor = cocos2d::backend::BlendFactor::ONE;
        blendDescriptor->destinationRGBBlendFactor = cocos2d::backend::BlendFactor::ONE;
        blendDescriptor->sourceAlphaBlendFactor = cocos2d::backend::BlendFactor::ZERO;
        blendDescriptor->destinationAlphaBlendFactor = cocos2d::backend::BlendFactor::ONE;
        break;

    case CubismRenderer::CubismBlendMode_Multiplicative:
        shaderSet = _shaderSets[ShaderNames_Mult + offset];
        blendDescriptor->sourceRGBBlendFactor = cocos2d::backend::BlendFactor::DST_COLOR;
        blendDescriptor->destinationRGBBlendFactor = cocos2d::backend::BlendFactor::ONE_MINUS_SRC_ALPHA;
        blendDescriptor->sourceAlphaBlendFactor = cocos2d::backend::BlendFactor::ZERO;
        blendDescriptor->destinationAlphaBlendFactor = cocos2d::backend::BlendFactor::ONE;
        break;
    }

    cocos2d::PipelineDescriptor* pipelineDescriptor = drawCommand->GetPipelineDescriptor();
    cocos2d::backend::ProgramState* programState = pipelineDescriptor->programState;
    if (!programState)
    {
        programState = new cocos2d::backend::ProgramState(shaderSet->ShaderProgram);
    }

    //テクスチャ設定
    SetupTexture(renderer, programState, model, index, shaderSet);

    // 頂点属性の設定
    SetVertexAttributes(programState, shaderSet);

    if (masked)
    {
        // frameBufferに書かれたテクスチャ
        cocos2d::Texture2D* tex = renderer->GetOffscreenSurface(renderer->GetClippingContextBufferForDraw()->_bufferIndex)->GetColorBuffer();

        programState->setTexture(shaderSet->SamplerTexture1Location, 1, tex->getBackendTexture());

        // View座標をClippingContextの座標に変換するための行列を設定
        programState->setUniform(shaderSet->UniformClipMatrixLocation,
                                 renderer->GetClippingContextBufferForDraw()->_matrixForDraw.GetArray(),
                                 sizeof(float) * 16);

        // カラーチャンネル
        CubismClippingContext_Cocos2dx* contextBuffer = renderer->GetClippingContextBufferForDraw();
        SetColorChannel(renderer, programState, shaderSet, contextBuffer);
    }

    //座標変換
    CubismMatrix44 matrix4x4 = renderer->GetMvpMatrix();
    programState->setUniform(shaderSet->UniformMatrixLocation, matrix4x4.GetArray(), sizeof(float) * 16);

    // ユニフォーム変数設定
    CubismRenderer::CubismTextureColor baseColor = renderer->GetModelColorWithOpacity(model.GetDrawableOpacity(index));
    CubismRenderer::CubismTextureColor multiplyColor = model.GetMultiplyColor(index);
    CubismRenderer::CubismTextureColor screenColor = model.GetScreenColor(index);
    SetColorUniformVariables(programState, shaderSet, baseColor, multiplyColor, screenColor);

    programState->getVertexLayout()->setLayout(sizeof(csmFloat32) * 4);
    blendDescriptor->blendEnabled = true;
    pipelineDescriptor->programState = programState;
}

cocos2d::backend::Program* CubismShader_Cocos2dx::LoadShaderProgram(const csmChar* vertShaderSrc, const csmChar* fragShaderSrc)
{
    // cocos2dx対応
    // Create shader program.
    return cocos2d::backend::Device::getInstance()->newProgram(vertShaderSrc, fragShaderSrc);
}

void CubismShader_Cocos2dx::SetVertexAttributes(cocos2d::backend::ProgramState* programState, CubismShaderSet* shaderSet)
{
    // 頂点配列の設定
    programState->getVertexLayout()->setAttribute("a_position", shaderSet->AttributePositionLocation, cocos2d::backend::VertexFormat::FLOAT2, 0, false);
    // テクスチャ頂点の設定
    programState->getVertexLayout()->setAttribute("a_texCoord", shaderSet->AttributeTexCoordLocation, cocos2d::backend::VertexFormat::FLOAT2, sizeof(csmFloat32) * 2, false);
}

void CubismShader_Cocos2dx::SetupTexture(CubismRenderer_Cocos2dx* renderer, cocos2d::backend::ProgramState* programState
                                        , const CubismModel& model, const csmInt32 index, CubismShaderSet* shaderSet)
{
    const csmInt32 textureIndex = model.GetDrawableTextureIndex(index);
    cocos2d::Texture2D* texture = renderer->GetBindedTexture(textureIndex);
    programState->setTexture(shaderSet->SamplerTexture0Location, 0, texture->getBackendTexture());
}

void CubismShader_Cocos2dx::SetColorUniformVariables(cocos2d::backend::ProgramState* programState, CubismShaderSet* shaderSet, CubismRenderer::CubismTextureColor& baseColor,
                                                     CubismRenderer::CubismTextureColor& multiplyColor, CubismRenderer::CubismTextureColor& screenColor)
{
    csmFloat32 base[4] = { baseColor.R, baseColor.G, baseColor.B, baseColor.A };
    csmFloat32 multiply[4] = { multiplyColor.R, multiplyColor.G, multiplyColor.B, multiplyColor.A };
    csmFloat32 screen[4] = { screenColor.R, screenColor.G, screenColor.B, screenColor.A };

    programState->setUniform(shaderSet->UniformBaseColorLocation, base, sizeof(float) * 4);
    programState->setUniform(shaderSet->UniformMultiplyColorLocation, multiply, sizeof(float) * 4);
    programState->setUniform(shaderSet->UniformScreenColorLocation, screen, sizeof(float) * 4);
}

void CubismShader_Cocos2dx::SetColorChannel(CubismRenderer_Cocos2dx* renderer, cocos2d::backend::ProgramState* programState,
                                            CubismShaderSet* shaderSet, CubismClippingContext_Cocos2dx* contextBuffer)
{
    const csmInt32 channelIndex = contextBuffer->_layoutChannelIndex;
    CubismRenderer::CubismTextureColor* colorChannel = contextBuffer->GetClippingManager()->GetChannelFlagAsColor(channelIndex);
    csmFloat32 colorFlag[4] = { colorChannel->R, colorChannel->G, colorChannel->B, colorChannel->A };
    programState->setUniform(shaderSet->UnifromChannelFlagLocation, colorFlag, sizeof(float) * 4);
}

}}}}
//------------ LIVE2D NAMESPACE ------------
