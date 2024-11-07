/**
 * Copyright(c) Live2D Inc. All rights reserved.
 *
 * Use of this source code is governed by the Live2D Open Software license
 * that can be found at https://www.live2d.com/eula/live2d-open-software-license-agreement_en.html.
 */

#include "CubismRenderer_Cocos2dx.hpp"
#include "Math/CubismMatrix44.hpp"
#include "Type/csmVector.hpp"
#include "Model/CubismModel.hpp"
#include <float.h>
#include "renderer/backend/Device.h"

#ifdef CSM_TARGET_WIN_GL
#include <Windows.h>
#endif

//------------ LIVE2D NAMESPACE ------------
namespace Live2D { namespace Cubism { namespace Framework { namespace Rendering {

/*********************************************************************************************************************
*                                      CubismClippingManager_Cocos2dx
********************************************************************************************************************/
void CubismClippingManager_Cocos2dx::SetupClippingContext(CubismModel& model, CubismRenderer_Cocos2dx* renderer, cocos2d::Texture2D* lastColorBuffer, csmRectF lastViewport)
{
    // 全てのクリッピングを用意する
    // 同じクリップ（複数の場合はまとめて１つのクリップ）を使う場合は１度だけ設定する
    csmInt32 usingClipCount = 0;
    for (csmUint32 clipIndex = 0; clipIndex < _clippingContextListForMask.GetSize(); clipIndex++)
    {
        // １つのクリッピングマスクに関して
        CubismClippingContext_Cocos2dx* cc = _clippingContextListForMask[clipIndex];

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
    // 後の計算のためにインデックスの最初をセット
    _currentMaskBuffer = renderer->GetOffscreenSurface(0);

    // 生成したOffscreenSurfaceと同じサイズでビューポートを設定
    renderer->GetCommandBuffer()->Viewport(0, 0, _currentMaskBuffer->GetViewPortSize().Width, _currentMaskBuffer->GetViewPortSize().Height);

    renderer->PreDraw(); // バッファをクリアする

    // _OffscreenSurfaceへ切り替え
    _currentMaskBuffer->BeginDraw(renderer->GetCommandBuffer(), lastColorBuffer);

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
        CubismClippingContext_Cocos2dx* clipContext = _clippingContextListForMask[clipIndex];
        csmRectF* allClippedDrawRect = clipContext->_allClippedDrawRect; //このマスクを使う、全ての描画オブジェクトの論理座標上の囲み矩形
        csmRectF* layoutBoundsOnTex01 = clipContext->_layoutBounds; //この中にマスクを収める
        const csmFloat32 MARGIN = 0.05f;
        const csmBool isRightHanded = false;

        // clipContextに設定したレンダーテクスチャをインデックスで取得
        CubismOffscreenSurface_Cocos2dx* clipContextOffscreenSurface = renderer->GetOffscreenSurface(clipContext->_bufferIndex);

        // 現在のレンダーテクスチャがclipContextのものと異なる場合
        if (_currentMaskBuffer != clipContextOffscreenSurface)
        {
            _currentMaskBuffer->EndDraw(renderer->GetCommandBuffer());
            _currentMaskBuffer = clipContextOffscreenSurface;
            renderer->GetCommandBuffer()->Viewport(0, 0, _currentMaskBuffer->GetViewPortSize().Width, _currentMaskBuffer->GetViewPortSize().Height);
            renderer->PreDraw(); // バッファをクリアする
            _currentMaskBuffer->BeginDraw(renderer->GetCommandBuffer(), lastColorBuffer);
        }


        // モデル座標上の矩形を、適宜マージンを付けて使う
        _tmpBoundsOnModel.SetRect(allClippedDrawRect);
        _tmpBoundsOnModel.Expand(allClippedDrawRect->Width * MARGIN, allClippedDrawRect->Height * MARGIN);
        //########## 本来は割り当てられた領域の全体を使わず必要最低限のサイズがよい
        // シェーダ用の計算式を求める。回転を考慮しない場合は以下のとおり
        // movePeriod' = movePeriod * scaleX + offX     [[ movePeriod' = (movePeriod - tmpBoundsOnModel.movePeriod)*scale + layoutBoundsOnTex01.movePeriod ]]
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
            CubismCommandBuffer_Cocos2dx::DrawCommandBuffer* drawCommandBufferData = clipContext->_clippingCommandBufferList->At(i);// [i];


            // 頂点情報が更新されておらず、信頼性がない場合は描画をパスする
            if (!model.GetDrawableDynamicFlagVertexPositionsDidChange(clipDrawIndex))
            {
                continue;
            }

            // Update Vertex / Index buffer.
            {
                csmFloat32* vertices = const_cast<csmFloat32*>(model.GetDrawableVertices(clipDrawIndex));
                Core::csmVector2* uvs = const_cast<Core::csmVector2*>(model.GetDrawableVertexUvs(clipDrawIndex));
                csmUint16* vertexIndices = const_cast<csmUint16*>(model.GetDrawableVertexIndices(clipDrawIndex));
                const csmUint32 vertexCount = model.GetDrawableVertexCount(clipDrawIndex);
                const csmUint32 vertexIndexCount = model.GetDrawableVertexIndexCount(clipDrawIndex);

                drawCommandBufferData->UpdateVertexBuffer(vertices, uvs, vertexCount);
                drawCommandBufferData->CommitVertexBuffer();
                if (vertexIndexCount > 0)
                {
                    drawCommandBufferData->UpdateIndexBuffer(vertexIndices, vertexIndexCount);
                }

                if (vertexCount <= 0)
                {
                    continue;
                }

            }

            renderer->IsCulling(model.GetDrawableCulling(clipDrawIndex) != 0);

            // マスクがクリアされていないなら処理する
            if (!_clearedMaskBufferFlags[clipContext->_bufferIndex])
            {
                // マスクをクリアする
                // (仮仕様) 1が無効（描かれない）領域、0が有効（描かれる）領域。（シェーダーCd*Csで0に近い値をかけてマスクを作る。1をかけると何も起こらない）
                renderer->GetOffscreenSurface(clipContext->_bufferIndex)->Clear(renderer->GetCommandBuffer(), 1.0f, 1.0f, 1.0f, 1.0f);
                _clearedMaskBufferFlags[clipContext->_bufferIndex] = true;
            }

            // 今回専用の変換を適用して描く
            // チャンネルも切り替える必要がある(A,R,G,B)
            renderer->SetClippingContextBufferForMask(clipContext);
            renderer->DrawMeshCocos2d(drawCommandBufferData->GetCommandDraw(), model, clipDrawIndex);
        }
    }

    // --- 後処理 ---
    _currentMaskBuffer->EndDraw(renderer->GetCommandBuffer()); // 描画対象を戻す
    renderer->SetClippingContextBufferForMask(NULL);
    renderer->GetCommandBuffer()->Viewport(lastViewport.X, lastViewport.Y, lastViewport.Width, lastViewport.Height);
}

/*********************************************************************************************************************
*                                      CubismClippingContext_Cocos2dx
********************************************************************************************************************/
CubismClippingContext_Cocos2dx::CubismClippingContext_Cocos2dx(CubismClippingManager<CubismClippingContext_Cocos2dx, CubismOffscreenSurface_Cocos2dx>* manager, CubismModel& model, const csmInt32* clippingDrawableIndices, csmInt32 clipCount)
   : CubismClippingContext(clippingDrawableIndices, clipCount)
{
    _owner = manager;

    _clippingCommandBufferList = CSM_NEW csmVector<CubismCommandBuffer_Cocos2dx::DrawCommandBuffer*>;
    for (csmUint32 i = 0; i < _clippingIdCount; ++i)
    {
        const csmInt32 clippingId = _clippingIdList[i];
        CubismCommandBuffer_Cocos2dx::DrawCommandBuffer* drawCommandBuffer = NULL;
        const csmInt32 drawableVertexCount = model.GetDrawableVertexCount(clippingId);
        const csmInt32 drawableVertexIndexCount = model.GetDrawableVertexIndexCount(clippingId);
        const csmSizeInt vertexSize = sizeof(csmFloat32) * 2;


        drawCommandBuffer = CSM_NEW CubismCommandBuffer_Cocos2dx::DrawCommandBuffer();
        drawCommandBuffer->GetCommandDraw()->GetCommand()->setDrawType(cocos2d::CustomCommand::DrawType::ELEMENT);
        drawCommandBuffer->GetCommandDraw()->GetCommand()->setPrimitiveType(cocos2d::backend::PrimitiveType::TRIANGLE);
        drawCommandBuffer->CreateVertexBuffer(vertexSize, drawableVertexCount * 2);      // Vertices + UVs
        drawCommandBuffer->CreateIndexBuffer(drawableVertexIndexCount);


        _clippingCommandBufferList->PushBack(drawCommandBuffer);
    }
}

CubismClippingContext_Cocos2dx::~CubismClippingContext_Cocos2dx()
{
    if (_clippingCommandBufferList != NULL)
    {
        for (csmUint32 i = 0; i < _clippingCommandBufferList->GetSize(); ++i)
        {
            CSM_DELETE(_clippingCommandBufferList->At(i));
            _clippingCommandBufferList->At(i) = NULL;
        }

        CSM_DELETE(_clippingCommandBufferList);
        _clippingCommandBufferList = NULL;
    }
}

CubismClippingManager<CubismClippingContext_Cocos2dx, CubismOffscreenSurface_Cocos2dx>* CubismClippingContext_Cocos2dx::GetClippingManager()
{
    return _owner;
}

/*********************************************************************************************************************
*                                      CubismDrawProfile_OpenGL
********************************************************************************************************************/
void CubismRendererProfile_Cocos2dx::Save()
{
    //-- push state --
    _lastScissorTest = GetCocos2dRenderer()->getScissorTest();
    _lastStencilTest = GetCocos2dRenderer()->getStencilTest();
    _lastDepthTest = GetCocos2dRenderer()->getDepthTest();
    _lastCullFace = GetCocos2dRenderer()->getCullMode();
    _lastWinding = GetCocos2dRenderer()->getWinding();


    // モデル描画直前のFBOとビューポートを保存
    _lastColorBuffer = GetCocos2dRenderer()->getColorAttachment();
    _lastDepthBuffer = GetCocos2dRenderer()->getDepthAttachment();
    _lastStencilBuffer = GetCocos2dRenderer()->getStencilAttachment();
    _lastRenderTargetFlag = GetCocos2dRenderer()->getRenderTargetFlag();
    _lastViewport = csmRectF(GetCocos2dRenderer()->getViewport().x, GetCocos2dRenderer()->getViewport().y, GetCocos2dRenderer()->getViewport().w, GetCocos2dRenderer()->getViewport().h);
}

void CubismRendererProfile_Cocos2dx::Restore()
{
    GetCocos2dRenderer()->setScissorTest(_lastScissorTest);
    GetCocos2dRenderer()->setStencilTest(_lastStencilTest);
    GetCocos2dRenderer()->setDepthTest(_lastDepthTest);
    GetCocos2dRenderer()->setCullMode(_lastCullFace);
    GetCocos2dRenderer()->setWinding(_lastWinding);

    GetCocos2dRenderer()->setRenderTarget(_lastRenderTargetFlag, _lastColorBuffer, _lastDepthBuffer, _lastStencilBuffer);
    GetCocos2dRenderer()->setViewPort(_lastViewport.X, _lastViewport.Y, _lastViewport.Width, _lastViewport.Height);
}

/*********************************************************************************************************************
 *                                      CubismRenderer_Cocos2dx
 ********************************************************************************************************************/

#ifdef CSM_TARGET_ANDROID_ES2
void CubismRenderer_Cocos2dx::SetExtShaderMode(csmBool extMode, csmBool extPAMode)
{
    CubismShader_Cocos2dx::SetExtShaderMode(extMode, extPAMode);
    CubismShader_Cocos2dx::DeleteInstance();
}

void CubismRenderer_Cocos2dx::ReloadShader()
{
    CubismShader_Cocos2dx::DeleteInstance();
}
#endif

CubismRenderer* CubismRenderer::Create()
{
    return CSM_NEW CubismRenderer_Cocos2dx();
}

void CubismRenderer::StaticRelease()
{
    CubismRenderer_Cocos2dx::DoStaticRelease();
}

namespace
{
    CubismCommandBuffer_Cocos2dx*       _commandBuffer;
}

CubismRenderer_Cocos2dx::CubismRenderer_Cocos2dx() : _clippingManager(NULL)
                                                     , _clippingContextBufferForMask(NULL)
                                                     , _clippingContextBufferForDraw(NULL)
{
    // テクスチャ対応マップの容量を確保しておく.
    _textures.PrepareCapacity(32, true);
}

CubismRenderer_Cocos2dx::~CubismRenderer_Cocos2dx()
{
    CSM_DELETE_SELF(CubismClippingManager_Cocos2dx, _clippingManager);

    if (_drawableDrawCommandBuffer.GetSize() > 0)
    {
        for (csmInt32 i = 0 ; i < _drawableDrawCommandBuffer.GetSize() ; i++)
        {
            if (_drawableDrawCommandBuffer[i] != NULL)
            {
                CSM_DELETE(_drawableDrawCommandBuffer[i]);
            }
        }
    }

    if (_drawableDrawCommandBuffer.GetSize() > 0)
    {
        _drawableDrawCommandBuffer.Clear();
    }

    if (_textures.GetSize() > 0)
    {
        _textures.Clear();
    }

    for (csmUint32 i = 0; i < _offscreenSurfaces.GetSize(); ++i)
    {
        _offscreenSurfaces[i].DestroyOffscreenSurface();
    }
    _offscreenSurfaces.Clear();
}

void CubismRenderer_Cocos2dx::DoStaticRelease()
{
#ifdef CSM_TARGET_WINGL
    s_isInitializeGlFunctionsSuccess = false;     ///< 初期化が完了したかどうか。trueなら初期化完了
    s_isFirstInitializeGlFunctions = true;        ///< 最初の初期化実行かどうか。trueなら最初の初期化実行
#endif
    CubismShader_Cocos2dx::DeleteInstance();
}

void CubismRenderer_Cocos2dx::Initialize(CubismModel* model)
{
    Initialize(model, 1);
}

void CubismRenderer_Cocos2dx::Initialize(Framework::CubismModel* model, csmInt32 maskBufferCount)
{
    // 1未満は1に補正する
    if (maskBufferCount < 1)
    {
        maskBufferCount = 1;
        CubismLogWarning("The number of render textures must be an integer greater than or equal to 1. Set the number of render textures to 1.");
    }

    if (model->IsUsingMasking())
    {
        _clippingManager = CSM_NEW CubismClippingManager_Cocos2dx();  //クリッピングマスク・バッファ前処理方式を初期化
        _clippingManager->Initialize(
            *model,
            maskBufferCount
        );

        _offscreenSurfaces.Clear();

        for (csmInt32 i = 0; i < maskBufferCount; ++i)
        {
            CubismOffscreenSurface_Cocos2dx OffscreenSurface;
            OffscreenSurface.CreateOffscreenSurface(_clippingManager->GetClippingMaskBufferSize().X, _clippingManager->GetClippingMaskBufferSize().Y);
            _offscreenSurfaces.PushBack(OffscreenSurface);
        }
    }

    _sortedDrawableIndexList.Resize(model->GetDrawableCount(), 0);

    _drawableDrawCommandBuffer.Resize(model->GetDrawableCount());

    for (csmInt32 i = 0; i < _drawableDrawCommandBuffer.GetSize(); ++i)
    {
        const csmInt32 drawableVertexCount = model->GetDrawableVertexCount(i);
        const csmInt32 drawableVertexIndexCount = model->GetDrawableVertexIndexCount(i);
        const csmSizeInt vertexSize = sizeof(csmFloat32) * 2;

        _drawableDrawCommandBuffer[i] = CSM_NEW CubismCommandBuffer_Cocos2dx::DrawCommandBuffer();
        _drawableDrawCommandBuffer[i]->GetCommandDraw()->GetCommand()->setDrawType(cocos2d::CustomCommand::DrawType::ELEMENT);
        _drawableDrawCommandBuffer[i]->GetCommandDraw()->GetCommand()->setPrimitiveType(cocos2d::backend::PrimitiveType::TRIANGLE);
        _drawableDrawCommandBuffer[i]->CreateVertexBuffer(vertexSize, drawableVertexCount * 2);      // Vertices + UVs

        if (drawableVertexIndexCount > 0)
        {
            _drawableDrawCommandBuffer[i]->CreateIndexBuffer(drawableVertexIndexCount);
        }
    }


    CubismRenderer::Initialize(model, maskBufferCount);  //親クラスの処理を呼ぶ
}

void CubismRenderer_Cocos2dx::PreDraw()
{
    _commandBuffer->SetOperationEnable(CubismCommandBuffer_Cocos2dx::OperationType_ScissorTest, false);
    _commandBuffer->SetOperationEnable(CubismCommandBuffer_Cocos2dx::OperationType_StencilTest, false);
    _commandBuffer->SetOperationEnable(CubismCommandBuffer_Cocos2dx::OperationType_DepthTest, false);


    //異方性フィルタリング。プラットフォームのOpenGLによっては未対応の場合があるので、未設定のときは設定しない
    if (GetAnisotropy() > 0.0f)
    {
        // Not supported.
    }
}

void CubismRenderer_Cocos2dx::DoDrawModel()
{
    //------------ クリッピングマスク・バッファ前処理方式の場合 ------------
    if (_clippingManager != NULL)
    {
        PreDraw();

        // サイズが違う場合はここで作成しなおし
        for (csmInt32 i = 0; i < _clippingManager->GetRenderTextureCount(); ++i)
        {
            if (_offscreenSurfaces[i].GetBufferWidth() != _clippingManager->GetClippingMaskBufferSize().X ||
                _offscreenSurfaces[i].GetBufferHeight() != _clippingManager->GetClippingMaskBufferSize().Y)
            {
                _offscreenSurfaces[i].CreateOffscreenSurface(_clippingManager->GetClippingMaskBufferSize().X, _clippingManager->GetClippingMaskBufferSize().Y);
            }
        }

        if(IsUsingHighPrecisionMask())
        {
            _clippingManager->SetupMatrixForHighPrecision(*GetModel(), false);
        }
        else
        {
            _clippingManager->SetupClippingContext(*GetModel(), this, _rendererProfile._lastColorBuffer, _rendererProfile._lastViewport);
        }
    }

    // 上記クリッピング処理内でも一度PreDrawを呼ぶので注意!!
    PreDraw();

    const csmInt32 drawableCount = GetModel()->GetDrawableCount();
    const csmInt32* renderOrder = GetModel()->GetDrawableRenderOrders();

    // インデックスを描画順でソート
    for (csmInt32 i = 0; i < drawableCount; ++i)
    {
        const csmInt32 order = renderOrder[i];
        _sortedDrawableIndexList[order] = i;
    }

    // Update Vertex / Index buffer.
    for (csmInt32 i = 0; i < drawableCount; ++i)
    {
        csmFloat32* vertices = const_cast<csmFloat32*>(GetModel()->GetDrawableVertices(i));
        Core::csmVector2* uvs = const_cast<Core::csmVector2*>(GetModel()->GetDrawableVertexUvs(i));
        csmUint16* vertexIndices = const_cast<csmUint16*>(GetModel()->GetDrawableVertexIndices(i));
        const csmUint32 vertexCount = GetModel()->GetDrawableVertexCount(i);
        const csmUint32 vertexIndexCount = GetModel()->GetDrawableVertexIndexCount(i);

        _drawableDrawCommandBuffer[i]->UpdateVertexBuffer(vertices, uvs, vertexCount);
        _drawableDrawCommandBuffer[i]->CommitVertexBuffer();
        if (vertexIndexCount > 0)
        {
            _drawableDrawCommandBuffer[i]->UpdateIndexBuffer(vertexIndices, vertexIndexCount);
        }

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

        // クリッピングマスク
        CubismClippingContext_Cocos2dx* clipContext = (_clippingManager != NULL)
            ? (*_clippingManager->GetClippingContextListForDraw())[drawableIndex]
            : NULL;

        if (clipContext != NULL && IsUsingHighPrecisionMask()) // マスクを書く必要がある
        {
            if(clipContext->_isUsing) // 書くことになっていた
            {
                // 生成したOffscreenSurfaceと同じサイズでビューポートを設定
                _commandBuffer->Viewport(0, 0, _offscreenSurfaces[ clipContext->_bufferIndex].GetViewPortSize().Width, _offscreenSurfaces[ clipContext->_bufferIndex].GetViewPortSize().Height);

                PreDraw(); // バッファをクリアする

                _offscreenSurfaces[ clipContext->_bufferIndex].BeginDraw(_commandBuffer, _rendererProfile._lastColorBuffer);

                // マスクをクリアする
                // 1が無効（描かれない）領域、0が有効（描かれる）領域。（シェーダで Cd*Csで0に近い値をかけてマスクを作る。1をかけると何も起こらない）
                _offscreenSurfaces[ clipContext->_bufferIndex].Clear(_commandBuffer, 1.0f, 1.0f, 1.0f, 1.0f);
            }

            {
                const csmInt32 clipDrawCount = clipContext->_clippingIdCount;
                for (csmInt32 index = 0; index < clipDrawCount; index++)
                {
                    const csmInt32 clipDrawIndex = clipContext->_clippingIdList[index];
                    CubismCommandBuffer_Cocos2dx::DrawCommandBuffer* drawCommandBufferMask = clipContext->_clippingCommandBufferList->At(index);

                    // 頂点情報が更新されておらず、信頼性がない場合は描画をパスする
                    if (!GetModel()->GetDrawableDynamicFlagVertexPositionsDidChange(clipDrawIndex))
                    {
                        continue;
                    }

                    IsCulling(GetModel()->GetDrawableCulling(clipDrawIndex) != 0);

                    if (GetModel()->GetDrawableVertexIndexCount(clipDrawIndex) <= 0)
                    {
                        continue;
                    }

                    // Update Vertex / Index buffer.
                    {
                        csmFloat32* vertices = const_cast<csmFloat32*>(GetModel()->GetDrawableVertices(clipDrawIndex));
                        Core::csmVector2* uvs = const_cast<Core::csmVector2*>(GetModel()->GetDrawableVertexUvs(clipDrawIndex));
                        csmUint16* vertexIndices = const_cast<csmUint16*>(GetModel()->GetDrawableVertexIndices(clipDrawIndex));
                        const csmUint32 vertexCount = GetModel()->GetDrawableVertexCount(clipDrawIndex);
                        const csmUint32 vertexIndexCount = GetModel()->GetDrawableVertexIndexCount(clipDrawIndex);

                        drawCommandBufferMask->UpdateVertexBuffer(vertices, uvs, vertexCount);
                        drawCommandBufferMask->CommitVertexBuffer();
                        if (vertexIndexCount > 0)
                        {
                            drawCommandBufferMask->UpdateIndexBuffer(vertexIndices, vertexIndexCount);
                        }

                        if (vertexCount <= 0)
                        {
                            continue;
                        }

                    }

                    // 今回専用の変換を適用して描く
                    // チャンネルも切り替える必要がある(A,R,G,B)
                    SetClippingContextBufferForMask(clipContext);
                    DrawMeshCocos2d(drawCommandBufferMask->GetCommandDraw(), *GetModel(), clipDrawIndex);
                }
            }

            {
                // --- 後処理 ---
                _offscreenSurfaces[ clipContext->_bufferIndex].EndDraw(_commandBuffer);
                SetClippingContextBufferForMask(NULL);
                _commandBuffer->Viewport(_rendererProfile._lastViewport.X, _rendererProfile._lastViewport.Y, _rendererProfile._lastViewport.Width, _rendererProfile._lastViewport.Height);

                PreDraw(); // バッファをクリアする
            }
        }

        CubismCommandBuffer_Cocos2dx::DrawCommandBuffer::DrawCommand* drawCommandDraw = _drawableDrawCommandBuffer[drawableIndex]->GetCommandDraw();

        // クリッピングマスクをセットする
        SetClippingContextBufferForDraw(clipContext);

        IsCulling(GetModel()->GetDrawableCulling(drawableIndex) != 0);

        if (GetModel()->GetDrawableVertexIndexCount(drawableIndex) <= 0)
        {
            continue;
        }

        DrawMeshCocos2d(drawCommandDraw, *GetModel(), drawableIndex);
    }

    PostDraw();
}

void CubismRenderer_Cocos2dx::DrawMeshCocos2d(CubismCommandBuffer_Cocos2dx::DrawCommandBuffer::DrawCommand* drawCommand
                                            ,const CubismModel& model, const csmInt32 index)
{
#ifndef CSM_DEBUG
    if (_textures[model.GetDrawableTextureIndex(index)] == 0) return;    // モデルが参照するテクスチャがバインドされていない場合は描画をスキップする
#endif

    // 裏面描画の有効・無効
    _commandBuffer->SetOperationEnable(CubismCommandBuffer_Cocos2dx::OperationType_Culling, (IsCulling() ? true : false));

    // Cubism SDK OpenGLはマスク・アートメッシュ共にCCWが表面
    _commandBuffer->SetWindingMode(CubismCommandBuffer_Cocos2dx::WindingType_CounterClockWise);

    if (IsGeneratingMask()) // マスク生成時
    {
        CubismShader_Cocos2dx::GetInstance()->SetupShaderProgramForMask(drawCommand, this, model, index);
    }
    else
    {
        CubismShader_Cocos2dx::GetInstance()->SetupShaderProgramForDraw(drawCommand, this, model, index);
    }

    // ポリゴンメッシュを描画する
    _commandBuffer->AddDrawCommand(drawCommand);

    // 後処理
    SetClippingContextBufferForDraw(NULL);
    SetClippingContextBufferForMask(NULL);
}

CubismCommandBuffer_Cocos2dx* CubismRenderer_Cocos2dx::GetCommandBuffer()
{
    return _commandBuffer;
}

void CubismRenderer_Cocos2dx::StartFrame(CubismCommandBuffer_Cocos2dx* commandBuffer)
{
    _commandBuffer = commandBuffer;
}

void CubismRenderer_Cocos2dx::EndFrame(CubismCommandBuffer_Cocos2dx* commandBuffer)
{
}

CubismCommandBuffer_Cocos2dx::DrawCommandBuffer* CubismRenderer_Cocos2dx::GetDrawCommandBufferData(csmInt32 drawableIndex)
{
    return _drawableDrawCommandBuffer[drawableIndex];
}

void CubismRenderer_Cocos2dx::SaveProfile()
{
    _rendererProfile.Save();
}

void CubismRenderer_Cocos2dx::RestoreProfile()
{
    _rendererProfile.Restore();
}

void CubismRenderer_Cocos2dx::BindTexture(csmUint32 modelTextureIndex, cocos2d::Texture2D* texture)
{
    _textures[modelTextureIndex] = texture;
}

const csmMap<csmInt32, cocos2d::Texture2D*>& CubismRenderer_Cocos2dx::GetBindedTextures() const
{
    return _textures;
}

void CubismRenderer_Cocos2dx::SetClippingMaskBufferSize(csmFloat32 width, csmFloat32 height)
{
    if (_clippingManager == NULL)
    {
        return;
    }

    // インスタンス破棄前にレンダーテクスチャの数を保存
    const csmInt32 renderTextureCount = _clippingManager->GetRenderTextureCount();

    //OffscreenSurfaceのサイズを変更するためにインスタンスを破棄・再作成する
    CSM_DELETE_SELF(CubismClippingManager_Cocos2dx, _clippingManager);

    _clippingManager = CSM_NEW CubismClippingManager_Cocos2dx();

    _clippingManager->SetClippingMaskBufferSize(width, height);

    _clippingManager->Initialize(
        *GetModel(),
        renderTextureCount
    );
}

csmInt32 CubismRenderer_Cocos2dx::GetRenderTextureCount() const
{
    return _clippingManager->GetRenderTextureCount();
}

CubismVector2 CubismRenderer_Cocos2dx::GetClippingMaskBufferSize() const
{
    return _clippingManager->GetClippingMaskBufferSize();
}

CubismOffscreenSurface_Cocos2dx* CubismRenderer_Cocos2dx::GetOffscreenSurface(csmInt32 index)
{
    return &_offscreenSurfaces[index];
}

void CubismRenderer_Cocos2dx::SetClippingContextBufferForMask(CubismClippingContext_Cocos2dx* clip)
{
    _clippingContextBufferForMask = clip;
}

CubismClippingContext_Cocos2dx* CubismRenderer_Cocos2dx::GetClippingContextBufferForMask() const
{
    return _clippingContextBufferForMask;
}

void CubismRenderer_Cocos2dx::SetClippingContextBufferForDraw(CubismClippingContext_Cocos2dx* clip)
{
    _clippingContextBufferForDraw = clip;
}

CubismClippingContext_Cocos2dx* CubismRenderer_Cocos2dx::GetClippingContextBufferForDraw() const
{
    return _clippingContextBufferForDraw;
}

const csmBool CubismRenderer_Cocos2dx::IsGeneratingMask() const
{
    return GetClippingContextBufferForMask() != NULL;
}

cocos2d::Texture2D* CubismRenderer_Cocos2dx::GetBindedTexture(csmInt32 textureIndex)
{
    return _textures[textureIndex];
}

}}}}

//------------ LIVE2D NAMESPACE ------------
