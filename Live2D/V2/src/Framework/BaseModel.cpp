#include "BaseModel.hpp"
#include "AMotion.hpp"
#include "BinaryReader.hpp"
#include "Drawable.hpp"
#include "ExpressionMotion.hpp"
#include "EyeBlink.hpp"
#include "GLRenderer.hpp"
#include "Id.hpp"
#include "Motion.hpp"
#include "Moc.hpp"
#include "MotionManager.hpp"
#include "Physics.hpp"
#include "Pose.hpp"
#include <memory>

namespace Live2D {
namespace V2 {

BaseModel::BaseModel() :
    mIdPool(), mMoc(nullptr), mEyeBlink(nullptr), mPhysics(std::make_unique<Physics>()),
    mPose(std::make_unique<Pose>()), mMainMotionMgr(std::make_unique<MotionManager>()),
    mExpressionMgr(std::make_unique<MotionManager>()) {

    mEyeBlink = std::make_unique<EyeBlink>(mIdPool);
}

BaseModel::~BaseModel() = default;

void BaseModel::LoadModelData(const std::vector<uint8_t>& data, int version) {
    BinaryReader br(data, mIdPool);
    br.ReadByte();
    br.ReadByte();
    br.ReadByte();
    version = br.ReadByte();
    br.SetFormatVersion(version);
    mMoc.reset(static_cast<Moc*>(br.ReadObjectRaw()));
    mMoc->Init();
    mModelMatrix = ModelMatrix((float)mMoc->GetCanvasWidth(), (float)mMoc->GetCanvasHeight());
    // 默认按画布高度适配（对齐 Common::MatrixManager 横屏规则），
    // 保证高画布的角色模型默认完整显示；
    // 渲染适配矩阵由 Common::MatrixManager 每帧按窗口重算，此处仅供 hitTestSimple
    // 反算
    mModelMatrix.SetHeight(2);
    mModelMatrix.SetCenterPosition(0, 0);
}
AMotion* BaseModel::LoadMotion(const std::string& name, const std::vector<uint8_t>& data) {
    auto* m = Motion::Load(data, mIdPool);
    mMotions[name].emplace_back(m);
    return m;
}
AMotion* BaseModel::LoadExpression(const std::string& name, const std::vector<uint8_t>& data) {
    auto* m = ExpressionMotion::Load(data, mIdPool);
    mExpressions[name].reset(m);
    return m;
}
Pose* BaseModel::LoadPose(const std::vector<uint8_t>& data) {
    mPose.reset(Pose::Load(data, mIdPool));
    return mPose.get();
}
void BaseModel::LoadPhysics(const std::vector<uint8_t>& data) {
    mPhysics.reset(Physics::Load(data, mIdPool));
}
bool BaseModel::HitTestSimple(const std::string& drawID, float x, float y) {
    auto* moc = mMoc.get();
    int drawIdx = moc->GetDrawableIndex(drawID);
    if (drawIdx < 0)
        return false;

    auto* dd = moc->GetDrawable(drawIdx);
    if (!dd || !dd->mAvailable)
        return false;

    auto& verts =
        !dd->mTransformedPoints.empty() ? dd->mTransformedPoints : dd->mInterpolatedPoints;
    if (verts.empty())
        return false;

    float left = (float)mMoc->GetCanvasWidth(), right = 0;
    float top = (float)mMoc->GetCanvasHeight(), bottom = 0;
    for (size_t j = 0; j < verts.size(); j += 2) {
        float vx = verts[j], vy = verts[j + 1];
        if (vx < left)
            left = vx;
        if (vx > right)
            right = vx;
        if (vy < top)
            top = vy;
        if (vy > bottom)
            bottom = vy;
    }

    float tx = mModelMatrix.InvertTransformX(x);
    float ty = mModelMatrix.InvertTransformY(y);
    return left <= tx && tx <= right && top <= ty && ty <= bottom;
}

}   // namespace V2
}   // namespace Live2D