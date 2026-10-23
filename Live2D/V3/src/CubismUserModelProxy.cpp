#include "GraphicsBackend.hpp"
#include "Log.hpp"
#include "Rendering/CubismRenderer.hpp"
#if defined(CSM_TARGET_WIN_GL) || defined(CSM_TARGET_LINUX_GL) || defined(CSM_TARGET_MAC_GL)
#include "Rendering/OpenGL/CubismRenderer_OpenGLES2.hpp"
#endif
#ifdef CSM_TARGET_VULKAN
#include "Rendering/Vulkan/CubismRenderer_Vulkan.hpp"
#endif
#include <CubismUserModelProxy.hpp>

#include <CubismFramework.hpp>
#include <Utils/CubismJson.hpp>

#include <string>

using namespace Live2D::Cubism::Framework;

namespace Live2D {
Rendering::CubismRenderer* Rendering::CubismRenderer::Create(csmUint32 width, csmUint32 height) {
#ifdef CSM_TARGET_VULKAN
    if (Common::Backend::IsVulkan()) {
        return CSM_NEW Rendering::CubismRenderer_Vulkan(width, height);
    }
#endif
#if defined(CSM_TARGET_WIN_GL) || defined(CSM_TARGET_LINUX_GL) || defined(CSM_TARGET_MAC_GL)
    if (Common::Backend::IsOpenGL()) {
        return CSM_NEW Rendering::CubismRenderer_OpenGLES2(width, height);
    }
#endif
    LOGF("Backend is not set or invalid");
}

void Rendering::CubismRenderer::StaticRelease() {
#ifdef CSM_TARGET_VULKAN
    if (Common::Backend::IsVulkan()) {
        return CubismRenderer_Vulkan::DoStaticRelease();
    }
#endif
#if defined(CSM_TARGET_WIN_GL) || defined(CSM_TARGET_LINUX_GL) || defined(CSM_TARGET_MAC_GL)
    if (Common::Backend::IsOpenGL()) {
        return CubismRenderer_OpenGLES2::DoStaticRelease();
    }
#endif
}

namespace V3 {

namespace {

/**
 * @brief 修复 motion3.json 中 Meta 计数字段与实际数据不一致的问题。
 *
 * 某些编辑器导出的 motion3.json 中 CurveCount / TotalSegmentCount /
 * TotalPointCount 与 Curves 实际数据不一致，导致动作加载失败。
 */
void FixMotionJson(std::string& jsonStr) {
    using namespace Live2D::Cubism::Framework::Utils;
    CubismJson* json = CubismJson::Create(reinterpret_cast<const csmByte*>(jsonStr.data()),
                                          static_cast<csmSizeInt>(jsonStr.size()));
    if (!json)
        return;

    Value& root = json->GetRoot();
    auto* curvesVec = root["Curves"].GetVector();
    if (!curvesVec) {
        CubismJson::Delete(json);
        return;
    }

    const csmInt32 actualCurveCount = static_cast<csmInt32>(curvesVec->GetSize());
    csmInt32 actualSegmentCount = 0;
    csmInt32 actualPointCount = 0;

    enum { Linear = 0, Bezier = 1, Stepped = 2, InverseStepped = 3 };

    Value& curves = root["Curves"];
    for (csmInt32 ci = 0; ci < actualCurveCount; ci++) {
        auto* segVec = curves[ci]["Segments"].GetVector();
        if (!segVec)
            continue;
        const csmInt32 segLen = static_cast<csmInt32>(segVec->GetSize());
        if (segLen < 2)
            continue;
        actualPointCount++;
        csmInt32 v = 2;
        while (v < segLen) {
            csmInt32 ident = static_cast<csmInt32>((*segVec)[v]->ToInt());
            if (ident == Bezier) {
                actualPointCount += 3;
                v += 7;
            } else if (ident == Linear || ident == Stepped || ident == InverseStepped) {
                actualPointCount += 1;
                v += 3;
            } else
                break;
            actualSegmentCount++;
        }
    }

    int metaCurveCount = root["Meta"]["CurveCount"].ToInt(-1);
    int metaSegCount = root["Meta"]["TotalSegmentCount"].ToInt(-1);
    int metaPointCount = root["Meta"]["TotalPointCount"].ToInt(-1);

    CubismJson::Delete(json);

    if (metaCurveCount == actualCurveCount && metaSegCount == actualSegmentCount &&
        metaPointCount == actualPointCount)
        return;

    auto fixInt = [&](const char* key, int newVal) {
        size_t kp = jsonStr.find(key);
        if (kp == std::string::npos)
            return;
        size_t colon = jsonStr.find(':', kp);
        if (colon == std::string::npos)
            return;
        size_t vs = colon + 1;
        while (vs < jsonStr.size() && (jsonStr[vs] == ' ' || jsonStr[vs] == '\t'))
            vs++;
        size_t ve = vs;
        while (ve < jsonStr.size() && jsonStr[ve] >= '0' && jsonStr[ve] <= '9')
            ve++;
        if (ve > vs)
            jsonStr.replace(vs, ve - vs, std::to_string(newVal));
    };

    fixInt("\"CurveCount\"", actualCurveCount);
    fixInt("\"TotalSegmentCount\"", actualSegmentCount);
    fixInt("\"TotalPointCount\"", actualPointCount);
}

}   // namespace

Csm::ACubismMotion* CubismUserModelProxy::LoadMotion(const Csm::csmByte* buffer,
                                                     Csm::csmSizeInt size, const Csm::csmChar* name,
                                                     Csm::ICubismModelSetting* modelSetting,
                                                     const Csm::csmChar* group, Csm::csmInt32 index,
                                                     csmBool shouldCheckMotionConsistency) {
    std::string fixed(reinterpret_cast<const char*>(buffer), size);
    FixMotionJson(fixed);
    return CubismUserModel::LoadMotion(reinterpret_cast<const Csm::csmByte*>(fixed.data()),
                                       static_cast<Csm::csmSizeInt>(fixed.size()),
                                       name,
                                       modelSetting,
                                       group,
                                       index,
                                       shouldCheckMotionConsistency);
}

bool CubismUserModelProxy::IsHit(CubismIdHandle drawableId, Csm::csmFloat32 pointX,
                                 Csm::csmFloat32 pointY) {
    const Csm::csmInt32 drawIndex = _model->GetDrawableIndex(drawableId);

    if (drawIndex < 0) {
        return false;   // 存在しない場合はfalse
    }

    const Csm::csmInt32 count = _model->GetDrawableVertexCount(drawIndex);
    const Csm::csmFloat32* vertices = _model->GetDrawableVertices(drawIndex);

    Csm::csmFloat32 left = vertices[0];
    Csm::csmFloat32 right = vertices[0];
    Csm::csmFloat32 top = vertices[1];
    Csm::csmFloat32 bottom = vertices[1];

    for (Csm::csmInt32 j = 1; j < count; ++j) {
        Csm::csmFloat32 x = vertices[Constant::VertexOffset + j * Constant::VertexStep];
        Csm::csmFloat32 y = vertices[Constant::VertexOffset + j * Constant::VertexStep + 1];

        if (x < left) {
            left = x;   // Min x
        }

        if (x > right) {
            right = x;   // Max x
        }

        if (y < top) {
            top = y;   // Min y
        }

        if (y > bottom) {
            bottom = y;   // Max y
        }
    }

    return ((left <= pointX) && (pointX <= right) && (top <= pointY) && (pointY <= bottom));
}

}   // namespace V3
}   // namespace Live2D
