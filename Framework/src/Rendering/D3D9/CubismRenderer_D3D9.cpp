/**
 * Copyright(c) Live2D Inc. All rights reserved.
 *
 * Use of this source code is governed by the Live2D Open Software license
 * that can be found at https://www.live2d.com/eula/live2d-open-software-license-agreement_en.html.
 */

#include "CubismRenderer_D3D9.hpp"

#include <cfloat> // FLT_MAX,MIN

#include "Math/CubismMatrix44.hpp"
#include "Type/csmVector.hpp"
#include "Model/CubismModel.hpp"
#include "CubismShader_D3D9.hpp"
#include "CubismRenderState_D3D9.hpp"

//------------ LIVE2D NAMESPACE ------------
namespace Live2D { namespace Cubism { namespace Framework { namespace Rendering {

D3DXMATRIX ConvertToD3DX(CubismMatrix44& mtx)
{
    D3DXMATRIX retMtx;
    retMtx._11 = mtx.GetArray()[ 0];
    retMtx._12 = mtx.GetArray()[ 1];
    retMtx._13 = mtx.GetArray()[ 2];
    retMtx._14 = mtx.GetArray()[ 3];

    retMtx._21 = mtx.GetArray()[ 4];
    retMtx._22 = mtx.GetArray()[ 5];
    retMtx._23 = mtx.GetArray()[ 6];
    retMtx._24 = mtx.GetArray()[ 7];

    retMtx._31 = mtx.GetArray()[ 8];
    retMtx._32 = mtx.GetArray()[ 9];
    retMtx._33 = mtx.GetArray()[10];
    retMtx._34 = mtx.GetArray()[11];

    retMtx._41 = mtx.GetArray()[12];
    retMtx._42 = mtx.GetArray()[13];
    retMtx._43 = mtx.GetArray()[14];
    retMtx._44 = mtx.GetArray()[15];

    return retMtx;
}

/*********************************************************************************************************************
*                                      CubismClippingManager_DX9
********************************************************************************************************************/
void CubismClippingManager_DX9::SetupClippingContext(LPDIRECT3DDEVICE9 device, CubismModel& model, CubismRenderer_D3D9* renderer, csmInt32 offscreenCurrent)
{
    // 全てのクリッピングを用意する
    // 同じクリップ（複数の場合はまとめて１つのクリップ）を使う場合は１度だけ設定する
    csmInt32 usingClipCount = 0;
    for (csmUint32 clipIndex = 0; clipIndex < _clippingContextListForMask.GetSize(); clipIndex++)
    {
        // １つのクリッピングマスクに関して
        CubismClippingContext_D3D9* cc = _clippingContextListForMask[clipIndex];

        // このクリップを利用する描画オブジェクト群全体を囲む矩形を計算
        CalcClippedDrawTotalBounds(model, cc);

        if (cc->_isUsing)
        {
            usingClipCount++; //使用中としてカウント
        }
    }

    if (usingClipCount <= 0)
    {
        return;
    }

    // マスク作成処理
    // ビューポートは退避済み
    // 生成したOffscreenSurfaceと同じサイズでビューポートを設定
    CubismRenderer_D3D9::GetRenderStateManager()->SetViewport(
            device,
            0,
            0,
            _clippingMaskBufferSize.X,
            _clippingMaskBufferSize.Y,
            0.0f, 1.0f);

    // 後の計算のためにインデックスの最初をセットする
    _currentMaskBuffer = renderer->GetMaskBuffer(offscreenCurrent, 0);

    // ----- マスク描画処理 -----
    // マスク用RenderTextureをactiveにセット
    _currentMaskBuffer->BeginDraw(device);

    // 各マスクのレイアウトを決定していく
    SetupLayoutBounds(usingClipCount);

    // サイズがレンダーテクスチャの枚数と合わない場合は合わせる
    if (_clearedMaskBufferFlags.GetSize() != _renderTextureCount)
    {
        _clearedMaskBufferFlags.Clear();

        for (csmInt32 i = 0; i < _renderTextureCount; ++i)
        {
            _clearedMaskBufferFlags.PushBack(false);
        }
    }
    else
    {
        // マスクのクリアフラグを毎フレーム開始時に初期化
        for (csmInt32 i = 0; i < _renderTextureCount; ++i)
        {
            _clearedMaskBufferFlags[i] = false;
        }
    }

    // 実際にマスクを生成する
    // 全てのマスクをどの様にレイアウトして描くかを決定し、ClipContext , ClippedDrawContext に記憶する
    for (csmUint32 clipIndex = 0; clipIndex < _clippingContextListForMask.GetSize(); clipIndex++)
    {
        // --- 実際に１つのマスクを描く ---
        CubismClippingContext_D3D9* clipContext = _clippingContextListForMask[clipIndex];
        csmRectF* allClippedDrawRect = clipContext->_allClippedDrawRect; //このマスクを使う、全ての描画オブジェクトの論理座標上の囲み矩形
        csmRectF* layoutBoundsOnTex01 = clipContext->_layoutBounds; //この中にマスクを収める
        const csmFloat32 MARGIN = 0.05f;
        const csmBool isRightHanded = true;

        // clipContextに設定したレンダーテクスチャをインデックスで取得
        CubismOffscreenSurface_D3D9* clipContextRenderTexture = renderer->GetMaskBuffer(offscreenCurrent, clipContext->_bufferIndex);

        // 現在のレンダーテクスチャがclipContextのものと異なる場合
        if (_currentMaskBuffer != clipContextRenderTexture)
        {
            _currentMaskBuffer->EndDraw(device);
            _currentMaskBuffer = clipContextRenderTexture;

            // マスク用RenderTextureをactiveにセット
            _currentMaskBuffer->BeginDraw(device);
        }

        // モデル座標上の矩形を、適宜マージンを付けて使う
        _tmpBoundsOnModel.SetRect(allClippedDrawRect);
        _tmpBoundsOnModel.Expand(allClippedDrawRect->Width * MARGIN, allClippedDrawRect->Height * MARGIN);
        //########## 本来は割り当てられた領域の全体を使わず必要最低限のサイズがよい
        // シェーダ用の計算式を求める。回転を考慮しない場合は以下のとおり
        // movePeriod' = movePeriod * scaleX + offX [[ movePeriod' = (movePeriod - tmpBoundsOnModel.movePeriod)*scale + layoutBoundsOnTex01.movePeriod ]]
        csmFloat32 scaleX = layoutBoundsOnTex01->Width / _tmpBoundsOnModel.Width;
        csmFloat32 scaleY = layoutBoundsOnTex01->Height / _tmpBoundsOnModel.Height;

        // マスク生成時に使う行列を求める
        createMatrixForMask(isRightHanded, layoutBoundsOnTex01, scaleX, scaleY);

        clipContext->_matrixForMask.SetMatrix(_tmpMatrixForMask.GetArray());
        clipContext->_matrixForDraw.SetMatrix(_tmpMatrixForDraw.GetArray());

        const csmInt32 clipDrawCount = clipContext->_clippingIdCount;
        for (csmInt32 i = 0; i < clipDrawCount; i++)
        {
            const csmInt32 clipDrawIndex = clipContext->_clippingIdList[i];

            // 頂点情報が更新されておらず、信頼性がない場合は描画をパスする
            if (!model.GetDrawableDynamicFlagVertexPositionsDidChange(clipDrawIndex))
            {
                continue;
            }

            renderer->IsCulling(model.GetDrawableCulling(clipDrawIndex) != 0);

            // マスクがクリアされていないなら処理する
            if (!_clearedMaskBufferFlags[clipContext->_bufferIndex])
            {
                // マスクをクリアする
                // (仮仕様) 1が無効（描かれない）領域、0が有効（描かれる）領域。（シェーダーCd*Csで0に近い値をかけてマスクを作る。1をかけると何も起こらない）
                renderer->GetMaskBuffer(offscreenCurrent, clipContext->_bufferIndex)->Clear(device, 1.0f, 1.0f, 1.0f, 1.0f);
                    _clearedMaskBufferFlags[clipContext->_bufferIndex] = true;
            }

            // 今回専用の変換を適用して描く
            // チャンネルも切り替える必要がある(A,R,G,B)
            renderer->SetClippingContextBufferForMask(clipContext);
            renderer->DrawMeshDX9(model, clipDrawIndex);
        }
    }

    _currentMaskBuffer->EndDraw(device);
    renderer->SetClippingContextBufferForMask(NULL);
}

/*********************************************************************************************************************
*                                      CubismClippingContext_D3D9
********************************************************************************************************************/
CubismClippingContext_D3D9::CubismClippingContext_D3D9(CubismClippingManager<CubismClippingContext_D3D9, CubismOffscreenSurface_D3D9>* manager, CubismModel& model, const csmInt32* clippingDrawableIndices, csmInt32 clipCount)
    : CubismClippingContext(clippingDrawableIndices, clipCount)
{
    _owner = manager;
}

CubismClippingContext_D3D9::~CubismClippingContext_D3D9()
{
}

CubismClippingManager<CubismClippingContext_D3D9, CubismOffscreenSurface_D3D9>* CubismClippingContext_D3D9::GetClippingManager()
{
    return _owner;
}


/*********************************************************************************************************************
 *                                      CubismRenderer_D3D9
 ********************************************************************************************************************/

namespace {
    CubismRenderState_D3D9*         s_renderStateManagerInstance;   ///< レンダーステートの管理
    CubismShader_D3D9*              s_shaderManagerInstance;        ///< シェーダー管理

    csmUint32                       s_bufferSetNum;         ///< 作成コンテキストの数。モデルロード前に設定されている必要あり。
    LPDIRECT3DDEVICE9               s_useDevice;            ///< 使用デバイス。モデルロード前に設定されている必要あり。

    csmUint32                       s_viewportWidth;        ///< 描画ターゲット幅 CubismRenderer_D3D9::startframeで渡されます
    csmUint32                       s_viewportHeight;       ///< 描画ターゲット高さ CubismRenderer_D3D9::startframeで渡されます
}

CubismRenderer* CubismRenderer::Create()
{
    return CSM_NEW CubismRenderer_D3D9();
}

void CubismRenderer::StaticRelease()
{
    CubismRenderer_D3D9::DoStaticRelease();
}

CubismRenderState_D3D9* CubismRenderer_D3D9::GetRenderStateManager()
{
    if (s_renderStateManagerInstance == NULL)
    {
        s_renderStateManagerInstance = CSM_NEW CubismRenderState_D3D9();
    }
    return s_renderStateManagerInstance;
}

void CubismRenderer_D3D9::DeleteRenderStateManager()
{
    if (s_renderStateManagerInstance)
    {
        CSM_DELETE_SELF(CubismRenderState_D3D9, s_renderStateManagerInstance);
        s_renderStateManagerInstance = NULL;
    }
}

CubismShader_D3D9* CubismRenderer_D3D9::GetShaderManager()
{
    if (s_shaderManagerInstance == NULL)
    {
        s_shaderManagerInstance = CSM_NEW CubismShader_D3D9();
    }
    return s_shaderManagerInstance;
}

void CubismRenderer_D3D9::DeleteShaderManager()
{
    if (s_shaderManagerInstance)
    {
        CSM_DELETE_SELF(CubismShader_D3D9, s_shaderManagerInstance);
        s_shaderManagerInstance = NULL;
    }
}

void CubismRenderer_D3D9::GenerateShader(LPDIRECT3DDEVICE9 device)
{
    CubismShader_D3D9* shaderManager = GetShaderManager();
    if(shaderManager)
    {
        shaderManager->GenerateShaders(device);
    }
}

void CubismRenderer_D3D9::OnDeviceLost()
{
    // シェーダー・頂点宣言開放
    ReleaseShader();
}

void CubismRenderer_D3D9::ReleaseShader()
{
    CubismShader_D3D9* shaderManager = GetShaderManager();
    if (shaderManager)
    {
        shaderManager->ReleaseShaderProgram();
    }
}

CubismRenderer_D3D9::CubismRenderer_D3D9()
    : _drawableNum(0)
    , _vertexStore(NULL)
    , _indexStore(NULL)
    , _clippingManager(NULL)
    , _clippingContextBufferForMask(NULL)
    , _clippingContextBufferForDraw(NULL)
{
    _commandBufferNum = 0;
    _commandBufferCurrent = 0;

    // テクスチャ対応マップの容量を確保しておく.
    _textures.PrepareCapacity(32, true);
}

CubismRenderer_D3D9::~CubismRenderer_D3D9()
{
    {
        // オフスクリーンを作成していたのなら開放
        for (csmUint32 i = 0; i < _offscreenSurfaces.GetSize(); i++)
        {
            for (csmUint32 j = 0; j < _offscreenSurfaces[i].GetSize(); j++)
            {
                _offscreenSurfaces[i][j].DestroyOffscreenSurface();
            }
            _offscreenSurfaces[i].Clear();
        }
        _offscreenSurfaces.Clear();
    }

    const csmInt32 drawableCount = _drawableNum; //GetModel()->GetDrawableCount();

    for (csmInt32 drawAssign = 0; drawAssign < drawableCount; drawAssign++)
    {
        // インデックス
        if (_indexStore[drawAssign])
        {
            CSM_FREE(_indexStore[drawAssign]);
        }
        // 頂点
        if (_vertexStore[drawAssign])
        {
            CSM_FREE(_vertexStore[drawAssign]);
        }
    }

    CSM_FREE(_indexStore);
    CSM_FREE(_vertexStore);
    _indexStore = NULL;
    _vertexStore = NULL;

    CSM_DELETE_SELF(CubismClippingManager_DX9, _clippingManager);
}

void CubismRenderer_D3D9::DoStaticRelease()
{
    // レンダーステートマネージャ削除
    DeleteRenderStateManager();
    // シェーダマネージャ削除
    DeleteShaderManager();
}

void CubismRenderer_D3D9::Initialize(CubismModel* model)
{
    Initialize(model, 1);
}

void CubismRenderer_D3D9::Initialize(CubismModel* model, csmInt32 maskBufferCount)
{
    // 0は許されず ここに来るまでに設定しなければならない
    if (s_bufferSetNum == 0)
    {
        CubismLogError("ContextNum has not been set.");
        CSM_ASSERT(0);
        return;
    }
    if (s_useDevice == 0)
    {
        CubismLogError("Device has not been set.");
        CSM_ASSERT(0);
        return;
    }

    // 1未満は1に補正する
    if (maskBufferCount < 1)
    {
        maskBufferCount = 1;
        CubismLogWarning("The number of render textures must be an integer greater than or equal to 1. Set the number of render textures to 1.");
    }

    if (model->IsUsingMasking())
    {
        _clippingManager = CSM_NEW CubismClippingManager_DX9();  //クリッピングマスク・バッファ前処理方式を初期化
        _clippingManager->Initialize(
            *model,
            maskBufferCount
        );

        const csmInt32 bufferWidth = _clippingManager->GetClippingMaskBufferSize().X;
        const csmInt32 bufferHeight = _clippingManager->GetClippingMaskBufferSize().Y;

        _offscreenSurfaces.Clear();

        for (csmUint32 i = 0; i < s_bufferSetNum; i++)
        {
            csmVector<CubismOffscreenSurface_D3D9> vector;
            _offscreenSurfaces.PushBack(vector);
            for (csmUint32 j = 0; j < maskBufferCount; j++)
            {
                CubismOffscreenSurface_D3D9 offscreenOffscreenSurface;
                offscreenOffscreenSurface.CreateOffscreenSurface(s_useDevice, bufferWidth, bufferHeight);
                _offscreenSurfaces[i].PushBack(offscreenOffscreenSurface);
            }
        }

    }

    _sortedDrawableIndexList.Resize(model->GetDrawableCount(), 0);

    CubismRenderer::Initialize(model, maskBufferCount);  //親クラスの処理を呼ぶ

    // モデルパーツごとに確保
    const csmInt32 drawableCount = GetModel()->GetDrawableCount();
    _drawableNum = drawableCount;

    _vertexStore = static_cast<CubismVertexD3D9**>(CSM_MALLOC(sizeof(CubismVertexD3D9*) * drawableCount));
    _indexStore = static_cast<csmUint16**>(CSM_MALLOC(sizeof(csmUint16*) * drawableCount));

    for (csmInt32 drawAssign = 0; drawAssign < drawableCount; drawAssign++)
    {
        // 頂点
        const csmInt32 vcount = GetModel()->GetDrawableVertexCount(drawAssign);
        if (vcount != 0)
        {
            _vertexStore[drawAssign] = static_cast<CubismVertexD3D9*>(CSM_MALLOC(sizeof(CubismVertexD3D9) * vcount));
        }
        else
        {
            _vertexStore[drawAssign] = NULL;
        }

        // インデックスはここで要素コピーを済ませる
        const csmInt32 icount = GetModel()->GetDrawableVertexIndexCount(drawAssign);
        if (icount != 0)
        {
            _indexStore[drawAssign] = static_cast<csmUint16*>(CSM_MALLOC(sizeof(csmUint16) * icount));
            const csmUint16* idxarray = GetModel()->GetDrawableVertexIndices(drawAssign);
            for (csmInt32 ct = 0; ct < icount; ct++)
            {// モデルデータからのコピー
                _indexStore[drawAssign][ct] = idxarray[ct];
            }
        }
        else
        {
            _indexStore[drawAssign] = NULL;
        }
    }

    _commandBufferNum = s_bufferSetNum;
    _commandBufferCurrent = 0;
}

void CubismRenderer_D3D9::FrameRenderingInit()
{
}

void CubismRenderer_D3D9::PreDraw()
{
    SetDefaultRenderState();
}

void CubismRenderer_D3D9::PostDraw()
{
    // ダブル・トリプルバッファを回す
    _commandBufferCurrent++;
    if (_commandBufferNum <= _commandBufferCurrent)
    {
        _commandBufferCurrent = 0;
    }
}

void CubismRenderer_D3D9::DoDrawModel()
{
    // NULLは許されず
    CSM_ASSERT(s_useDevice != NULL);

    PreDraw();

    //------------ クリッピングマスク・バッファ前処理方式の場合 ------------
    if (_clippingManager != NULL)
    {
        // サイズが違う場合はここで作成しなおし
        for (csmInt32 i = 0; i < _clippingManager->GetRenderTextureCount(); ++i)
        {
            if (_offscreenSurfaces[_commandBufferCurrent][i].GetBufferWidth() != static_cast<csmUint32>(_clippingManager->GetClippingMaskBufferSize().X) ||
                _offscreenSurfaces[_commandBufferCurrent][i].GetBufferHeight() != static_cast<csmUint32>(_clippingManager->GetClippingMaskBufferSize().Y))
            {
                _offscreenSurfaces[_commandBufferCurrent][i].CreateOffscreenSurface(s_useDevice,
                    static_cast<csmUint32>(_clippingManager->GetClippingMaskBufferSize().X), static_cast<csmUint32>(_clippingManager->GetClippingMaskBufferSize().Y));
            }
        }

        if (IsUsingHighPrecisionMask())
        {
            _clippingManager->SetupMatrixForHighPrecision(*GetModel(), true);
        }
        else
        {
            _clippingManager->SetupClippingContext(s_useDevice, *GetModel(), this, _commandBufferCurrent);
        }

        if (!IsUsingHighPrecisionMask())
        {
            // ビューポートを元に戻す
            GetRenderStateManager()->SetViewport(s_useDevice,
                0,
                0,
                s_viewportWidth,
                s_viewportHeight,
                0.0f, 1.0);
        }
    }

    const csmInt32 drawableCount = GetModel()->GetDrawableCount();
    const csmInt32* renderOrder = GetModel()->GetDrawableRenderOrders();

    // インデックスを描画順でソート
    for (csmInt32 i = 0; i < drawableCount; ++i)
    {
        const csmInt32 order = renderOrder[i];
        _sortedDrawableIndexList[order] = i;
    }

    // 描画
    for (csmInt32 i = 0; i < drawableCount; ++i)
    {
        const csmInt32 drawableIndex = _sortedDrawableIndexList[i];

        // Drawableが表示状態でなければ処理をパスする
        if (!GetModel()->GetDrawableDynamicFlagIsVisible(drawableIndex))
        {
            continue;
        }

        // クリッピングマスクをセットする
        CubismClippingContext_D3D9* clipContext = (_clippingManager != NULL)
            ? (*_clippingManager->GetClippingContextListForDraw())[drawableIndex]
            : NULL;

        if (clipContext != NULL && IsUsingHighPrecisionMask()) // マスクを書く必要がある
        {
            if (clipContext->_isUsing) // 書くことになっていた
            {
                GetRenderStateManager()->SetViewport(s_useDevice,
                    0,
                    0,
                    _clippingManager->GetClippingMaskBufferSize().X,
                    _clippingManager->GetClippingMaskBufferSize().Y,
                    0.0f, 1.0f);

                // 正しいレンダーターゲットを持つオフスクリーンサーフェイスバッファを呼ぶ
                CubismOffscreenSurface_D3D9* currentHighPrecisionMaskColorBuffer = &_offscreenSurfaces[_commandBufferCurrent][clipContext->_bufferIndex];

                currentHighPrecisionMaskColorBuffer->BeginDraw(s_useDevice);

                // 1が無効（描かれない）領域、0が有効（描かれる）領域。（シェーダで Cd*Csで0に近い値をかけてマスクを作る。1をかけると何も起こらない）
                currentHighPrecisionMaskColorBuffer->Clear(s_useDevice, 1.0f, 1.0f, 1.0f, 1.0f);

                const csmInt32 clipDrawCount = clipContext->_clippingIdCount;
                for (csmInt32 ctx = 0; ctx < clipDrawCount; ctx++)
                {
                    const csmInt32 clipDrawIndex = clipContext->_clippingIdList[ctx];

                    // 頂点情報が更新されておらず、信頼性がない場合は描画をパスする
                    if (!GetModel()->GetDrawableDynamicFlagVertexPositionsDidChange(clipDrawIndex))
                    {
                        continue;
                    }

                    IsCulling(GetModel()->GetDrawableCulling(clipDrawIndex) != 0);

                    // 今回専用の変換を適用して描く
                    // チャンネルも切り替える必要がある(A,R,G,B)
                    SetClippingContextBufferForMask(clipContext);
                    DrawMeshDX9(*GetModel(), clipDrawIndex);
                }

                // --- 後処理 ---
                currentHighPrecisionMaskColorBuffer->EndDraw(s_useDevice);
                SetClippingContextBufferForMask(NULL);

                // ビューポートを元に戻す
                GetRenderStateManager()->SetViewport(s_useDevice,
                    0,
                    0,
                    s_viewportWidth,
                    s_viewportHeight,
                    0.0f, 1.0f);
            }
        }

        // クリッピングマスクをセットする
        SetClippingContextBufferForDraw(clipContext);

        IsCulling(GetModel()->GetDrawableCulling(drawableIndex) != 0);
        DrawMeshDX9(*GetModel(), drawableIndex);
    }

    // ダブルバッファ・トリプルバッファを回す
    PostDraw();
}

void CubismRenderer_D3D9::ExecuteDrawForDraw(const CubismModel& model, const csmInt32 index)
{
    // 使用シェーダエフェクト取得
    CubismShader_D3D9* shaderManager = Live2D::Cubism::Framework::Rendering::CubismRenderer_D3D9::GetShaderManager();
    if (!shaderManager)
    {
        return;
    }

    ID3DXEffect* shaderEffect = shaderManager->GetShaderEffect();
    if (!shaderEffect)
    {
        return;
    }

    SetBlendMode(model.GetDrawableBlendMode(index));
    SetTextureFilter();
    SetTechniqueForDraw(model, index);

    // numPassには指定のtechnique内に含まれるpassの数が返る
    UINT numPass = 0;
    shaderEffect->Begin(&numPass, 0);
    shaderEffect->BeginPass(0);

    // テスクチャ
    SetExecutionTextures(model, index, shaderEffect);

    // 定数バッファ
    {
        const csmBool masked = GetClippingContextBufferForDraw() != NULL;
        if (masked)
        {
            // View座標をClippingContextの座標に変換するための行列を設定
            CubismMatrix44 ClipF = GetClippingContextBufferForDraw()->_matrixForDraw;
            D3DXMATRIX clipM = ConvertToD3DX(ClipF);
            shaderEffect->SetMatrix("clipMatrix", &clipM);

            // 使用するカラーチャンネルを設定
            CubismClippingContext_D3D9* contextBuffer = GetClippingContextBufferForDraw();
            SetColorChannel(shaderEffect, contextBuffer);
        }

        // プロジェクション行列
        SetProjectionMatrix(shaderEffect, GetMvpMatrix());

        // 色
        CubismTextureColor modelColorRGBA = GetModelColorWithOpacity(model.GetDrawableOpacity(index));
        CubismTextureColor multiplyColor = model.GetMultiplyColor(index);
        CubismTextureColor screenColor = model.GetScreenColor(index);
        SetColorVectors(shaderEffect, modelColorRGBA, multiplyColor, screenColor);
    }

    // パラメータ反映
    shaderEffect->CommitChanges();

    // 描画
    DrawIndexedPrimiteveWithSetup(model, index);

    shaderEffect->EndPass();
    shaderEffect->End();
}

void CubismRenderer_D3D9::ExecuteDrawForMask(const CubismModel& model, const csmInt32 index)
{
    // 使用シェーダエフェクト取得
    CubismShader_D3D9* shaderManager = Live2D::Cubism::Framework::Rendering::CubismRenderer_D3D9::GetShaderManager();
    if (!shaderManager)
    {
        return;
    }

    ID3DXEffect* shaderEffect = shaderManager->GetShaderEffect();
    if (!shaderEffect)
    {
        return;
    }

    shaderEffect->SetTechnique("ShaderNames_SetupMask");
    SetBlendMode(CubismBlendMode::CubismBlendMode_Mask);
    SetTextureFilter();

    // numPassには指定のtechnique内に含まれるpassの数が返る
    UINT numPass = 0;
    shaderEffect->Begin(&numPass, 0);
    shaderEffect->BeginPass(0);

    // マスクには線形補間を適用
    GetRenderStateManager()->SetTextureFilter(s_useDevice, 1, D3DTEXF_LINEAR, D3DTEXF_LINEAR, D3DTEXF_LINEAR, D3DTADDRESS_WRAP, D3DTADDRESS_WRAP);

    // 定数バッファ
    {
        // プロジェクション行列
        SetProjectionMatrix(shaderEffect, GetClippingContextBufferForMask()->_matrixForMask);

        // 色
        csmRectF* rect = GetClippingContextBufferForMask()->_layoutBounds;
        CubismTextureColor baseColor = {rect->X * 2.0f - 1.0f, rect->Y * 2.0f - 1.0f, rect->GetRight() * 2.0f - 1.0f, rect->GetBottom() * 2.0f - 1.0f};
        CubismTextureColor multiplyColor = model.GetMultiplyColor(index);
        CubismTextureColor screenColor = model.GetScreenColor(index);
        SetColorVectors(shaderEffect, baseColor, multiplyColor, screenColor);

        // 使用するカラーチャンネルを設定
        CubismClippingContext_D3D9* contextBuffer = GetClippingContextBufferForMask();
        SetColorChannel(shaderEffect, contextBuffer);

        // テスクチャ
        SetExecutionTextures(model, index, shaderEffect);

        // パラメータ反映
        shaderEffect->CommitChanges();
    }

    // 描画
    DrawIndexedPrimiteveWithSetup(model, index);

    shaderEffect->EndPass();
    shaderEffect->End();
}

void CubismRenderer_D3D9::DrawMeshDX9(const CubismModel& model, const csmInt32 index)
{
    // デバイス未設定
    if (s_useDevice == NULL)
    {
        return;
    }

    // 描画物無し
    if(model.GetDrawableVertexIndexCount(index) == 0)
    {
        return;
    }

    // 描画不要なら描画処理をスキップする
    if (model.GetDrawableOpacity(index) <= 0.0f && !IsGeneratingMask())
    {
        return;
    }

    // モデルが参照するテクスチャがバインドされていない場合は描画をスキップする
    if (GetTextureWithIndex(model, index) == NULL)
    {
        return;
    }

    // 裏面描画の有効・無効
    GetRenderStateManager()->SetCullMode(s_useDevice, (IsCulling() ? D3DCULL_CW : D3DCULL_NONE));

    // 頂点バッファにコピー
    {
        const csmInt32 drawableIndex = index;
        const csmInt32 vertexCount = model.GetDrawableVertexCount(index);
        const csmFloat32* vertexArray = model.GetDrawableVertices(index);
        const csmFloat32* uvArray = reinterpret_cast<const csmFloat32*>(model.GetDrawableVertexUvs(index));
        CopyToBuffer(drawableIndex, vertexCount, vertexArray, uvArray);
    }

    // シェーダーセット
    if (IsGeneratingMask())
    {
        ExecuteDrawForMask(model, index);
    }
    else
    {
        ExecuteDrawForDraw(model, index);
    }

    SetClippingContextBufferForDraw(NULL);
    SetClippingContextBufferForMask(NULL);
}

void CubismRenderer_D3D9::SaveProfile()
{
    // NULLは許されず
    CSM_ASSERT(s_useDevice != NULL);

    // 現在のレンダリングステートをPush
    GetRenderStateManager()->SaveCurrentNativeState(s_useDevice);
}

void CubismRenderer_D3D9::RestoreProfile()
{
    // NULLは許されず
    CSM_ASSERT(s_useDevice != NULL);

    // SaveCurrentNativeStateと対
    GetRenderStateManager()->RestoreNativeState(s_useDevice);
}

void CubismRenderer_D3D9::BindTexture(csmUint32 modelTextureAssign, LPDIRECT3DTEXTURE9 texture)
{
    _textures[modelTextureAssign] = texture;
}

const csmMap<csmInt32, LPDIRECT3DTEXTURE9>& CubismRenderer_D3D9::GetBindedTextures() const
{
    return _textures;
}

void CubismRenderer_D3D9::SetClippingMaskBufferSize(csmFloat32 width, csmFloat32 height)
{
    if (_clippingManager == NULL)
    {
        return;
    }

    // インスタンス破棄前にレンダーテクスチャの数を保存
    const csmInt32 renderTextureCount = _clippingManager->GetRenderTextureCount();


    //OffscreenSurfaceのサイズを変更するためにインスタンスを破棄・再作成する
    CSM_DELETE_SELF(CubismClippingManager_DX9, _clippingManager);

    _clippingManager = CSM_NEW CubismClippingManager_DX9();

    _clippingManager->SetClippingMaskBufferSize(width, height);

    _clippingManager->Initialize(
        *GetModel(),
        renderTextureCount
    );
}

csmInt32 CubismRenderer_D3D9::GetRenderTextureCount() const
{
    return _clippingManager->GetRenderTextureCount();
}

CubismVector2 CubismRenderer_D3D9::GetClippingMaskBufferSize() const
{
    return _clippingManager->GetClippingMaskBufferSize();
}

CubismOffscreenSurface_D3D9* CubismRenderer_D3D9::GetMaskBuffer(csmUint32 backbufferNum, csmInt32 offscreenIndex)
{
    return &_offscreenSurfaces[backbufferNum][offscreenIndex];
}

void CubismRenderer_D3D9::InitializeConstantSettings(csmUint32 bufferSetNum, LPDIRECT3DDEVICE9 device)
{
    s_bufferSetNum = bufferSetNum;
    s_useDevice = device;
}

void CubismRenderer_D3D9::SetDefaultRenderState()
{
    // Zは無効 描画順で制御
    GetRenderStateManager()->SetZEnable(s_useDevice,
        D3DZB_FALSE,
        D3DCMP_LESS);

    // ビューポート
    GetRenderStateManager()->SetViewport(s_useDevice,
        0,
        0,
        s_viewportWidth,
        s_viewportHeight,
        0.0f, 1.0);

    // カラーマスク
    GetRenderStateManager()->SetColorMask(s_useDevice,
        D3DCOLORWRITEENABLE_ALPHA | D3DCOLORWRITEENABLE_BLUE | D3DCOLORWRITEENABLE_GREEN | D3DCOLORWRITEENABLE_RED);

    // カリング
    GetRenderStateManager()->SetCullMode(s_useDevice,
        D3DCULL_CW); // CWを消す

    // フィルタ
    GetRenderStateManager()->SetTextureFilter(s_useDevice,
        0, D3DTEXF_LINEAR, D3DTEXF_LINEAR, D3DTEXF_LINEAR, D3DTADDRESS_WRAP, D3DTADDRESS_WRAP
    );
}

void CubismRenderer_D3D9::StartFrame(LPDIRECT3DDEVICE9 device, csmUint32 viewportWidth, csmUint32 viewportHeight)
{
    // フレームで使用するデバイス設定
    s_useDevice = device;
    s_viewportWidth = viewportWidth;
    s_viewportHeight = viewportHeight;

    // レンダーステートフレーム先頭処理
    GetRenderStateManager()->StartFrame();

    // シェーダ・頂点宣言
    GetShaderManager()->SetupShader(s_useDevice);
}

void CubismRenderer_D3D9::EndFrame(LPDIRECT3DDEVICE9 device)
{
    // 使用シェーダエフェクト取得
    Live2D::Cubism::Framework::Rendering::CubismShader_D3D9* shaderManager = Live2D::Cubism::Framework::Rendering::CubismRenderer_D3D9::GetShaderManager();
    {
        ID3DXEffect* shaderEffect = shaderManager->GetShaderEffect();
        // テクスチャの参照を外しておく
        if (shaderEffect)
        {
            shaderEffect->SetTexture("mainTexture", NULL);
            shaderEffect->SetTexture("maskTexture", NULL);
            shaderEffect->CommitChanges();
        }
    }
}

void CubismRenderer_D3D9::SetClippingContextBufferForDraw(CubismClippingContext_D3D9* clip)
{
    _clippingContextBufferForDraw = clip;
}

CubismClippingContext_D3D9* CubismRenderer_D3D9::GetClippingContextBufferForDraw() const
{
    return _clippingContextBufferForDraw;
}

void CubismRenderer_D3D9::SetClippingContextBufferForMask(CubismClippingContext_D3D9* clip)
{
    _clippingContextBufferForMask = clip;
}

CubismClippingContext_D3D9* CubismRenderer_D3D9::GetClippingContextBufferForMask() const
{
    return _clippingContextBufferForMask;
}

void CubismRenderer_D3D9::CopyToBuffer(csmInt32 drawAssign, const csmInt32 vcount, const csmFloat32* varray, const csmFloat32* uvarray)
{
    for (csmInt32 ct = 0; ct < vcount * 2; ct += 2)
    {// モデルデータからのコピー
        _vertexStore[drawAssign][ct / 2].x = varray[ct + 0];
        _vertexStore[drawAssign][ct / 2].y = varray[ct + 1];

        _vertexStore[drawAssign][ct / 2].u = uvarray[ct + 0];
        _vertexStore[drawAssign][ct / 2].v = uvarray[ct + 1];
    }
}

LPDIRECT3DTEXTURE9 CubismRenderer_D3D9::GetTextureWithIndex(const CubismModel& model, const csmInt32 index)
{
    LPDIRECT3DTEXTURE9 result = NULL;
    const csmInt32 textureIndex = model.GetDrawableTextureIndex(index);
    if (textureIndex >= 0)
    {
        result = _textures[textureIndex];
    }

    return result;
}

const csmBool inline CubismRenderer_D3D9::IsGeneratingMask() const
{
    return (GetClippingContextBufferForMask() != NULL);
}

void CubismRenderer_D3D9::SetBlendMode(CubismBlendMode blendMode) const
{
    switch (blendMode)
    {
    case CubismBlendMode::CubismBlendMode_Normal:
    default:
        GetRenderStateManager()->SetBlend(s_useDevice,
            true,
            true,
            D3DBLEND_ONE,
            D3DBLENDOP_ADD,
            D3DBLEND_INVSRCALPHA,
            D3DBLEND_ONE,
            D3DBLENDOP_ADD,
            D3DBLEND_INVSRCALPHA);
        break;

    case CubismBlendMode::CubismBlendMode_Additive:
        GetRenderStateManager()->SetBlend(s_useDevice,
            true,
            true,
            D3DBLEND_ONE,
            D3DBLENDOP_ADD,
            D3DBLEND_ONE,
            D3DBLEND_ZERO,
            D3DBLENDOP_ADD,
            D3DBLEND_ONE);
        break;

    case CubismBlendMode::CubismBlendMode_Multiplicative:
       GetRenderStateManager()->SetBlend(s_useDevice,
        true,
        true,
        D3DBLEND_DESTCOLOR,
        D3DBLENDOP_ADD,
        D3DBLEND_INVSRCALPHA,
        D3DBLEND_ZERO,
        D3DBLENDOP_ADD,
        D3DBLEND_ONE);
        break;

    case CubismBlendMode::CubismBlendMode_Mask:
        GetRenderStateManager()->SetBlend(s_useDevice,
            true,
            true,
            D3DBLEND_ZERO,
            D3DBLENDOP_ADD,
            D3DBLEND_INVSRCCOLOR,
            D3DBLEND_ZERO,
            D3DBLENDOP_ADD,
            D3DBLEND_INVSRCALPHA);
            break;
    }
}

void CubismRenderer_D3D9::SetTechniqueForDraw(const CubismModel& model, const csmInt32 index) const
{
    ID3DXEffect* shaderEffect = GetShaderManager()->GetShaderEffect();
    const csmBool masked = GetClippingContextBufferForDraw() != NULL;  // この描画オブジェクトはマスク対象か
    const csmBool premult = IsPremultipliedAlpha();
    const csmBool invertedMask = model.GetDrawableInvertedMask(index);
    if (masked)
    {
        if(premult)
        {
            if (invertedMask)
            {
                shaderEffect->SetTechnique("ShaderNames_NormalMaskedInvertedPremultipliedAlpha");
            }
            else
            {
                shaderEffect->SetTechnique("ShaderNames_NormalMaskedPremultipliedAlpha");
            }
        }
        else
        {
            if (invertedMask)
            {
                shaderEffect->SetTechnique("ShaderNames_NormalMaskedInverted");
           }
            else
            {
                shaderEffect->SetTechnique("ShaderNames_NormalMasked");
            }
        }
    }
    else
    {
        if(premult)
        {
            shaderEffect->SetTechnique("ShaderNames_NormalPremultipliedAlpha");
        }
        else
        {
            shaderEffect->SetTechnique("ShaderNames_Normal");
        }
    }
}

void CubismRenderer_D3D9::SetTextureFilter() const
{
    if (GetAnisotropy() > 0.0)
        {
            GetRenderStateManager()->SetTextureFilter(s_useDevice, 0, D3DTEXF_ANISOTROPIC, D3DTEXF_ANISOTROPIC, D3DTEXF_LINEAR, D3DTADDRESS_WRAP, D3DTADDRESS_WRAP, GetAnisotropy());
        }
        else
        {
            GetRenderStateManager()->SetTextureFilter(s_useDevice, 0, D3DTEXF_LINEAR, D3DTEXF_LINEAR, D3DTEXF_LINEAR, D3DTADDRESS_WRAP, D3DTADDRESS_WRAP);
        }
}

void CubismRenderer_D3D9::SetColorVectors(ID3DXEffect* shaderEffect, CubismTextureColor& baseColor, CubismTextureColor& multiplyColor, CubismTextureColor& screenColor)
{
    D3DXVECTOR4 shaderBaseColor(baseColor.R, baseColor.G, baseColor.B, baseColor.A);
    D3DXVECTOR4 shaderMultiplyColor(multiplyColor.R, multiplyColor.G, multiplyColor.B, multiplyColor.A);
    D3DXVECTOR4 shaderScreenColor(screenColor.R, screenColor.G, screenColor.B, screenColor.A);

    shaderEffect->SetVector("baseColor", &shaderBaseColor);
    shaderEffect->SetVector("multiplyColor", &shaderMultiplyColor);
    shaderEffect->SetVector("screenColor", &shaderScreenColor);
}

void CubismRenderer_D3D9::SetExecutionTextures(const CubismModel& model, const csmInt32 index, ID3DXEffect* shaderEffect)
{
    const csmBool drawing = !IsGeneratingMask();
    const csmBool masked = GetClippingContextBufferForDraw() != NULL;
    LPDIRECT3DTEXTURE9 mainTexture = GetTextureWithIndex(model, index);
    LPDIRECT3DTEXTURE9 maskTexture = ((drawing && masked) ? _offscreenSurfaces[_commandBufferCurrent][GetClippingContextBufferForDraw()->_bufferIndex].GetTexture() : NULL);

    shaderEffect->SetTexture("mainTexture", mainTexture);
    shaderEffect->SetTexture("maskTexture", maskTexture);
}

void CubismRenderer_D3D9::SetColorChannel(ID3DXEffect* shaderEffect, CubismClippingContext_D3D9* contextBuffer)
{
    const csmInt32 channelIndex = contextBuffer->_layoutChannelIndex;
    CubismRenderer::CubismTextureColor* colorChannel = contextBuffer->GetClippingManager()->GetChannelFlagAsColor(channelIndex);
    D3DXVECTOR4 channel(colorChannel->R, colorChannel->G, colorChannel->B, colorChannel->A);
    shaderEffect->SetVector("channelFlag", &channel);
}

void CubismRenderer_D3D9::SetProjectionMatrix(ID3DXEffect* shaderEffect, CubismMatrix44& matrix)
{
    D3DXMATRIX proj = ConvertToD3DX(matrix);
    shaderEffect->SetMatrix("projectMatrix", &proj);
}

void CubismRenderer_D3D9::DrawIndexedPrimiteveWithSetup(const CubismModel& model, const csmInt32 index)
{
    const csmInt32 vertexCount = model.GetDrawableVertexCount(index);
    const csmInt32 indexCount = model.GetDrawableVertexIndexCount(index);
    const csmInt32 triangleCount = indexCount / 3;
    const csmUint16* indexArray = _indexStore[index];
    const CubismVertexD3D9* vertexArray = _vertexStore[index];
    s_useDevice->DrawIndexedPrimitiveUP(D3DPT_TRIANGLELIST, 0, vertexCount, triangleCount, indexArray, D3DFMT_INDEX16, vertexArray, sizeof(CubismVertexD3D9));
}

}}}}

//------------ LIVE2D NAMESPACE ------------
