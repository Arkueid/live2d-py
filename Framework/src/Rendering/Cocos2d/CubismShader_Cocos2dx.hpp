/**
 * Copyright(c) Live2D Inc. All rights reserved.
 *
 * Use of this source code is governed by the Live2D Open Software license
 * that can be found at https://www.live2d.com/eula/live2d-open-software-license-agreement_en.html.
 */

#pragma once

#include "CubismFramework.hpp"
#include "CubismCommandBuffer_Cocos2dx.hpp"
#include "CubismRenderer_Cocos2dx.hpp"
#include "Type/csmVector.hpp"

#ifdef CSM_TARGET_ANDROID_ES2
#include <jni.h>
#include <errno.h>
#include <GLES2/gl2.h>
#include <GLES2/gl2ext.h>
#endif

#ifdef CSM_TARGET_IPHONE_ES2
#include <OpenGLES/ES2/gl.h>
#include <OpenGLES/ES2/glext.h>
#endif

#if defined(CSM_TARGET_WIN_GL) || defined(CSM_TARGET_LINUX_GL)
#include <GL/glew.h>
#include <GL/gl.h>
#endif

#ifdef CSM_TARGET_MAC_GL
#ifndef CSM_TARGET_COCOS
#include <GL/glew.h>
#endif
#include <OpenGL/gl.h>
#endif

//------------ LIVE2D NAMESPACE ------------
namespace Live2D { namespace Cubism { namespace Framework { namespace Rendering {

// 前方宣言
class CubismRenderer_Cocos2dx;
class CubismClippingContext_Cocos2dx;

/**
 * @brief   Cocos2dx用のシェーダプログラムを生成・破棄するクラス<br>
 *           シングルトンなクラスであり、CubismShader_Cocos2dx::GetInstance()からアクセスする。
 */
class CubismShader_Cocos2dx
{
public:
    /**
     * @brief   インスタンスを取得する（シングルトン）。
     *
     * @return  インスタンスのポインタ
     */
    static CubismShader_Cocos2dx* GetInstance();

    /**
     * @brief   インスタンスを解放する（シングルトン）。
     */
    static void DeleteInstance();

    /**
     * @brief   マスク用のシェーダプログラムの一連のセットアップを実行する
     *
     * @param[in]   drawCommand            ->  コマンドバッファ
     * @param[in]   renderer               ->  レンダラー
     * @param[in]   model                  ->  描画対象のモデル
     * @param[in]   index                  ->  描画対象のメッシュのインデックス
     */
    void SetupShaderProgramForMask(CubismCommandBuffer_Cocos2dx::DrawCommandBuffer::DrawCommand* drawCommand, CubismRenderer_Cocos2dx* renderer
                                ,const CubismModel& model, const csmInt32 index);

    /**
     * @brief   描画用のシェーダプログラムの一連のセットアップを実行する
     *
     * @param[in]   drawCommand            ->  コマンドバッファ
     * @param[in]   renderer               ->  レンダラー
     * @param[in]   model                  ->  描画対象のモデル
     * @param[in]   index                  ->  描画対象のメッシュのインデックス
     */
    void SetupShaderProgramForDraw(CubismCommandBuffer_Cocos2dx::DrawCommandBuffer::DrawCommand* drawCommand, CubismRenderer_Cocos2dx* renderer
                                ,const CubismModel& model, const csmInt32 index);

private:
    /**
    * @bref    シェーダープログラムとシェーダ変数のアドレスを保持する構造体
    *
    */
    struct CubismShaderSet
    {
        cocos2d::backend::Program* ShaderProgram;               ///< シェーダプログラムのアドレス
        unsigned int AttributePositionLocation;   ///< シェーダプログラムに渡す変数のアドレス(Position)
        unsigned int AttributeTexCoordLocation;   ///< シェーダプログラムに渡す変数のアドレス(TexCoord)
        cocos2d::backend::UniformLocation UniformMatrixLocation;        ///< シェーダプログラムに渡す変数のアドレス(Matrix)
        cocos2d::backend::UniformLocation UniformClipMatrixLocation;    ///< シェーダプログラムに渡す変数のアドレス(ClipMatrix)
        cocos2d::backend::UniformLocation SamplerTexture0Location;      ///< シェーダプログラムに渡す変数のアドレス(Texture0)
        cocos2d::backend::UniformLocation SamplerTexture1Location;      ///< シェーダプログラムに渡す変数のアドレス(Texture1)
        cocos2d::backend::UniformLocation UniformBaseColorLocation;     ///< シェーダプログラムに渡す変数のアドレス(BaseColor)
        cocos2d::backend::UniformLocation UniformMultiplyColorLocation;     ///< シェーダプログラムに渡す変数のアドレス(MultiplyColor)
        cocos2d::backend::UniformLocation UniformScreenColorLocation;     ///< シェーダプログラムに渡す変数のアドレス(ScreenColor)
        cocos2d::backend::UniformLocation UnifromChannelFlagLocation;   ///< シェーダプログラムに渡す変数のアドレス(ChannelFlag)
    };

    /**
     * @brief   privateなコンストラクタ
     */
    CubismShader_Cocos2dx();

    /**
     * @brief   privateなデストラクタ
     */
    virtual ~CubismShader_Cocos2dx();

    /**
     * @brief   シェーダプログラムを解放する
     */
    void ReleaseShaderProgram();

    /**
     * @brief   シェーダプログラムを初期化する
     */
    void GenerateShaders();

    /**
     * @brief   シェーダプログラムをロードしてアドレス返す。
     *
     * @param[in]   vertShaderSrc   ->  頂点シェーダのソース
     * @param[in]   fragShaderSrc   ->  フラグメントシェーダのソース
     *
     * @return  シェーダプログラムのアドレス
     */
    cocos2d::backend::Program* LoadShaderProgram(const csmChar* vertShaderSrc, const csmChar* fragShaderSrc);

    /**
     * @brief   必要な頂点属性を設定する
     *
     * @param[in]   programState          ->  cocos2dプログラムステート
     * @param[in]   shaderSet             ->  シェーダープログラムのセット
     */
    void SetVertexAttributes(cocos2d::backend::ProgramState* programState, CubismShaderSet* shaderSet);

    /**
     * @brief   テクスチャの設定を行う
     *
     * @param[in]   renderer               ->  レンダラー
     * @param[in]   programState           ->  cocos2dプログラムステート
     * @param[in]   model                  ->  描画対象のモデル
     * @param[in]   index                  ->  描画対象のメッシュのインデックス
     * @param[in]   shaderSet              ->  シェーダープログラムのセット
     */
    void SetupTexture(CubismRenderer_Cocos2dx* renderer, cocos2d::backend::ProgramState* programState
                    , const CubismModel& model, const csmInt32 index, CubismShaderSet* shaderSet);

    /**
     * @brief   色関連のユニフォーム変数の設定を行う
     *
     * @param[in]   programState          ->  cocos2dプログラムステート
     * @param[in]   shaderSet             ->  シェーダープログラムのセット
     * @param[in]   baseColor             ->  ベースカラー
     * @param[in]   multiplyColor         ->  乗算カラー
     * @param[in]   screenColor           ->  スクリーンカラー
     */
    void SetColorUniformVariables(cocos2d::backend::ProgramState* programState, CubismShaderSet* shaderSet, CubismRenderer::CubismTextureColor& baseColor
                                , CubismRenderer::CubismTextureColor& multiplyColor, CubismRenderer::CubismTextureColor& screenColor);

    /**
     * @brief   カラーチャンネル関連のユニフォーム変数の設定を行う
     *
     * @param[in]   renderer              ->  レンダラー
     * @param[in]   programState          ->  cocos2dプログラムステート
     * @param[in]   shaderSet             ->  シェーダープログラムのセット
     * @param[in]   contextBuffer         ->  描画コンテキスト
     */
    void SetColorChannel(CubismRenderer_Cocos2dx* renderer, cocos2d::backend::ProgramState* programState,
                         CubismShaderSet* shaderSet, CubismClippingContext_Cocos2dx* contextBuffer);

#ifdef CSM_TARGET_ANDROID_ES2
public:
    /**
     * @brief   Tegraプロセッサ対応。拡張方式による描画の有効・無効
     *
     * @param[in]   extMode     ->  trueなら拡張方式で描画する
     * @param[in]   extPAMode   ->  trueなら拡張方式のPA設定を有効にする
     */
    static void SetExtShaderMode(csmBool extMode, csmBool extPAMode);

private:
    static csmBool  s_extMode;      ///< Tegra対応.拡張方式で描画
    static csmBool  s_extPAMode;    ///< 拡張方式のPA設定用の変数
#endif

    csmVector<CubismShaderSet*> _shaderSets;   ///< ロードしたシェーダプログラムを保持する変数
};

}}}}
//------------ LIVE2D NAMESPACE ------------
