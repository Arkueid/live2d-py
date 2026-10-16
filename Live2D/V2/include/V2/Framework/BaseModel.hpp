#pragma once
#include "Moc.hpp"
#include "ModelMatrix.hpp"
#include <IModel.hpp>
#include <memory>
#include <string>
#include <unordered_map>
#include "Id.hpp"


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
    void LoadModelData(const std::vector<uint8_t>& data, int version);
    AMotion* LoadMotion(const std::string& name, const std::vector<uint8_t>& data);
    AMotion* LoadExpression(const std::string& name, const std::vector<uint8_t>& data);
    Pose* LoadPose(const std::vector<uint8_t>& data);
    void LoadPhysics(const std::vector<uint8_t>& data);
    bool HitTestSimple(const std::string& drawID, float x, float y);
    ModelMatrix& GetModelMatrix() { return mModelMatrix; }
    MotionManager* GetMainMotionManager() const { return mMainMotionMgr.get(); }
    MotionManager* GetExpressionManager() const { return mExpressionMgr.get(); }
    void SetAlpha(float a) { mAlpha = (a < 0 ? 0 : (a > 1 ? 1 : a)); }
    float GetAlpha() const { return mAlpha; }
    void SetAccel(float x, float y, float z) {
        mAccelX = x;
        mAccelY = y;
        mAccelZ = z;
    }
    void SetDrag(float x, float y) {
        mDragX = x;
        mDragY = y;
    }
    bool IsInitialized() const { return mInitialized; }
    void SetInitialized(bool v) { mInitialized = v; }
    bool IsUpdating() const { return mUpdating; }
    void SetUpdating(bool v) { mUpdating = v; }

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
    std::unique_ptr<Moc> mMoc;
    Id::IdPool mIdPool;
};
}   // namespace V2
}   // namespace Live2D