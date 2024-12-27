﻿#include "LAppModel.hpp"
/**
 * Copyright(c) Live2D Inc. All rights reserved.
 *
 * Use of this source code is governed by the Live2D Open Software license
 * that can be found at https://www.live2d.com/eula/live2d-open-software-license-agreement_en.html.
 */

#include <algorithm>
#include <fstream>
#include <vector>
#include <CubismModelSettingJson.hpp>
#include <Motion/CubismMotion.hpp>
#include <Physics/CubismPhysics.hpp>
#include <CubismDefaultParameterId.hpp>
#include <Rendering/OpenGL/CubismRenderer_OpenGLES2.hpp>
#include <Utils/CubismString.hpp>
#include <Id/CubismIdManager.hpp>
#include <Motion/CubismMotionQueueEntry.hpp>
#include "LAppDefine.hpp"
#include "LAppPal.hpp"
#include "LAppTextureManager.hpp"

#include <Log.hpp>
#include <filesystem>
#include <unordered_set>

#include "Default.hpp"

using namespace Live2D::Cubism::Framework;
using namespace Live2D::Cubism::Framework::DefaultParameterId;
using namespace LAppDefine;

namespace
{
    csmByte* CreateBuffer(const csmChar* path, csmSizeInt* size)
    {
        Info("create buffer: %s ", path);
        return LAppPal::LoadFileAsBytes(path, size);
    }

    void DeleteBuffer(csmByte* buffer, const csmChar* path = "")
    {
        Info("delete buffer: %s", path);
        LAppPal::ReleaseBytes(buffer);
    }
}


class FakeMotion : public ACubismMotion
{
protected:
    void DoUpdateParameters(CubismModel* model, csmFloat32 userTimeSeconds, csmFloat32 weight,
                            CubismMotionQueueEntry* motionQueueEntry) override
    {
    }

public:
    FakeMotion()
    {
    }
};

LAppModel::LAppModel()
    : CubismUserModel(), _modelSetting(NULL), _userTimeSeconds(0.0f), _autoBlink(true), _autoBreath(true),
      _matrixManager()
{
    _mocConsistency = MocConsistencyValidationEnable;

    _debugMode = DebugLogEnable;

    _idParamAngleX = CubismFramework::GetIdManager()->GetId(ParamAngleX);
    _idParamAngleY = CubismFramework::GetIdManager()->GetId(ParamAngleY);
    _idParamAngleZ = CubismFramework::GetIdManager()->GetId(ParamAngleZ);
    _idParamBodyAngleX = CubismFramework::GetIdManager()->GetId(ParamBodyAngleX);
    _idParamEyeBallX = CubismFramework::GetIdManager()->GetId(ParamEyeBallX);
    _idParamEyeBallY = CubismFramework::GetIdManager()->GetId(ParamEyeBallY);
}

LAppModel::~LAppModel()
{
    _renderBuffer.DestroyOffscreenSurface();

    ReleaseMotions();
    ReleaseExpressions();

    if (_modelSetting == nullptr)
        return;

    for (csmInt32 i = 0; i < _modelSetting->GetMotionGroupCount(); i++)
    {
        const csmChar* group = _modelSetting->GetMotionGroupName(i);
        ReleaseMotionGroup(group);
    }
    delete (_modelSetting);
}

void LAppModel::LoadAssets(const csmChar* fileName)
{
    // linux 下不支持对 "XXX/XXX.model.json/../" 的解析
    // 因此改用 cpp17 的标准库
    std::filesystem::path p = std::filesystem::u8path(fileName);
    // ensure trailing EOS
    _modelHomeDir = p.parent_path().generic_u8string().c_str();
    _modelHomeDir += "/";

    Info("load model setting: %s", fileName);

    csmSizeInt size;
    const csmString path = fileName;

    csmByte* buffer = CreateBuffer(path.GetRawString(), &size);
    ICubismModelSetting* setting = new CubismModelSettingJson(buffer, size);
    DeleteBuffer(buffer, path.GetRawString());

    SetupModel(setting);

    if (_model == NULL)
    {
        Info("Failed to LoadAssets().");
        return;
    }

    CreateRenderer();

    SetupTextures();
}

void LAppModel::SetupModel(ICubismModelSetting* setting)
{
    _updating = true;
    _initialized = false;

    _modelSetting = setting;

    csmByte* buffer;
    csmSizeInt size;

    // Cubism Model
    if (strcmp(_modelSetting->GetModelFileName(), "") != 0)
    {
        csmString path = _modelSetting->GetModelFileName();
        path = _modelHomeDir + path;

        if (_debugMode)
        {
            Info("create model: %s", setting->GetModelFileName());
        }

        buffer = CreateBuffer(path.GetRawString(), &size);
        LoadModel(buffer, size, _mocConsistency);
        DeleteBuffer(buffer, path.GetRawString());
    }

    // Expression
    if (_modelSetting->GetExpressionCount() > 0)
    {
        const csmInt32 count = _modelSetting->GetExpressionCount();
        for (csmInt32 i = 0; i < count; i++)
        {
            csmString name = _modelSetting->GetExpressionName(i);
            csmString path = _modelSetting->GetExpressionFileName(i);
            path = _modelHomeDir + path;

            buffer = CreateBuffer(path.GetRawString(), &size);
            ACubismMotion* motion = LoadExpression(buffer, size, name.GetRawString());

            if (motion)
            {
                if (_expressions[name] != NULL)
                {
                    ACubismMotion::Delete(_expressions[name]);
                    _expressions[name] = NULL;
                }
                _expressions[name] = motion;
            }

            DeleteBuffer(buffer, path.GetRawString());
        }
    }

    // Physics
    if (strcmp(_modelSetting->GetPhysicsFileName(), "") != 0)
    {
        csmString path = _modelSetting->GetPhysicsFileName();
        path = _modelHomeDir + path;

        buffer = CreateBuffer(path.GetRawString(), &size);
        LoadPhysics(buffer, size);
        DeleteBuffer(buffer, path.GetRawString());
    }

    // Pose
    if (strcmp(_modelSetting->GetPoseFileName(), "") != 0)
    {
        csmString path = _modelSetting->GetPoseFileName();
        path = _modelHomeDir + path;

        buffer = CreateBuffer(path.GetRawString(), &size);
        LoadPose(buffer, size);
        DeleteBuffer(buffer, path.GetRawString());
    }

    // EyeBlink
    if (_modelSetting->GetEyeBlinkParameterCount() > 0)
    {
        _eyeBlink = CubismEyeBlink::Create(_modelSetting);
    }

    // Breath
    {
        _breath = CubismBreath::Create();

        csmVector<CubismBreath::BreathParameterData> breathParameters;

        breathParameters.PushBack(CubismBreath::BreathParameterData(_idParamAngleX, 0.0f, 15.0f, 6.5345f, 0.5f));
        breathParameters.PushBack(CubismBreath::BreathParameterData(_idParamAngleY, 0.0f, 8.0f, 3.5345f, 0.5f));
        breathParameters.PushBack(CubismBreath::BreathParameterData(_idParamAngleZ, 0.0f, 10.0f, 5.5345f, 0.5f));
        breathParameters.PushBack(CubismBreath::BreathParameterData(_idParamBodyAngleX, 0.0f, 4.0f, 15.5345f, 0.5f));
        breathParameters.PushBack(
            CubismBreath::BreathParameterData(CubismFramework::GetIdManager()->GetId(ParamBreath), 0.5f, 0.5f, 3.2345f,
                                              0.5f));

        _breath->SetParameters(breathParameters);
    }

    // UserData
    if (strcmp(_modelSetting->GetUserDataFile(), "") != 0)
    {
        csmString path = _modelSetting->GetUserDataFile();
        path = _modelHomeDir + path;
        buffer = CreateBuffer(path.GetRawString(), &size);
        LoadUserData(buffer, size);
        DeleteBuffer(buffer, path.GetRawString());
    }

    // EyeBlinkIds
    {
        csmInt32 eyeBlinkIdCount = _modelSetting->GetEyeBlinkParameterCount();
        for (csmInt32 i = 0; i < eyeBlinkIdCount; ++i)
        {
            _eyeBlinkIds.PushBack(_modelSetting->GetEyeBlinkParameterId(i));
        }
    }

    // LipSyncIds
    {
        csmInt32 lipSyncIdCount = _modelSetting->GetLipSyncParameterCount();
        for (csmInt32 i = 0; i < lipSyncIdCount; ++i)
        {
            _lipSyncIds.PushBack(_modelSetting->GetLipSyncParameterId(i));
        }
    }

    if (_modelSetting == NULL || _modelMatrix == NULL)
    {
        Info("Failed to SetupModel().");
        return;
    }

    // Layout
    csmMap<csmString, csmFloat32> layout;
    _modelSetting->GetLayoutMap(layout);
    _modelMatrix->SetupFromLayout(layout);

    _model->SaveParameters();

    for (csmInt32 i = 0; i < _modelSetting->GetMotionGroupCount(); i++)
    {
        const csmChar* group = _modelSetting->GetMotionGroupName(i);
        PreloadMotionGroup(group);
    }

    _motionManager->StopAllMotions();

    _updating = false;
    _initialized = true;
}

void LAppModel::PreloadMotionGroup(const csmChar* group)
{
    const csmInt32 count = _modelSetting->GetMotionCount(group);

    for (csmInt32 i = 0; i < count; i++)
    {
        // ex) idle_0
        csmString name = Utils::CubismString::GetFormatedString("%s_%d", group, i);
        csmString path = _modelSetting->GetMotionFileName(group, i);

        // 定义了动作但是没有动作路径
        if (path.GetLength() == 0)
        {
            Info("load motion without file: %s => [%s_%d] ", path.GetRawString(), group, i);
            continue;
        }
        else
        {
            Info("load motion: %s => [%s_%d] ", path.GetRawString(), group, i);
        }

        path = _modelHomeDir + path;

        csmByte* buffer;
        csmSizeInt size;
        buffer = CreateBuffer(path.GetRawString(), &size);
        CubismMotion* tmpMotion = static_cast<CubismMotion*>(LoadMotion(buffer, size, name.GetRawString()));

        if (tmpMotion)
        {
            csmFloat32 fadeTime = _modelSetting->GetMotionFadeInTimeValue(group, i);
            if (fadeTime >= 0.0f)
            {
                tmpMotion->SetFadeInTime(fadeTime);
            }

            fadeTime = _modelSetting->GetMotionFadeOutTimeValue(group, i);
            if (fadeTime >= 0.0f)
            {
                tmpMotion->SetFadeOutTime(fadeTime);
            }
            tmpMotion->SetEffectIds(_eyeBlinkIds, _lipSyncIds);

            if (_motions[name] != NULL)
            {
                ACubismMotion::Delete(_motions[name]);
            }
            _motions[name] = tmpMotion;
        }

        DeleteBuffer(buffer, path.GetRawString());
    }
}

void LAppModel::ReleaseMotionGroup(const csmChar* group) const
{
    const csmInt32 count = _modelSetting->GetMotionCount(group);
    for (csmInt32 i = 0; i < count; i++)
    {
        csmString voice = _modelSetting->GetMotionSoundFileName(group, i);
        if (strcmp(voice.GetRawString(), "") != 0)
        {
            csmString path = voice;
            path = _modelHomeDir + path;
        }
    }
}

/**
 * @brief すべてのモーションデータの解放
 *
 * すべてのモーションデータを解放する。
 */
void LAppModel::ReleaseMotions()
{
    for (csmMap<csmString, ACubismMotion*>::const_iterator iter = _motions.Begin(); iter != _motions.End(); ++iter)
    {
        ACubismMotion::Delete(iter->Second);
    }

    _motions.Clear();
}

/**
 * @brief すべての表情データの解放
 *
 * すべての表情データを解放する。
 */
void LAppModel::ReleaseExpressions()
{
    for (csmMap<csmString, ACubismMotion*>::const_iterator iter = _expressions.Begin(); iter != _expressions.End(); ++
         iter)
    {
        ACubismMotion::Delete(iter->Second);
    }

    _expressions.Clear();
}

void LAppModel::Update()
{
    LAppPal::UpdateTime();

    const csmFloat32 deltaTimeSeconds = LAppPal::GetDeltaTime();
    _userTimeSeconds += deltaTimeSeconds;

    _dragManager->Update(deltaTimeSeconds);
    _dragX = _dragManager->GetX();
    _dragY = _dragManager->GetY();

    // モーションによるパラメータ更新の有無
    csmBool motionUpdated = false;

    //-----------------------------------------------------------------
    _model->LoadParameters(); // 前回セーブされた状態を
    if (!_motionManager->IsFinished())
    {
        motionUpdated = _motionManager->UpdateMotion(_model, deltaTimeSeconds); // モーションを更新
    }
    _model->SaveParameters(); // 状態を保存
    //-----------------------------------------------------------------

    // 不透明度
    _opacity = _model->GetModelOpacity();

    // まばたき
    if (!motionUpdated)
    {
        if (_autoBlink && _eyeBlink != NULL)
        {
            // メインモーションの更新がないとき
            _eyeBlink->UpdateParameters(_model, deltaTimeSeconds); // 目パチ
        }
    }

    if (_expressionManager != NULL)
    {
        _expressionManager->UpdateMotion(_model, deltaTimeSeconds); // 表情でパラメータ更新（相対変化）
    }

    // ドラッグによる変化
    // ドラッグによる顔の向きの調整
    _model->AddParameterValue(_idParamAngleX, _dragX * 30); // -30から30の値を加える
    _model->AddParameterValue(_idParamAngleY, _dragY * 30);
    _model->AddParameterValue(_idParamAngleZ, _dragX * _dragY * -30);

    // ドラッグによる体の向きの調整
    _model->AddParameterValue(_idParamBodyAngleX, _dragX * 10); // -10から10の値を加える

    // ドラッグによる目の向きの調整
    _model->AddParameterValue(_idParamEyeBallX, _dragX); // -1から1の値を加える
    _model->AddParameterValue(_idParamEyeBallY, _dragY);

    // 呼吸など
    if (_autoBreath && _breath != NULL)
    {
        _breath->UpdateParameters(_model, deltaTimeSeconds);
    }

    // 物理演算の設定
    if (_physics != NULL)
    {
        _physics->Evaluate(_model, deltaTimeSeconds);
    }

    // ポーズの設定
    if (_pose != NULL)
    {
        _pose->UpdateParameters(_model, deltaTimeSeconds);
    }
}

CubismMotionQueueEntryHandle LAppModel::StartMotion(const csmChar* group, csmInt32 no, csmInt32 priority,
                                                    ACubismMotion::BeganMotionCallback onStartMotionHandler,
                                                    ACubismMotion::FinishedMotionCallback onFinishedMotionHandler)
{
    if (priority == PriorityForce)
    {
        _motionManager->SetReservePriority(priority);
    }
    else if (!_motionManager->ReserveMotion(priority))
    {
        if (_debugMode)
        {
            Info("can't start motion.");
        }
        return InvalidMotionQueueEntryHandleValue;
    }

    const csmString fileName = _modelSetting->GetMotionFileName(group, no);

    // ex) idle_0
    csmString name = Utils::CubismString::GetFormatedString("%s_%d", group, no);
    CubismMotion* motion = static_cast<CubismMotion*>(_motions[name.GetRawString()]);
    csmBool autoDelete = false;

    csmBool hasMotion = true;

    if (fileName.GetLength() <= 0)
    {
        hasMotion = false;
        Info("motion(%s) has no file attached", name.GetRawString());
        goto handler_label;
    }

    if (motion == NULL)
    {
        csmString path = fileName;

        path = _modelHomeDir + path;

        csmByte* buffer;
        csmSizeInt size;
        buffer = CreateBuffer(path.GetRawString(), &size);

        motion = static_cast<CubismMotion*>(LoadMotion(buffer, size, NULL));

        if (motion)
        {
            csmFloat32 fadeTime = _modelSetting->GetMotionFadeInTimeValue(group, no);
            if (fadeTime >= 0.0f)
            {
                motion->SetFadeInTime(fadeTime);
            }

            fadeTime = _modelSetting->GetMotionFadeOutTimeValue(group, no);
            if (fadeTime >= 0.0f)
            {
                motion->SetFadeOutTime(fadeTime);
            }
            motion->SetEffectIds(_eyeBlinkIds, _lipSyncIds);
            autoDelete = true; // 終了時にメモリから削除
        }

        DeleteBuffer(buffer, path.GetRawString());
    }

    if (motion)
    {
        motion->group = group;
        motion->no = no;
        motion->SetBeganMotionHandler(onStartMotionHandler);
        motion->SetFinishedMotionHandler(onFinishedMotionHandler);
    }

handler_label:

    if (!hasMotion)
    {
        // 添加空指针判断，如果 motion 文件不存在，直接调用动作结束回调函数
        // 修复模型文件不存在时，导致崩溃
        FakeMotion fakeMotion;
        fakeMotion.group = group;
        fakeMotion.no = no;
        onStartMotionHandler(&fakeMotion);
        onFinishedMotionHandler(&fakeMotion);

        _motionManager->SetReservePriority(PriorityNone);
        return InvalidMotionQueueEntryHandleValue;
    }

    return _motionManager->StartMotionPriority(motion, autoDelete, priority);
}

CubismMotionQueueEntryHandle LAppModel::StartRandomMotion(const csmChar* group, csmInt32 priority,
                                                          ACubismMotion::BeganMotionCallback onStartMotionHandler,
                                                          ACubismMotion::FinishedMotionCallback onFinishedMotionHandler)
{
    if (_modelSetting->GetMotionCount(group) == 0)
    {
        return InvalidMotionQueueEntryHandleValue;
    }

    csmInt32 no = rand() % _modelSetting->GetMotionCount(group);

    return StartMotion(group, no, priority, onStartMotionHandler, onFinishedMotionHandler);
}

void LAppModel::DoDraw()
{
    if (_model == NULL)
    {
        return;
    }

    GetRenderer<Rendering::CubismRenderer_OpenGLES2>()->DrawModel();
}

void LAppModel::Draw()
{
    if (_model == NULL)
    {
        return;
    }

    _model->Update();

    CubismMatrix44& matrix = _matrixManager.GetProjection(this);
    matrix.MultiplyByMatrix(_modelMatrix);

    GetRenderer<Rendering::CubismRenderer_OpenGLES2>()->SetMvpMatrix(&matrix);

    DoDraw();
}

csmBool LAppModel::HitTest(const csmChar* hitAreaName, csmFloat32 x, csmFloat32 y)
{
    // 透明時は当たり判定なし。
    if (_opacity < 1)
    {
        return false;
    }
    const csmInt32 count = _modelSetting->GetHitAreasCount();
    for (csmInt32 i = 0; i < count; i++)
    {
        if (strcmp(_modelSetting->GetHitAreaName(i), hitAreaName) == 0)
        {
            const CubismIdHandle drawID = _modelSetting->GetHitAreaId(i);
            return IsHit(drawID, x, y);
        }
    }
    return false; // 存在しない場合はfalse
}

csmString LAppModel::HitTest(float x, float y)
{
    this->_matrixManager.ScreenToScene(&x, &y);
    // 透明時は当たり判定なし。
    if (_opacity < 1)
    {
        return "";
    }
    const csmInt32 count = _modelSetting->GetHitAreasCount();
    for (csmInt32 i = 0; i < count; i++)
    {
        const CubismIdHandle drawID = _modelSetting->GetHitAreaId(i);
        if (IsHit(drawID, x, y))
        {
            return _modelSetting->GetHitAreaName(i);
        }
    }
    return "";
}

void LAppModel::Resize(int ww, int wh)
{
    _matrixManager.UpdateScreenToScene(ww, wh);
}

void LAppModel::Touch(float x, float y, ACubismMotion::BeganMotionCallback s_call,
                      ACubismMotion::FinishedMotionCallback f_call)
{
    csmString hitArea = HitTest(x, y);
    if (strlen(hitArea.GetRawString()) != 0)
    {
        Info("hit area: [%s]", hitArea.GetRawString());
        if (strcmp(hitArea.GetRawString(), HitAreaHead) == 0)
        {
            SetRandomExpression();
        }
        StartRandomMotion(hitArea.GetRawString(), MOTION_PRIORITY_FORCE,
                          s_call, f_call);
    }
}

void LAppModel::SetExpression(const csmChar* expressionID)
{
    ACubismMotion* motion = _expressions[expressionID];
    if (_debugMode)
    {
        Info("expression: [%s]", expressionID);
    }

    if (motion != NULL)
    {
        _expressionManager->StartMotionPriority(motion, false, PriorityForce);
    }
    else
    {
        if (_debugMode)
            Info("expression[%s] is null ", expressionID);
    }
}

void LAppModel::SetRandomExpression()
{
    if (_expressions.GetSize() == 0)
    {
        return;
    }

    csmInt32 no = rand() % _expressions.GetSize();
    csmMap<csmString, ACubismMotion*>::const_iterator map_ite;
    csmInt32 i = 0;
    for (map_ite = _expressions.Begin(); map_ite != _expressions.End(); map_ite++)
    {
        if (i == no)
        {
            csmString name = (*map_ite).First;
            SetExpression(name.GetRawString());
            return;
        }
        i++;
    }
}

void LAppModel::ReloadRenderer()
{
    DeleteRenderer();

    CreateRenderer();

    SetupTextures();
}

void LAppModel::SetupTextures()
{
    for (csmInt32 modelTextureNumber = 0; modelTextureNumber < _modelSetting->GetTextureCount(); modelTextureNumber++)
    {
        // テクスチャ名が空文字だった場合はロード・バインド処理をスキップ
        if (strcmp(_modelSetting->GetTextureFileName(modelTextureNumber), "") == 0)
        {
            continue;
        }

        // OpenGLのテクスチャユニットにテクスチャをロードする
        csmString texturePath = _modelSetting->GetTextureFileName(modelTextureNumber);
        texturePath = _modelHomeDir + texturePath;

        LAppTextureManager::TextureInfo* texture = _textureManager.CreateTextureFromPngFile(texturePath.GetRawString());
        const csmInt32 glTextueNumber = texture->id;

        // OpenGL
        GetRenderer<Rendering::CubismRenderer_OpenGLES2>()->BindTexture(modelTextureNumber, glTextueNumber);
    }

#ifdef PREMULTIPLIED_ALPHA_ENABLE
    GetRenderer<Rendering::CubismRenderer_OpenGLES2>()->IsPremultipliedAlpha(true);
#else
    GetRenderer<Rendering::CubismRenderer_OpenGLES2>()->IsPremultipliedAlpha(false);
#endif
}

void LAppModel::MotionEventFired(const csmString& eventValue)
{
    CubismLogInfo("%s is fired on LAppModel!!", eventValue.GetRawString());
}

Csm::Rendering::CubismOffscreenSurface_OpenGLES2& LAppModel::GetRenderBuffer()
{
    return _renderBuffer;
}

csmBool LAppModel::HasMocConsistencyFromFile(const csmChar* mocFileName)
{
    CSM_ASSERT(strcmp(mocFileName, ""));

    csmByte* buffer;
    csmSizeInt size;

    csmString path = mocFileName;
    path = _modelHomeDir + path;

    buffer = CreateBuffer(path.GetRawString(), &size);

    csmBool consistency = CubismMoc::HasMocConsistencyFromUnrevivedMoc(buffer, size);
    if (!consistency)
    {
        Error("Inconsistent MOC3.");
    }
    else
    {
        Info("Consistent MOC3.");
    }

    DeleteBuffer(buffer);

    return consistency;
}

bool LAppModel::IsMotionFinished()
{
    return _motionManager->IsFinished();
}

void LAppModel::SetParameterValue(const char* paramId, float value, float weight)
{
    const Csm::CubismId* paramHanle = CubismFramework::GetIdManager()->GetId(paramId);
    _model->SetParameterValue(paramHanle, value, weight);
}

void LAppModel::AddParameterValue(const char* paramId, float value)
{
    const Csm::CubismId* paramHanle = CubismFramework::GetIdManager()->GetId(paramId);
    _model->AddParameterValue(paramHanle, value);
}

void LAppModel::SetAutoBreathEnable(bool enable)
{
    _autoBreath = enable;
}

void LAppModel::SetAutoBlinkEnable(bool enable)
{
    _autoBlink = enable;
}

int LAppModel::GetParameterCount()
{
    return _model->GetParameterCount();
}

void LAppModel::GetParameter(int i, const char*& id, int& type, float& value, float& maxValue, float& minValue,
                             float& defaultValue)
{
    id = _model->GetParameterId(i)->GetString().GetRawString();
    type = _model->GetParameterType(i);
    value = _model->GetParameterValue(i);
    maxValue = _model->GetParameterMaximumValue(i);
    minValue = _model->GetParameterMinimumValue(i);
    defaultValue = _model->GetParameterDefaultValue(i);
}

int LAppModel::GetPartCount()
{
    return _model->GetPartCount();
}

Csm::csmString LAppModel::GetPartId(int idx)
{
    return _model->GetPartId(idx)->GetString();
}

void LAppModel::SetPartOpacity(int idx, float opacity)
{
    _model->SetPartOpacity(idx, opacity);
}

using namespace Live2D::Cubism::Core;

static bool isInTriangle(const csmVector2 p0, const csmVector2 p1, const csmVector2 p2, const csmVector2 p)
{
    // https://github.com/Arkueid/live2d-py/issues/18
    // 情况1：
    //  要检测的三角形很多，说明模型很精细，那么一定程度上三角形的面积会很小，
    //  只有少量的三角形的范围检测会失败，增加的额外计算量不会太大，
    //  因此只需要简单判断范围即可回避大量浮点计算
    // 情况2：
    //  要检测的三角形比较少，说明模型很粗糙，那么总的计算量就会相对较少，
    //  增加几次范围检测理论上是可以接受的
    // 总结为：需要计算叉积的实际三角形其实不会很多，因此范围检测可以避免大部分计算

    // 范围检测
    if (p.X < std::min({p0.X, p1.X, p2.X}))
    {
        return false;
    }
    if (p.X > std::max({p0.X, p1.X, p2.X}))
    {
        return false;
    }
    if (p.Y < std::min({p0.Y, p1.Y, p2.Y}))
    {
        return false;
    }
    if (p.Y > std::max({p0.Y, p1.Y, p2.Y}))
    {
        return false;
    }

    // 叉积检测
    const float dX = p.X - p2.X;
    const float dY = p.Y - p2.Y;
    const float dX21 = p2.X - p1.X;
    const float dY12 = p1.Y - p2.Y;
    const float D = dY12 * (p0.X - p2.X) + dX21 * (p0.Y - p2.Y);
    const float s = dY12 * dX + dX21 * dY;
    const float t = (p2.Y - p0.Y) * dX + (p0.X - p2.X) * dY;
    if (D < 0) return s <= 0 && t <= 0 && s + t >= D;
    return s >= 0 && t >= 0 && s + t <= D;
}

void LAppModel::HitPart(float x, float y, bool topOnly, std::vector<std::string>& partIds)
{
    _matrixManager.ScreenToScene(&x, &y);
    x = _modelMatrix->InvertTransformX(x);
    y = _modelMatrix->InvertTransformY(y);
    const csmInt32 drawableCount = _model->GetDrawableCount();
    const csmInt32* renderOrders = _model->GetDrawableRenderOrders();
    int* drawableIndices = new int[drawableCount];
    for (csmInt32 i = 0; i < drawableCount; i++)
    {
        // 绘制顺序，先绘制的被后绘制的覆盖
        drawableIndices[drawableCount - 1 - renderOrders[i]] = i;
    }
    // 多个 part index 可能指向同一个 part id，所以用 part id set
    std::unordered_set<const char*> hitParts;
    bool topClicked = false;

    for (int i = 0; i < drawableCount; i++)
    {
        int drawableIndex = drawableIndices[i];
        if (_model->GetDrawableOpacity(drawableIndex) == 0.0f)
        {
            continue;
        }
        int partIndex = _model->GetDrawableParentPartIndex(drawableIndex);
        if (partIndex == -1)
        {
            // 绘制对象不属于 part
            continue;
        }
        const char* partId = _model->GetPartId(partIndex)->GetString().GetRawString();
        if (_model->GetPartOpacity(partIndex) == 0.0f)
        {
            continue;
        }
        // 已经点击过的部件
        if (hitParts.find(partId) != hitParts.end())
        {
            continue;
        }
        // 顶点连线个数，3个顶点一个三角形，一定是3的整数倍
        const int indexCount = _model->GetDrawableVertexIndexCount(drawableIndex);
        // 顶点坐标
        const csmVector2* vertices = _model->GetDrawableVertexPositions(drawableIndex);
        // 三角形顶点索引
        const csmUint16* indices = _model->GetDrawableVertexIndices(drawableIndex);
        const int triangleCount = indexCount / 3;

        for (int j = 0; j < triangleCount; j++)
        {
            if (!isInTriangle(vertices[indices[j * 3]], vertices[indices[j * 3 + 1]], vertices[indices[j * 3 + 2]],
                              {x, y}))
            {
                continue;
            }
            partIds.emplace_back(partId);
            hitParts.insert(partIds.back().c_str());
            topClicked = true;
            break;
        }

        if (topOnly && topClicked)
        {
            break;
        }
    }
    delete[] drawableIndices;
}

void LAppModel::setPartMultiplyColor(int partNo, float r, float g, float b, float a)
{
    _model->SetPartMultiplyColor(partNo, r, g, b, a);
    if (_model->GetOverwriteColorForPartMultiplyColors(partNo))
    {
        return;
    }
    _model->SetOverwriteColorForPartMultiplyColors(partNo, true);
}

void LAppModel::getPartMultiplyColor(int partNo, float& r, float& g, float& b, float& a)
{
    auto color = _model->GetPartMultiplyColor(partNo);
    r = color.R;
    g = color.G;
    b = color.B;
    a = color.A;
}

void LAppModel::setPartScreenColor(int partNo, float r, float g, float b, float a)
{
    _model->SetPartScreenColor(partNo, r, g, b, a);
    if (_model->GetOverwriteColorForPartScreenColors(partNo))
    {
        return;
    }
    _model->SetOverwriteColorForPartScreenColors(partNo, true);
}

void LAppModel::getPartScreenColor(int partNo, float& r, float& g, float& b, float& a)
{
    auto color = _model->GetPartScreenColor(partNo);
    r = color.R;
    g = color.G;
    b = color.B;
    a = color.A;
}

void LAppModel::Drag(float x, float y)
{
    _matrixManager.ScreenToScene(&x, &y);
    SetDragging(x, y);
}

void LAppModel::SetOffset(float dx, float dy)
{
    _matrixManager.SetOffset(dx, dy);
}

void LAppModel::SetScale(float scale)
{
    _matrixManager.SetScale(scale);
}
