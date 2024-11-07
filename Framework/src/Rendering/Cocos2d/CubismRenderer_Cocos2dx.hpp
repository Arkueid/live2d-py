/**
 * Copyright(c) Live2D Inc. All rights reserved.
 *
 * Use of this source code is governed by the Live2D Open Software license
 * that can be found at https://www.live2d.com/eula/live2d-open-software-license-agreement_en.html.
 */

#pragma once

#include "../CubismRenderer.hpp"
#include "../CubismClippingManager.hpp"
#include "CubismFramework.hpp"
#include "CubismOffscreenSurface_Cocos2dx.hpp"
#include "CubismCommandBuffer_Cocos2dx.hpp"
#include "CubismShader_Cocos2dx.hpp"
#include "Math/CubismVector2.hpp"
#include "Type/csmVector.hpp"
#include "Type/csmRectF.hpp"
#include "Type/csmMap.hpp"


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

//  前方宣言
class CubismRenderer_Cocos2dx;
class CubismClippingContext_Cocos2dx;
class CubismShader_Cocos2dx;

/**
 * @brief  クリッピングマスクの処理を実行するクラス
 *
 */
class CubismClippingManager_Cocos2dx : public CubismClippingManager<CubismClippingContext_Cocos2dx, CubismOffscreenSurface_Cocos2dx>
{
public:

    /**
     * @brief   クリッピングコンテキストを作成する。モデル描画時に実行する。
     *
     * @param[in]   model        ->  モデルのインスタンス
     * @param[in]   renderer     ->  レンダラのインスタンス
     * @param[in]   lastFBO      ->  フレームバッファ
     * @param[in]   lastViewport ->  ビューポート
     */
    void SetupClippingContext(CubismModel& model, CubismRenderer_Cocos2dx* renderer, cocos2d::Texture2D* lastColorBuffer, csmRectF lastViewport);
};

/**
 * @brief   クリッピングマスクのコンテキスト
 */
class CubismClippingContext_Cocos2dx : public CubismClippingContext
{
    friend class CubismClippingManager_Cocos2dx;
    friend class CubismRenderer_Cocos2dx;

public:
    /**
     * @brief   引数付きコンストラクタ
     *
     */
    CubismClippingContext_Cocos2dx(CubismClippingManager<CubismClippingContext_Cocos2dx, CubismOffscreenSurface_Cocos2dx>* manager, CubismModel& model, const csmInt32* clippingDrawableIndices, csmInt32 clipCount);

    /**
     * @brief   デストラクタ
     */
    virtual ~CubismClippingContext_Cocos2dx();

    /**
     * @brief   このマスクを管理するマネージャのインスタンスを取得する。
     *
     * @return  クリッピングマネージャのインスタンス
     */
    CubismClippingManager<CubismClippingContext_Cocos2dx, CubismOffscreenSurface_Cocos2dx>* GetClippingManager();

private:
    csmVector<CubismCommandBuffer_Cocos2dx::DrawCommandBuffer*>* _clippingCommandBufferList;
    CubismClippingManager<CubismClippingContext_Cocos2dx, CubismOffscreenSurface_Cocos2dx>* _owner;        ///< このマスクを管理しているマネージャのインスタンス
};

/**
 * @brief   Cubismモデルを描画する直前のCocos2dxのステートを保持・復帰させるクラス
 *
 */
class CubismRendererProfile_Cocos2dx
{
    friend class CubismRenderer_Cocos2dx;

private:
    /**
     * @biref   privateなコンストラクタ
     */
    CubismRendererProfile_Cocos2dx() {};

    /**
     * @biref   privateなデストラクタ
     */
    virtual ~CubismRendererProfile_Cocos2dx() {};

    /**
     * @brief   Cocos2dxのステートを保持する
     */
    void Save();

    /**
     * @brief   保持したCocos2dxのステートを復帰させる
     *
     */
    void Restore();

    csmBool _lastScissorTest;             ///< モデル描画直前のGL_VERTEX_ATTRIB_ARRAY_ENABLEDパラメータ
    csmBool _lastBlend;                   ///< モデル描画直前のGL_SCISSOR_TESTパラメータ
    csmBool _lastStencilTest;             ///< モデル描画直前のGL_STENCIL_TESTパラメータ
    csmBool _lastDepthTest;               ///< モデル描画直前のGL_DEPTH_TESTパラメータ
    cocos2d::CullMode _lastCullFace;                ///< モデル描画直前のGL_CULL_FACEパラメータ
    cocos2d::Winding _lastWinding;
    cocos2d::Texture2D* _lastColorBuffer;                         ///< モデル描画直前のフレームバッファ
    cocos2d::Texture2D* _lastDepthBuffer;
    cocos2d::Texture2D* _lastStencilBuffer;
    cocos2d::RenderTargetFlag _lastRenderTargetFlag;
    csmRectF _lastViewport;                 ///< モデル描画直前のビューポート
};

/**
 * @brief   Cocos2dx用の描画命令を実装したクラス
 *
 */
class CubismRenderer_Cocos2dx : public CubismRenderer
{
    friend class CubismRenderer;
    friend class CubismClippingManager_Cocos2dx;
    friend class CubismShader_Cocos2dx;

public:
    /**
     * @brief    レンダラの初期化処理を実行する<br>
     *           引数に渡したモデルからレンダラの初期化処理に必要な情報を取り出すことができる
     *
     * @param[in]  model -> モデルのインスタンス
     */
    void Initialize(Framework::CubismModel* model) override;

    void Initialize(Framework::CubismModel* model, csmInt32 maskBufferCount) override;

    /**
     * @brief   OpenGLテクスチャのバインド処理<br>
     *           CubismRendererにテクスチャを設定し、CubismRenderer中でその画像を参照するためのIndex値を戻り値とする
     *
     * @param[in]   modelTextureIndex  ->  セットするモデルテクスチャの番号
     * @param[in]   texture     ->  バックエンドテクスチャ
     *
     */
    void BindTexture(csmUint32 modelTextureIndex, cocos2d::Texture2D* texture);

    /**
     * @brief   OpenGLにバインドされたテクスチャのリストを取得する
     *
     * @return  テクスチャのアドレスのリスト
     */
    const csmMap<csmInt32, cocos2d::Texture2D*>& GetBindedTextures() const;

    /**
     * @brief  クリッピングマスクバッファのサイズを設定する<br>
     *         マスク用のOffscreenSurfaceを破棄・再作成するため処理コストは高い。
     *
     * @param[in]  width -> クリッピングマスクバッファの横サイズ
     * @param[in]  height -> クリッピングマスクバッファの縦サイズ
     *
     */
    void SetClippingMaskBufferSize(csmFloat32 width, csmFloat32 height);

    /**
     * @brief  レンダーテクスチャの枚数を取得する。
     *
     * @return  レンダーテクスチャの枚数
     *
     */
    csmInt32 GetRenderTextureCount() const;

    /**
     * @brief  クリッピングマスクバッファのサイズを取得する
     *
     * @return クリッピングマスクバッファのサイズ
     *
     */
    CubismVector2 GetClippingMaskBufferSize() const;

    /**
     * @brief  オフスクリーンサーフェスを取得する
     *
     * @return オフスクリーンサーフェスへの参照
     *
     */
    CubismOffscreenSurface_Cocos2dx* GetOffscreenSurface(csmInt32 index);

    static CubismCommandBuffer_Cocos2dx* GetCommandBuffer();

    static void StartFrame(CubismCommandBuffer_Cocos2dx* commandBuffer);

    static void EndFrame(CubismCommandBuffer_Cocos2dx* commandBuffer);

protected:
    /**
     * @brief   コンストラクタ
     */
    CubismRenderer_Cocos2dx();

    /**
     * @brief   デストラクタ
     */
    virtual ~CubismRenderer_Cocos2dx();

    /**
     * @brief   モデルを描画する実際の処理
     *
     */
    void DoDrawModel() override;

    /**
     * @brief   描画オブジェクト（アートメッシュ）を描画する。<br>
     *           描画するモデルと、描画対象のインデックスを渡す。
     *
     * @param[in]   drawCommand     ->  コマンドバッファ
     * @param[in]   model           ->  描画対象のモデル
     * @param[in]   index           ->  描画すべき対象のインデックス
     *
     */
    void DrawMeshCocos2d(CubismCommandBuffer_Cocos2dx::DrawCommandBuffer::DrawCommand* drawCommand
                        ,const CubismModel& model, const csmInt32 index);

    CubismCommandBuffer_Cocos2dx::DrawCommandBuffer* GetDrawCommandBufferData(csmInt32 drawableIndex);

#ifdef CSM_TARGET_ANDROID_ES2
public:
    /**
     * @brief   Tegraプロセッサ対応。拡張方式による描画の有効・無効
     *
     * @param[in]   extMode     ->  trueなら拡張方式で描画する
     * @param[in]   extPAMode   ->  trueなら拡張方式のPA設定を有効にする
     */
    static void SetExtShaderMode(csmBool extMdoe, csmBool extPAMode = false);

    /**
     * @brief   Android-Tegra対応. シェーダプログラムをリロードする。
     */
    static void ReloadShader();
#endif

private:
    // Prevention of copy Constructor
    CubismRenderer_Cocos2dx(const CubismRenderer_Cocos2dx&);
    CubismRenderer_Cocos2dx& operator=(const CubismRenderer_Cocos2dx&);

    /**
     * @brief   レンダラが保持する静的なリソースを解放する<br>
     *           Cocos2dxの静的なシェーダプログラムを解放する
     */
    static void DoStaticRelease();

    /**
     * @brief   描画開始時の追加処理。<br>
     *           モデルを描画する前にクリッピングマスクに必要な処理を実装している。
     */
    void PreDraw();

    /**
     * @brief   描画完了後の追加処理。
     *
     */
    void PostDraw(){};

    /**
     * @brief   モデル描画直前のCocos2dxのステートを保持する
     */
    void SaveProfile() override;

    /**
     * @brief   モデル描画直前のCocos2dxのステートを保持する
     */
    void RestoreProfile() override;

    /**
     * @brief   マスクテクスチャに描画するクリッピングコンテキストをセットする。
     */
    void SetClippingContextBufferForMask(CubismClippingContext_Cocos2dx* clip);

    /**
     * @brief   マスクテクスチャに描画するクリッピングコンテキストを取得する。
     *
     * @return  マスクテクスチャに描画するクリッピングコンテキスト
     */
    CubismClippingContext_Cocos2dx* GetClippingContextBufferForMask() const;

    /**
     * @brief   画面上に描画するクリッピングコンテキストをセットする。
     */
    void SetClippingContextBufferForDraw(CubismClippingContext_Cocos2dx* clip);

    /**
     * @brief   画面上に描画するクリッピングコンテキストを取得する。
     *
     * @return  画面上に描画するクリッピングコンテキスト
     */
    CubismClippingContext_Cocos2dx* GetClippingContextBufferForDraw() const;

    /**
     * @brief   マスク生成時かを判定する
     *
     * @return  判定値
     */
    const csmBool inline IsGeneratingMask() const;

    /**
     * @brief   テクスチャマップからバインド済みテクスチャIDを取得する
     *          バインドされていなければNULLを返却される
     *
     * @return  バインドされたテクスチャ
     */
    cocos2d::Texture2D* GetBindedTexture(csmInt32 textureIndex);


    csmMap<csmInt32, cocos2d::Texture2D*> _textures;                      ///< モデルが参照するテクスチャとレンダラでバインドしているテクスチャとのマップ
    csmVector<csmInt32> _sortedDrawableIndexList;       ///< 描画オブジェクトのインデックスを描画順に並べたリスト
    CubismRendererProfile_Cocos2dx _rendererProfile;               ///< OpenGLのステートを保持するオブジェクト
    CubismClippingManager_Cocos2dx* _clippingManager;               ///< クリッピングマスク管理オブジェクト
    CubismClippingContext_Cocos2dx* _clippingContextBufferForMask;  ///< マスクテクスチャに描画するためのクリッピングコンテキスト
    CubismClippingContext_Cocos2dx* _clippingContextBufferForDraw;  ///< 画面上描画するためのクリッピングコンテキスト

    csmVector<CubismOffscreenSurface_Cocos2dx> _offscreenSurfaces;          ///< マスク描画用のフレームバッファ
    csmVector<CubismCommandBuffer_Cocos2dx::DrawCommandBuffer*> _drawableDrawCommandBuffer;
};

}}}}
//------------ LIVE2D NAMESPACE ------------
