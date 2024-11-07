/**
 * Copyright(c) Live2D Inc. All rights reserved.
 *
 * Use of this source code is governed by the Live2D Open Software license
 * that can be found at https://www.live2d.com/eula/live2d-open-software-license-agreement_en.html.
 */

#include "CubismExpressionMotionManager.hpp"
#include "CubismExpressionMotion.hpp"
#include "CubismMotionQueueEntry.hpp"
#include "CubismFramework.hpp"
#include "Math/CubismMath.hpp"

namespace Live2D { namespace Cubism { namespace Framework {

CubismExpressionMotionManager::CubismExpressionMotionManager()
    : _currentPriority(0)
    , _reservePriority(0)
    , _expressionParameterValues(CSM_NEW csmVector<ExpressionParameterValue>())
{ }

CubismExpressionMotionManager::~CubismExpressionMotionManager()
{
    if(_expressionParameterValues)
    {
        CSM_DELETE(_expressionParameterValues);

        _expressionParameterValues = NULL;
    }

    _fadeWeights.Clear();
}

csmInt32 CubismExpressionMotionManager::GetCurrentPriority() const
{
    return _currentPriority;
}

csmInt32 CubismExpressionMotionManager::GetReservePriority() const
{
    return _reservePriority;
}

void CubismExpressionMotionManager::SetReservePriority(csmInt32 priority)
{
    _reservePriority = priority;
}

CubismMotionQueueEntryHandle CubismExpressionMotionManager::StartMotionPriority(ACubismMotion* motion, csmBool autoDelete, csmInt32 priority)
{
    if (priority == _reservePriority)
    {
        _reservePriority = 0;           // 予約を解除
    }
    _currentPriority = priority;        // 再生中モーションの優先度を設定

    _fadeWeights.PushBack(0.0f);

    return CubismMotionQueueManager::StartMotion(motion, autoDelete);
}

csmBool CubismExpressionMotionManager::UpdateMotion(CubismModel* model, csmFloat32 deltaTimeSeconds)
{
    _userTimeSeconds += deltaTimeSeconds;
    csmBool updated = false;
    csmVector<CubismMotionQueueEntry*>* motions = GetCubismMotionQueueEntries();

    csmFloat32 expressionWeight = 0.0f;
    csmInt32 expressionIndex = 0;

    // ------- 処理を行う --------
    // 既にモーションがあれば終了フラグを立てる
    for (csmVector<CubismMotionQueueEntry*>::iterator ite = motions->Begin(); ite != motions->End();)
    {
        CubismMotionQueueEntry* motionQueueEntry = *ite;

        if (motionQueueEntry == NULL)
        {
            ite = motions->Erase(ite);          // 削除
            continue;
        }

        CubismExpressionMotion* expressionMotion = (CubismExpressionMotion*)motionQueueEntry->GetCubismMotion();

        if (expressionMotion == NULL)
        {
            CSM_DELETE(motionQueueEntry);
            ite = motions->Erase(ite);          // 削除
            continue;
        }

        csmVector<CubismExpressionMotion::ExpressionParameter> expressionParameters = expressionMotion->GetExpressionParameters();
        if (motionQueueEntry->IsAvailable())
        {
            // 再生中のExpressionが参照しているパラメータをすべてリストアップ
            for (csmInt32 i = 0; i < expressionParameters.GetSize(); ++i)
            {
                if (expressionParameters[i].ParameterId == NULL)
                {
                    continue;
                }

                csmInt32 index = -1;
                // リストにパラメータIDが存在するか検索
                for (csmInt32 j = 0; j < _expressionParameterValues->GetSize(); ++j)
                {
                    if (_expressionParameterValues->At(j).ParameterId != expressionParameters[i].ParameterId)
                    {
                        continue;
                    }

                    index = j;
                    break;
                }

                if (index >= 0)
                {
                    continue;
                }

                // パラメータがリストに存在しないなら新規追加
                ExpressionParameterValue item;
                item.ParameterId = expressionParameters[i].ParameterId;
                item.AdditiveValue = CubismExpressionMotion::DefaultAdditiveValue;
                item.MultiplyValue = CubismExpressionMotion::DefaultMultiplyValue;
                item.OverwriteValue = model->GetParameterValue(item.ParameterId);
                _expressionParameterValues->PushBack(item);
            }
        }

        // ------ 値を計算する ------
        expressionMotion->SetupMotionQueueEntry(motionQueueEntry, _userTimeSeconds);
        _fadeWeights[expressionIndex] = expressionMotion->UpdateFadeWeight(motionQueueEntry, _userTimeSeconds);
        expressionMotion->CalculateExpressionParameters(model, _userTimeSeconds, motionQueueEntry,
            _expressionParameterValues, expressionIndex, _fadeWeights[expressionIndex]);

        expressionWeight += expressionMotion->GetFadeInTime() == 0.0f
            ? 1.0f
            : CubismMath::GetEasingSine((_userTimeSeconds - motionQueueEntry->GetFadeInStartTime()) / expressionMotion->GetFadeInTime());

        updated = true;

        if (motionQueueEntry->IsTriggeredFadeOut())
        {
            // フェードアウト開始
            motionQueueEntry->StartFadeout(motionQueueEntry->GetFadeOutSeconds(), _userTimeSeconds);
        }

        ++ite;
        ++expressionIndex;
    }

    // ----- 最新のExpressionのフェードが完了していればそれ以前を削除する ------
    if (motions->GetSize() > 1)
    {
        CubismExpressionMotion* expressionMotion =
            (CubismExpressionMotion*)(motions->At(motions->GetSize() - 1))->GetCubismMotion();

        csmFloat32 latestFadeWeight = _fadeWeights[_fadeWeights.GetSize() - 1];
        if (latestFadeWeight >= 1.0f)
        {
            // 配列の最後の要素は削除しない
            for (csmInt32 i = motions->GetSize()-2; i >= 0; i--)
            {
                CubismMotionQueueEntry* motionQueueEntry = motions->At(i);
                CSM_DELETE(motionQueueEntry);
                motions->Remove(i);
                _fadeWeights.Remove(i);
            }
        }
    }

    if (expressionWeight > 1.0f)
    {
        expressionWeight = 1.0f;
    }

    // モデルに各値を適用
    for (csmInt32 i = 0; i < _expressionParameterValues->GetSize(); ++i)
    {
        model->SetParameterValue(_expressionParameterValues->At(i).ParameterId,
            (_expressionParameterValues->At(i).OverwriteValue + _expressionParameterValues->At(i).AdditiveValue) * _expressionParameterValues->At(i).MultiplyValue,
            expressionWeight);

        _expressionParameterValues->At(i).AdditiveValue = CubismExpressionMotion::DefaultAdditiveValue;
        _expressionParameterValues->At(i).MultiplyValue = CubismExpressionMotion::DefaultMultiplyValue;
    }

    return updated;
}

csmFloat32 CubismExpressionMotionManager::GetFadeWeight(csmInt32 index)
{
    return _fadeWeights[index];
}

}}}
