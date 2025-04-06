/**
 * @brief Model.hpp
 * @author Arkueid
 * @date 2025/04/06
 * @note 更细粒度、更独立的 live2d 模型管理类
 */

#pragma once

#include <vector>
#include <string>

#include <Model/CubismUserModel.hpp>
#include <Motion/ACubismMotion.hpp>

#include <LAppTextureManager.hpp>
#include <MatrixManager.hpp>

using namespace Csm;

class Model : public Csm::CubismUserModel
{
public:
    Model();
    ~Model() override;

    /**
     * @brief
     * @param filePath model3.json path
     */
    void LoadModelJson(const char *filePath);

    // update

    /**
     * @brief
     * @param deltaSecs time elapsed since last frame
     * @return true if motion is not finished and motion is updated
     */
    bool UpdateMotion(float deltaSecs);

    void UpdateDrag(float deltaSecs);

    void UpdateBreath(float deltaSecs);

    void UpdateBlink(float deltaSecs);

    void UpdateExpression(float deltaSecs);

    void UpdatePhysics(float deltaSecs);

    void UpdatePose(float deltaSecs);

    // param
    void GetParameterIds(std::vector<std::string> &ids);

    float GetParameterValue(int index);

    float GetParameterMaximumValue(int index);

    float GetParameterMinimumValue(int index);

    float GetParameterDefaultValue(int index);

    void SetParameterValue(const char *id, float value, float weight = 1.0f);

    void AddParameterValue(const char *id, float value);

    void LoadParameters();

    void SaveParameters();

    // transform
    void Resize(int width, int height);

    void SetOffset(float x, float y);

    void Rotate(float angle);

    void SetScale(float scale);

    // motion
    // TODO
    void StartMotion(const char *group, int no, int priority = 3,
                     void *startCallee = nullptr, ACubismMotion::BeganMotionCallback startCalleeHandler = nullptr,
                     void *finishCallee = nullptr, ACubismMotion::FinishedMotionCallback finishCalleeHandler = nullptr);

    void StartRandomMotion(const char *group = nullptr, int priority = 3,
                           void *startCallee = nullptr, ACubismMotion::BeganMotionCallback startCalleeHandler = nullptr,
                           void *finishCallee = nullptr, ACubismMotion::FinishedMotionCallback finishCalleeHandler = nullptr);

    bool IsMotionFinished();

    void LoadExtraMotion(const char* group, int no, const char* motionJsonPath);

    // touch
    void HitPart(float x, float y, std::vector<const char *>& partIds, bool topOnly = false);

    bool IsAreaHit(const char *areaName, float x, float y);

    void Drag(float x, float y);

    // render

    /**
     * @brief draw model
     */
    void CreateRenderer(int maskBufferCount = 1);
    void ReloadRenderer(int maskBufferCount = 1);

    void Draw();

    // part
    void GetPartIds(std::vector<std::string> &ids);

    void SetPartOpacity(int index, float opacity);

    void SetPartScreenColor(int index, float r, float g, float b, float a);

    void SetPartMultiplyColor(int index, float r, float g, float b, float a);

    // expression
    void SetExpression(const char *expressionId);

    void GetExpressionIds(std::vector<std::string> &ids);
    
    std::string SetRandomExpression();

    void ResetExpression();

    void SetDefaultExpression(const char *expressionId);

    void SetFadeOutExpression(const char *expressionId, double fadeOutTime);

    // reset
    void StopAllMotions();

    void ResetAllParameters();

    void ResetPose();

private:
    void ReleaseMotions();
    void ReleaseExpressions();
    void SetupTextures();
    void PreloadMotionGroup(const csmChar* group);
    void SetupModel();
private:
    ICubismModelSetting* _modelSetting;
    csmVector<CubismIdHandle> _eyeBlinkIds;
    csmVector<CubismIdHandle> _lipSyncIds;

    csmString _modelHomeDir;
    csmMap<Csm::csmString, ACubismMotion*> _motions;
    csmMap<Csm::csmString, ACubismMotion*> _expressions;

    const Csm::CubismId* _idParamAngleX;
    const Csm::CubismId* _idParamAngleY;
    const Csm::CubismId* _idParamAngleZ;
    const Csm::CubismId* _idParamBodyAngleX;
    const Csm::CubismId* _idParamEyeBallX;
    const Csm::CubismId* _idParamEyeBallY;

    int _ParamAngleXi;
    int _ParamAngleYi;
    int _ParamAngleZi;
    int _ParamBodyAngleXi;
    int _ParamEyeBallXi;
    int _ParamEyeBallYi;

    LAppTextureManager _textureManager;

    MatrixManager _matrixManager;

    csmFloat32 _dragX;
    csmFloat32 _dragY;

    int* _tmpOrderedDrawIndice;
    const float* _parameterDefaultValues;
    float* _parameterValues;
    int _parameterCount;

    std::vector<csmString> _motionGroupNames;
    std::vector<int> _motionCounts;

    std::string _defaultExpressionId;
    double _expFadeOutTimeMillis;
};