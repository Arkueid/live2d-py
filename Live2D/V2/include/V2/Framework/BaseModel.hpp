#pragma once
#include "ModelImpl.hpp"
#include "ModelMatrix.hpp"
#include "ModelContext.hpp"
#include <IModel.hpp>
#include <memory>
#include <string>
#include <unordered_map>
namespace Live2D {
namespace V2 {
class Live2DModelOpenGL;
class MotionManager;
class EyeBlink;
class Pose;
class Physics;
class AMotion;
// 统一模型接口: Model -> BaseModel -> IModel 单继承链
class BaseModel : public IModel {
public:
    BaseModel();
    ~BaseModel() override;
    void loadModelData(const std::vector<uint8_t>& data, int version);
    AMotion* loadMotion(const std::string& name, const std::vector<uint8_t>& data);
    AMotion* loadExpression(const std::string& name, const std::vector<uint8_t>& data);
    Pose* loadPose(const std::vector<uint8_t>& data);
    void loadPhysics(const std::vector<uint8_t>& data);
    bool hitTestSimple(const std::string& drawID, float x, float y);
    ModelMatrix& getModelMatrix() { return mModelMatrix; }
    MotionManager* getMainMotionManager() const { return mMainMotionMgr.get(); }
    MotionManager* getExpressionManager() const { return mExpressionMgr.get(); }
    void setAlpha(float a) { mAlpha = (a < 0 ? 0 : (a > 1 ? 1 : a)); }
    float getAlpha() const { return mAlpha; }
    void setAccel(float x, float y, float z) {
        mAccelX = x;
        mAccelY = y;
        mAccelZ = z;
    }
    void setDrag(float x, float y) {
        mDragX = x;
        mDragY = y;
    }
    bool isInitialized() const { return mInitialized; }
    void setInitialized(bool v) { mInitialized = v; }
    bool isUpdating() const { return mUpdating; }
    void setUpdating(bool v) { mUpdating = v; }

protected:
    ModelMatrix mModelMatrix;
    std::unique_ptr<EyeBlink> mEyeBlink;
    std::unique_ptr<Physics> mPhysics;
    std::unique_ptr<Pose> mPose;
    std::unique_ptr<MotionManager> mMainMotionMgr;
    std::unique_ptr<MotionManager> mExpressionMgr;
    std::unordered_map<std::string, std::vector<std::unique_ptr<AMotion>>> mMotions;
    std::unordered_map<std::string, std::unique_ptr<AMotion>> mExpressions;
    float mAlpha = 1.0f, mAccelX = 0, mAccelY = 0, mAccelZ = 0, mDragX = 0, mDragY = 0;
    bool mInitialized = false, mUpdating = false;
    std::unique_ptr<ModelImpl> mModelImpl;
    std::unique_ptr<ModelContext> mModelContext;
};
}   // namespace V2
}   // namespace Live2D