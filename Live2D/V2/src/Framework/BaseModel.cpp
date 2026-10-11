#include "BaseModel.hpp"
#include "BinaryReader.hpp"
#include "ModelImpl.hpp"
#include "MeshContext.hpp"
#include "ModelContext.hpp"
#include "AMotion.hpp"
#include "Live2DMotion.hpp"
#include "GLRenderer.hpp"
#include "ExpressionMotion.hpp"
#include "EyeBlink.hpp"
#include "MotionManager.hpp"
#include "Physics.hpp"
#include "Pose.hpp"

namespace Live2D {
namespace V2 {

BaseModel::BaseModel()
    : mModelImpl(nullptr)
    , mModelContext(nullptr)
    , mEyeBlink(std::make_unique<EyeBlink>())
    , mPhysics(std::make_unique<Physics>())
    , mPose(std::make_unique<Pose>())
    , mMainMotionMgr(std::make_unique<MotionManager>())
    , mExpressionMgr(std::make_unique<MotionManager>()) {}

BaseModel::~BaseModel() = default;

void BaseModel::loadModelData(const std::vector<uint8_t>& data, int version) {
    BinaryReader br(data);
    br.readByte();
    br.readByte();
    br.readByte();
    version = br.readByte();
    br.setFormatVersion(version);
    auto* impl = static_cast<ModelImpl*>(br.readObjectRaw());
    mModelImpl.reset(impl);
    mModelContext = std::make_unique<ModelContext>();
    mModelContext->init(mModelImpl.get());
    mModelMatrix =
        ModelMatrix((float)mModelImpl->getCanvasWidth(), (float)mModelImpl->getCanvasHeight());
    mModelMatrix.setWidth(2);
    mModelMatrix.setCenterPosition(0, 0);
}
AMotion* BaseModel::loadMotion(const std::string& name, const std::vector<uint8_t>& data) {
    auto* m = Live2DMotion::load(data);
    mMotions[name].emplace_back(m);
    return m;
}
AMotion* BaseModel::loadExpression(const std::string& name, const std::vector<uint8_t>& data) {
    auto* m = ExpressionMotion::load(data);
    mExpressions[name].reset(m);
    return m;
}
Pose* BaseModel::loadPose(const std::vector<uint8_t>& data) {
    mPose.reset(Pose::load(data));
    return mPose.get();
}
void BaseModel::loadPhysics(const std::vector<uint8_t>& data) {
    mPhysics.reset(Physics::load(data));
}
bool BaseModel::hitTestSimple(const std::string& drawID, float x, float y) {
    auto* mc = mModelContext.get();
    int drawIdx = mc->getDrawDataIndex(&Id::getID(drawID));
    if (drawIdx < 0)
        return false;

    auto* dctx = mc->getDrawContext(drawIdx);
    if (!dctx || !dctx->mAvailable)
        return false;

    auto& verts =
        !dctx->mTransformedPoints.empty() ? dctx->mTransformedPoints : dctx->mInterpolatedPoints;
    if (verts.empty())
        return false;

    float left = (float)mModelImpl->getCanvasWidth(), right = 0;
    float top = (float)mModelImpl->getCanvasHeight(), bottom = 0;
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

    float tx = mModelMatrix.invertTransformX(x);
    float ty = mModelMatrix.invertTransformY(y);
    return left <= tx && tx <= right && top <= ty && ty <= bottom;
}

}   // namespace V2
}   // namespace Live2D