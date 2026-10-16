#pragma once
#include <string>
#include <vector>
namespace Live2D {
namespace V2 {
class Moc;

enum PhysicsSrcType { SRC_TO_X = 0, SRC_TO_Y, SRC_TO_G_ANGLE };
enum PhysicsTargetType { TARGET_FROM_ANGLE = 0, TARGET_FROM_ANGLE_V };

struct PhysicsPoint {
    float mass = 1, x = 0, y = 0, vx = 0, vy = 0, ax = 0, ay = 0, fx = 0, fy = 0;
    float lastX = 0, lastY = 0, lastVX = 0, lastVY = 0;
    void SetupLast() {
        lastX = x;
        lastY = y;
        lastVX = vx;
        lastVY = vy;
    }
};

struct PhysicsSrc {
    PhysicsSrcType type;
    std::string paramId;
    float scale = 1, weight = 1;
    void Update(Moc* moc, class PhysicsHair* ctx);
};
struct PhysicsTarget {
    PhysicsTargetType type;
    std::string paramId;
    float scale = 1, weight = 1;
    void Update(Moc* moc, class PhysicsHair* ctx);
};

class PhysicsHair {
public:
    PhysicsHair();
    void Setup(float length, float stiffness, float mass);
    void AddSrcParam(PhysicsSrcType t, const std::string& id, float scale, float weight);
    void AddTargetParam(PhysicsTargetType t, const std::string& id, float scale, float weight);
    void Update(Moc* moc, long long timeMs);

    PhysicsPoint& GetP1() { return p1; }
    PhysicsPoint& GetP2() { return p2; }
    float GetAngle() const { return angle; }
    void SetAngle(float a) { angle = a; }
    float GetLastAngle() const { return lastAngle; }
    float GetLastAngleVelocity() const { return angleVelocity; }

private:
    float CalcAngle() const;
    void UpdatePhysics(Moc* moc, float deltaSec);

    PhysicsPoint p1, p2;
    float length = 0, angle = 0, stiffness = 0;
    float lastAngle = 0, currentAngle = 0, angleVelocity = 0;
    long long lastTime = 0, currentTime = 0;
    std::vector<PhysicsSrc> sourceParams;
    std::vector<PhysicsTarget> targetParams;
};
}   // namespace V2
}   // namespace Live2D