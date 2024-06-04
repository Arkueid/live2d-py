/**
 * Copyright(c) Live2D Inc. All rights reserved.
 *
 * Use of this source code is governed by the Live2D Open Software license
 * that can be found at https://www.live2d.com/eula/live2d-open-software-license-agreement_en.html.
 */

#pragma once

#include "CubismNativeInclude_D3D9.hpp"

#include "../CubismRenderer.hpp"
#include "../CubismClippingManager.hpp"
#include "CubismFramework.hpp"
#include "Type/csmVector.hpp"
#include "Type/csmRectF.hpp"
#include "Math/CubismVector2.hpp"
#include "Type/csmMap.hpp"
#include "Rendering/D3D9/CubismOffscreenSurface_D3D9.hpp"
#include "CubismRenderState_D3D9.hpp"
#include "CubismType_D3D9.hpp"

//------------ LIVE2D NAMESPACE ------------
namespace Live2D { namespace Cubism { namespace Framework { namespace Rendering {

//  前方宣言
class CubismRenderer_D3D9;
class CubismShader_D3D9;
class CubismClippingContext_D3D9;

/**
 * @brief D3DXMATRIXに変換
 */
D3DXMATRIX ConvertToD3DX(CubismMatrix44& mtx);

/**
 * @brief  クリッピングマスクの処理を実行するクラス
 *
 */
class CubismClippingManager_DX9 : public CubismClippingManager<CubismClippingContext_D3D9, CubismOffscreenSurface_D3D9>
{
public:
    /**
     * @brief   クリッピングコンテキストを作成する。モデル描画時に実行する。
     *
     * @param[in]   model       ->  モデルのインスタンス
     * @param[in]   renderer    ->  レンダラのインスタンス
     */
    void SetupClippingContext(LPDIRECT3DDEVICE9 device, CubismModel& model, CubismRenderer_D3D9* renderer, csmInt32 offscreenCurrent);
};

/**
 * @brief   クリッピングマスクのコンテキスト
 */
class CubismClippingContext_D3D9 : public CubismClippingContext
{
    friend class CubismClippingManager_DX9;
    friend class CubismShader_D3D9;
    friend class CubismRenderer_D3D9;

public:
    /**
     * @brief   引数付きコンストラクタ
     *
     */
    CubismClippingContext_D3D9(CubismClippingManager<CubismClippingContext_D3D9, CubismOffscreenSurface_D3D9>* manager, CubismModel& model, const csmInt32* clippingDrawableIndices, csmInt32 clipCount);

    /**
     * @brief   デストラクタ
     */
    virtual ~CubismClippingContext_D3D9();

    /**
     * @brief   このマスクを管理するマネージャのインスタンスを取得する。
     *
     * @return  クリッピングマネージャのインスタンス
     */
    CubismClippingManager<CubismClippingContext_D3D9, CubismOffscreenSurface_D3D9>* GetClippingManager();

private:
    CubismClippingManager<CubismClippingContext_D3D9, CubismOffscreenSurface_D3D9>* _owner;        ///< このマスクを管理しているマネージャのインスタンス
};


/**
 * @brief   DirectX9用の描画命令を実装したクラス
 *
 */
class CubismRenderer_D3D9 : public CubismRenderer
{
    friend class CubismRenderer;
    friend class CubismClippingManager_DX9;
    friend class CubismShader_D3D9;

public:

    /**
     * @brief    レンダラを作成するための各種設定
     *           モデルロードの前に一度だけ呼び出す
     *
     * @param[in]   bufferSetNum -> 1パーツに付き作成するバッファ数
     * @param[in]   device       -> 使用デバイス
     */
    static void InitializeConstantSettings(csmUint32 bufferSetNum, LPDIRECT3DDEVICE9 device);

    /**
     *  @brief  CubismRenderStateにデフォルトの設定をセットする
     */
    static void SetDefaultRenderState();

    /**
     * @brief  Cubism描画関連の先頭で行う処理。
     *          各フレームでのCubism処理前にこれを呼んでもらう
     *
     * @param[in]   device         -> 使用デバイス
     */
    static void StartFrame(LPDIRECT3DDEVICE9 device, csmUint32 viewportWidth, csmUint32 viewportHeight);

    /**
     * @brief  Cubism描画関連の終了時行う処理。
     *          各フレームでのCubism処理前にこれを呼んでもらう
     *
     * @param[in]   device         -> 使用デバイス
     */
    static void EndFrame(LPDIRECT3DDEVICE9 device);

    /**
     * @brief  CubismRenderer_D3D9で使用するレンダーステート管理マネージャ取得
     */
    static CubismRenderState_D3D9* GetRenderStateManager();

    /**
     * @brief   レンダーステート管理マネージャ削除
     */
    static void DeleteRenderStateManager();

    /**
     * @brief   シェーダ管理機構の取得
     */
    static CubismShader_D3D9* GetShaderManager();

    /**
     * @brief   シェーダ管理機構の削除
     */
    static void DeleteShaderManager();

    /**
     * @brief   デバイスロスト・デバイス再作成時コールする
     */
    static void OnDeviceLost();

    /**
     * @brief   使用シェーダー作成
     */
    static void GenerateShader(LPDIRECT3DDEVICE9 device);

    virtual void CubismRenderer_D3D9::Initialize(CubismModel* model);

    /**
     * @brief   レンダラの初期化処理を実行する<br>
     *           引数に渡したモデルからレンダラの初期化処理に必要な情報を取り出すことができる
     *
     * @param[in]  model -> モデルのインスタンス
     */
    virtual void Initialize(Framework::CubismModel* model, csmInt32 maskBufferCount) override;

    /**
     * @brief   レンダリング前のフレーム先頭で呼ばれる
     */
    void FrameRenderingInit();

    /**
     * @brief   テクスチャのバインド処理<br>
     *           CubismRendererにテクスチャを設定し、CubismRenderer中でその画像を参照するためのIndex値を戻り値とする
     *
     * @param[in]   modelTextureAssign  ->  セットするモデルテクスチャのアサイン番号
     * @param[in]   texture            ->  描画に使用するテクスチャ
     *
     */
    void BindTexture(csmUint32 modelTextureAssign, const LPDIRECT3DTEXTURE9 texture);

    /**
     * @brief   バインドされたテクスチャのリストを取得する
     *
     * @return  テクスチャのアドレスのリスト
     */
    const csmMap<csmInt32, LPDIRECT3DTEXTURE9>& GetBindedTextures() const;

    /**
     * @brief  クリッピングマスクバッファのサイズを設定する<br>
     *         マスク用のOffscreenSurfaceを破棄・再作成するため処理コストは高い。
     *
     * @param[in]  size -> クリッピングマスクバッファのサイズ
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
     * @brief  クリッピングマスクのバッファを取得する
     *
     * @return クリッピングマスクのバッファへの参照
     *
     */
    CubismOffscreenSurface_D3D9* GetMaskBuffer(csmUint32 backbufferNum, csmInt32 offscreenIndex);

protected:
    /**
     * @brief   コンストラクタ
     */
    CubismRenderer_D3D9();

    /**
     * @brief   デストラクタ
     */
    virtual ~CubismRenderer_D3D9();

    /**
     * @brief   モデルを描画する実際の処理
     *
     */
    virtual void DoDrawModel() override;

    /**
     * @brief   描画オブジェクト（アートメッシュ）を描画する。<br>
     *           ポリゴンメッシュとテクスチャ番号をセットで渡す。
     *
     * @param[in]   gfxc            ->  レンダリングコンテキスト
     * @param[in]   drawableIndex   ->  描画パーツ番号
     * @param[in]   colorBlendMode  ->  カラー合成タイプ
     * @param[in]   invertedMask     ->  マスク使用時のマスクの反転使用
     */
    void DrawMeshDX9(const CubismModel& model, const csmInt32 index);

    /**
     * @brief  描画用に使用するシェーダの設定・コンスタントバッファの設定などを行い、描画を実行
     *
     * @param[in]   model           ->  描画対象のモデル
     * @param[in]   index           ->  描画すべき対象のインデックス
     */
    void ExecuteDrawForDraw(const CubismModel& model, const csmInt32 index);

    /**
     * @brief  マスク作成用に使用するシェーダの設定・コンスタントバッファの設定などを行い、描画を実行
     *
     * @param[in]   model           ->  描画対象のモデル
     * @param[in]   index           ->  描画すべき対象のインデックス
     */
    void ExecuteDrawForMask(const CubismModel& model, const csmInt32 index);

private:

    /**
     * @brief   レンダラが保持する静的なリソースを解放する<br>
     */
    static void DoStaticRelease();

    /**
     * @brief   使用シェーダーと頂点定義の削除
     */
    static void ReleaseShader();

    // Prevention of copy Constructor
    CubismRenderer_D3D9(const CubismRenderer_D3D9&);
    CubismRenderer_D3D9& operator=(const CubismRenderer_D3D9&);

    /**
     * @brief   描画開始時の追加処理。<br>
     *           モデルを描画する前にクリッピングマスクに必要な処理を実装している。
     */
    void PreDraw();

    /**
     * @brief   描画完了後の追加処理。<br>
     *           ダブル・トリプルバッファリングに必要な処理を実装している。
     */
    void PostDraw();

    /**
     * @brief   モデル描画直前のステートを保持する
     */
    virtual void SaveProfile() override;

    /**
     * @brief   モデル描画直前のステートを保持する
     */
    virtual void RestoreProfile() override;

    /**
     * @brief   マスクテクスチャに描画するクリッピングコンテキストをセットする。
     */
    void SetClippingContextBufferForMask(CubismClippingContext_D3D9* clip);

    /**
     * @brief   マスクテクスチャに描画するクリッピングコンテキストを取得する。
     *
     * @return  マスクテクスチャに描画するクリッピングコンテキスト
     */
    CubismClippingContext_D3D9* GetClippingContextBufferForMask() const;

    /**
     * @brief   画面上に描画するクリッピングコンテキストをセットする。
     */
    void SetClippingContextBufferForDraw(CubismClippingContext_D3D9* clip);

    /**
     * @brief   画面上に描画するクリッピングコンテキストを取得する。
     *
     * @return  画面上に描画するクリッピングコンテキスト
     */
    CubismClippingContext_D3D9* GetClippingContextBufferForDraw() const;

    /**
     * @brief   GetDrawableVertices,GetDrawableVertexUvsの内容を内部バッファへコピー
     */
    void CopyToBuffer(csmInt32 drawAssign, const csmInt32 vcount, const csmFloat32* varray, const csmFloat32* uvarray);

    /**
     * @brief   modelのindexにバインドされているテクスチャを返す。<br>
     *          もしバインドしていないindexを指定した場合NULLが返る。
     * @param[in]   model       ->  描画対象のモデル
     * @param[in]   index       ->  描画対象のインデックス
     *
     * @return  バインドされたテクスチャ
     */
    LPDIRECT3DTEXTURE9 GetTextureWithIndex(const CubismModel& model, const csmInt32 index);

    /**
     * @brief   マスク生成時かを判定する
     *
     * @return  判定値
     */
    const csmBool inline IsGeneratingMask() const;

    /**
     * @brief   ブレンドモードを設定する
     *
     * @return  判定値
     */
    void SetBlendMode(CubismBlendMode blendMode) const;

    /**
     * @brief   D3D9のテクニックを設定する
     *
     * @param[in]   model       ->  描画対象のモデル
     * @param[in]   index       ->  描画対象のインデックス
     */
    void SetTechniqueForDraw(const CubismModel& model, const csmInt32 index) const;

    /**
     * @brief   RenderStateManagerにテクスチャフィルターを設定する
     */
    void SetTextureFilter() const;

    /**
     * @brief  色関連の定数バッファを設定
     *
     * @param[in]   shaderEffect    -> シェーダーエフェクト
     * @param[in]   baseColor       ->  ベースカラー
     * @param[in]   multiplyColor   ->  乗算カラー
     * @param[in]   screenColor     ->  スクリーンカラー
     */
    void SetColorVectors(ID3DXEffect* shaderEffect, CubismTextureColor& baseColor, CubismTextureColor& multiplyColor, CubismTextureColor& screenColor);

    /**
     * @brief  描画実行時のテクスチャを設定
     *
     * @param[in]   model           ->  描画対象のモデル
     * @param[in]   index           ->  描画すべき対象のインデックス
     * @param[in]   shaderEffect    -> シェーダーエフェクト
     */
    void SetExecutionTextures(const CubismModel& model, const csmInt32 index, ID3DXEffect* shaderEffect);

    /**
     * @brief  描画に使用するカラーチャンネルを設定
     *
     * @param[in]   shaderEffect    -> シェーダーエフェクト
     * @param[in]   contextBuffer   -> 描画コンテキスト
     */
    void SetColorChannel(ID3DXEffect* shaderEffect, CubismClippingContext_D3D9* contextBuffer);

    /**
     * @brief  描画に使用するプロジェクション行列を更新する
     *
     * @param[in]   shaderEffect    -> シェーダーエフェクト
     * @param[in]   matrix          ->  設定するプロジェクション行列
     */
    void SetProjectionMatrix(ID3DXEffect* shaderEffect, CubismMatrix44& matrix);

    /**
     * @brief  引数を指定して DrawIndexedPrimitiveUP 描画命令を実行
     *
     * @param[in]   model           ->  描画対象のモデル
     * @param[in]   index           ->  描画すべき対象のインデックス
     */
    void DrawIndexedPrimiteveWithSetup(const CubismModel& model, const csmInt32 index);

    csmUint32 _drawableNum;           ///< _vertexBuffers, _indexBuffersの確保数

    /**
     * @note D3D9ではDrawPrimitiveUPで描画コマンドを積んでおり、
     * この場合は頂点メモリをユーザー側で保存し続ける必要がないため、一本の配列のみ持つ実装となっている。
     */
    CubismVertexD3D9** _vertexStore;           ///< 頂点をストアしておく領域
    csmUint16** _indexStore;            ///< インデックスをストアしておく領域

    csmInt32 _commandBufferNum;      ///< 描画バッファを複数作成する場合の数
    csmInt32 _commandBufferCurrent;  ///< 現在使用中のバッファ番号

    csmVector<csmInt32> _sortedDrawableIndexList;       ///< 描画オブジェクトのインデックスを描画順に並べたリスト

    csmMap<csmInt32, LPDIRECT3DTEXTURE9> _textures;                      ///< モデルが参照するテクスチャとレンダラでバインドしているテクスチャとのマップ

    csmVector<csmVector<CubismOffscreenSurface_D3D9> > _offscreenSurfaces;          ///< マスク描画用のフレームバッファ

    CubismClippingManager_DX9* _clippingManager;               ///< クリッピングマスク管理オブジェクト
    CubismClippingContext_D3D9* _clippingContextBufferForMask;  ///< マスクテクスチャに描画するためのクリッピングコンテキスト
    CubismClippingContext_D3D9* _clippingContextBufferForDraw;  ///< 画面上描画するためのクリッピングコンテキスト
};

}}}}
//------------ LIVE2D NAMESPACE ------------
