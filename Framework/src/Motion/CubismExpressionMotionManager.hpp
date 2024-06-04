/**
 * Copyright(c) Live2D Inc. All rights reserved.
 *
 * Use of this source code is governed by the Live2D Open Software license
 * that can be found at https://www.live2d.com/eula/live2d-open-software-license-agreement_en.html.
 */

#pragma once

#include "Model/CubismModel.hpp"
#include "ACubismMotion.hpp"
#include "CubismMotionQueueManager.hpp"

namespace Live2D { namespace Cubism { namespace Framework {

/**
 * @brief 表情モーションの管理
 *
 * 表情モーションの管理を行うクラス。
 */
class CubismExpressionMotionManager : public CubismMotionQueueManager
{
public:
    /**
     * @brief パラメータに適用する表情の値を持たせる構造体
     */
    struct ExpressionParameterValue
    {
        CubismIdHandle      ParameterId;        ///< パラメータID
        csmFloat32          AdditiveValue;      ///< 加算値
        csmFloat32          MultiplyValue;      ///< 乗算値
        csmFloat32          OverwriteValue;           ///< 上書き値
    };

    /**
     * @brief コンストラクタ
     *
     * コンストラクタ。
     */
    CubismExpressionMotionManager();

    /**
     * @brief デストラクタ
     *
     * デストラクタ。
     */
    virtual ~CubismExpressionMotionManager();

    /**
     * @brief 再生中のモーションの優先度の取得
     *
     * 再生中の表情モーションの優先度の取得する。
     *
     * @return  表情モーションの優先度
     */
    csmInt32 GetCurrentPriority() const;

    /**
     * @brief 予約中のモーションの優先度の取得
     *
     * 予約中の表情モーションの優先度を取得する。
     *
     * @return  表情モーションの優先度
     */
    csmInt32 GetReservePriority() const;

    /**
     * @brief 予約中のモーションの優先度の設定
     *
     * 予約中の表情モーションの優先度を設定する。
     *
     * @param[in]   priority     優先度
     */
    void SetReservePriority(csmInt32 priority);

    /**
     * @brief 優先度を設定して表情モーションの開始
     *
     * 優先度を設定して表情モーションを開始する。
     *
     * @param[in]   motion          モーション
     * @param[in]   autoDelete      再生が終了したモーションのインスタンスを削除するならtrue
     * @param[in]   priority        優先度
     * @return                      開始したモーションの識別番号を返す。個別のモーションが終了したか否かを判定するIsFinished()の引数で使用する。開始できない時は「-1」
     */
    CubismMotionQueueEntryHandle StartMotionPriority(ACubismMotion* motion, csmBool autoDelete, csmInt32 priority);

    /**
     * @brief モーションの更新
     *
     * 表情モーションを更新して、モデルにパラメータ値を反映する。
     *
     * @param[in]   model   対象のモデル
     * @param[in]   deltaTimeSeconds    デルタ時間[秒]
     * @retval  true    更新されている
     * @retval  false   更新されていない
     */
    csmBool UpdateMotion(CubismModel* model, csmFloat32 deltaTimeSeconds);

    /**
     * 現在の表情のフェードのウェイト値を取得する。
     *
     * @param index 取得する表情モーションのインデックス
     *
     * @return 表情のフェードのウェイト値
     */
    csmFloat32 GetFadeWeight(csmInt32 index);

private:

    // モデルに適用する各パラメータの値
    csmVector<ExpressionParameterValue>* _expressionParameterValues;

    // 再生中の表情のウェイト
    csmVector<csmFloat32> _fadeWeights;

    csmInt32 _currentPriority;                  ///<  現在再生中のモーションの優先度
    csmInt32 _reservePriority;                  ///<  再生予定のモーションの優先度。再生中は0になる。モーションファイルを別スレッドで読み込むときの機能。
};

}}}
