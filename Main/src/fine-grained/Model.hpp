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

using namespace Csm;

class Model : public Csm::CubismUserModel
{
public:
    Model();
    ~Model() override;

    /**
     * @brief
     * @param path model3.json path
     */
    void LoadModelJson(const char *path);

    /**
     * @brief load motion, expression and pose data
     * @note must be called after LoadModelJson
     */
    void SetupModel();

    // update

    /**
     * @brief
     * @param deltaTime time elapsed since last frame
     * @return true if motion is not finished and motion is updated
     */
    bool UpdateMotion(float deltaTime);

    void UpdateDrag(float deltaTime);

    void UpdateBreath(float deltaTime);

    void UpdateBlink(float deltaTime);

    void UpdateExpression(float deltaTime);

    void UpdatePhysics(float deltaTime);

    // param
    void GetParameterIds(std::vector<std::string> &ids);

    float GetParameterValue(const char *id);

    float GetParameterMaximumValue(const char *id);

    float GetParameterMinimumValue(const char *id);

    float GetParameterDefaultValue(const char *id);

    void SetParameterValue(const char *id, float value, float weight = 1.0f);

    void AddParameterValue(const char *id, float value);

    // transform
    void SetOffsetX(float x);

    void SetOffsetY(float y);

    void Rotate(float angle);

    void SetScale(float scale);

    // motion
    // TODO
    void StartMotion(const char *group, int no, int priority,
                     void *startCallee = nullptr, ACubismMotion::BeganMotionCallback startCalleeHandler = nullptr,
                     void *finishCallee = nullptr, ACubismMotion::FinishedMotionCallback finishCalleeHandler = nullptr);

    void StartRandomMotion(const char *group, int priority,
                           void *startCallee = nullptr, ACubismMotion::BeganMotionCallback startCalleeHandler = nullptr,
                           void *finishCallee = nullptr, ACubismMotion::FinishedMotionCallback finishCalleeHandler = nullptr);

    bool IsMotionFinished();

    void LoadExtraMotion(const char* group, const char* motionJsonPath);

    // touch
    void HitPart(float x, float y, std::vector<const char *> partIds, bool topOnly = false);

    bool IsAreaHit(const char *areaName, float x, float y);

    void Drag(float x, float y);

    // render

    /**
     * @brief draw model
     */
    void SetupRenderer();

    void ReleaseRenderer();
    
    void Draw();

    // part
    void GetPartIds(std::vector<std::string> &ids);

    void SetPartOpacity(const char *partId, float opacity);

    void SetPartScreenColor(const char *partId, float r, float g, float b, float a);

    void SetPartMultiplyColor(const char *partId, float r, float g, float b, float a);

    // expression
    void SetExpression(const char *expressionId);

    void GetExpressionIds(std::vector<std::string> &ids);
    
    /**
     * @brief
     * @return expression id being set
     */
    const char *SetRandomExpression();

    void ResetExpression();

    void SetDefaultExpression(const char *expressionId);

    void SetFadeOutExpression(const char *expressionId, float fadeOutTime);

    // reset
    void StopAllMotions();

    void ResetAllParameters();

    void ResetPose();

private:
};