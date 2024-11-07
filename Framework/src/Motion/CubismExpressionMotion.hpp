/**
 * Copyright(c) Live2D Inc. All rights reserved.
 *
 * Use of this source code is governed by the Live2D Open Software license
 * that can be found at https://www.live2d.com/eula/live2d-open-software-license-agreement_en.html.
 */

#pragma once

#include "ACubismMotion.hpp"
#include "Utils/CubismJson.hpp"
#include "Model/CubismModel.hpp"
#include "CubismExpressionMotionManager.hpp"

namespace Live2D { namespace Cubism { namespace Framework {

/**
 * @brief 表情のモーション
 *
 * 表情のモーションクラス。
 */
class CubismExpressionMotion : public ACubismMotion
{
public:
    /**
     * @brief 表情パラメータ値の計算方式
     *
     */
    enum ExpressionBlendType
    {
        Additive = 0,   ///< 加算
        Multiply = 1,   ///< 乗算
        Overwrite = 2   ///< 上書き
    };

    /**
     * @brief 表情のパラメータ情報
     *
     * 表情のパラメータ情報の構造体。
     */
    struct ExpressionParameter
    {
        CubismIdHandle      ParameterId;        ///< パラメータID
        ExpressionBlendType BlendType;          ///< パラメータの演算種類
        csmFloat32          Value;              ///< 値
    };

    /**
     * @brief インスタンスの作成
     *
     * インスタンスを作成する。
     *
     * @param[in]   buf     expファイルが読み込まれているバッファ
     * @param[in]   size    バッファのサイズ
     * @return  作成されたインスタンス
     */
    static CubismExpressionMotion* Create(const csmByte* buf, csmSizeInt size);

    /**
    * @brief モデルのパラメータの更新の実行
    *
    * モデルのパラメータ更新を実行する。
    *
    * @param[in]   model               対象のモデル
    * @param[in]   userTimeSeconds     デルタ時間の積算値[秒]
    * @param[in]   weight              モーションの重み
    * @param[in]   motionQueueEntry    CubismMotionQueueManagerで管理されているモーション
    */
    virtual void DoUpdateParameters(CubismModel* model, csmFloat32 userTimeSeconds, csmFloat32 weight, CubismMotionQueueEntry* motionQueueEntry);

    /**
    * @brief 表情によるモデルのパラメータの計算
    *
    * モデルの表情に関するパラメータを計算する。
    *
    * @param[in]   model                        対象のモデル
    * @param[in]   userTimeSeconds              デルタ時間の積算値[秒]
    * @param[in]   motionQueueEntry             CubismMotionQueueManagerで管理されているモーション
    * @param[in]   expressionParameterValues    モデルに適用する各パラメータの値
    * @param[in]   expressionIndex              表情のインデックス
    */
    void CalculateExpressionParameters(CubismModel* model, csmFloat32 userTimeSeconds, CubismMotionQueueEntry* motionQueueEntry,
        csmVector<CubismExpressionMotionManager::ExpressionParameterValue>* expressionParameterValues, csmInt32 expressionIndex, csmFloat32 fadeWeight);

    /**
     * @brief 表情が参照しているパラメータを取得
     *
     * 表情が参照しているパラメータを取得する。
     */
    csmVector<ExpressionParameter> GetExpressionParameters();

    /**
     * @brief 表情のフェードの値を取得
     *
     * 現在の表情のフェードのウェイト値を取得する。
     *
     * @return 表情のフェードのウェイト値
     *
     * @deprecated CubismExpressionMotion._fadeWeightが削除予定のため非推奨
     * CubismExpressionMotionManager.getFadeWeight(int index) を使用してください。
     *
     * @see CubismExpressionMotionManager#GetFadeWeight(int index)
     */
    csmFloat32 GetFadeWeight();

    static const csmFloat32 DefaultAdditiveValue;    ///<  加算適用の初期値
    static const csmFloat32 DefaultMultiplyValue;    ///<  乗算適用の初期値

protected:
    /**
    * @brief コンストラクタ
    *
    * コンストラクタ。
    */
    CubismExpressionMotion();

    /**
    * @brief デストラクタ
    *
    * デストラクタ。
    */
    virtual ~CubismExpressionMotion();

    /**
     * @brief exp3.jsonのパース
     *
     * exp3.jsonをパースする。
     *
     * @param[in]   exp3Json    exp3.jsonが読み込まれているバッファ
     * @param[in]   size        バッファのサイズ
     */
    void Parse(const csmByte* exp3Json, csmSizeInt size);

    csmVector<ExpressionParameter> _parameters;     ///< 表情が参照しているパラメータ一覧

private:

    /**
     * @brief ブレンド計算
     *
     * 入力された値でブレンド計算をする。
     *
     * @param[in]   source          現在の値
     * @param[in]   destination     適用する値
     */
    csmFloat32 CalculateValue(csmFloat32 source, csmFloat32 destination, csmFloat32 fadeWeight);


    /**
     * 表情の現在のウェイト
     *
     * @deprecated 不具合を引き起こす要因となるため非推奨。
     */
    csmFloat32 _fadeWeight;
};

}}}
