#include "ClippingManagerVulkan.hpp"
#include "DEF.hpp"
#include "IDrawData.hpp"
#include "Mesh.hpp"
#include "MeshContext.hpp"
#include "ModelContext.hpp"
#include "VkRenderer.hpp"
#include <algorithm>
#include <cstring>

namespace Live2D {
namespace V2 {

static constexpr int CHANNEL_COUNT = 4;

ClippingManagerVulkan::ClippingManagerVulkan(VkRenderer& renderer)
    : mRenderer(renderer) {
    // 与 GL 版一致: ch0=Alpha, ch1=Red, ch2=Green, ch3=Blue
    mChannelColors.resize(CHANNEL_COUNT);
    mChannelColors[0] = {0.0f, 0.0f, 0.0f, 1.0f};
    mChannelColors[1] = {1.0f, 0.0f, 0.0f, 0.0f};
    mChannelColors[2] = {0.0f, 1.0f, 0.0f, 0.0f};
    mChannelColors[3] = {0.0f, 0.0f, 1.0f, 0.0f};
}

ClippingManagerVulkan::~ClippingManagerVulkan() = default;

static ClipContext* findSameClip(const std::vector<std::unique_ptr<ClipContext>>& list,
                                 const std::vector<std::string>& ids) {
    for (auto& ctx : list) {
        if (ctx->mClipIDList.size() != ids.size())
            continue;
        int match = 0;
        for (auto& a : ctx->mClipIDList)
            for (auto& b : ids)
                if (a == b) {
                    match++;
                    break;
                }
        if (match == (int)ids.size())
            return ctx.get();
    }
    return nullptr;
}

void ClippingManagerVulkan::init(ModelContext* modelContext,
                                 const std::vector<IDrawData*>& drawDataList,
                                 const std::vector<MeshContext*>& drawContextList) {
    for (size_t i = 0; i < drawDataList.size(); i++) {
        auto& clipIDs = drawDataList[i]->getClipIDList();
        if (clipIDs.empty())
            continue;

        auto* clipCtx = findSameClip(mClipContextList, clipIDs);
        if (!clipCtx) {
            auto newCtx = std::make_unique<ClipContext>(modelContext, clipIDs);
            clipCtx = newCtx.get();
            mClipContextList.push_back(std::move(newCtx));
        }

        auto* drawId = drawDataList[i]->getId();
        int drawIdx = modelContext->getDrawDataIndex(drawId);
        for (auto& cid : clipIDs) {
            int maskIdx = modelContext->getDrawDataIndex(&Id::getID(cid));
            if (maskIdx >= 0) {
                bool found = false;
                for (int m : clipCtx->mClippingMaskDrawIndexList)
                    if (m == maskIdx) {
                        found = true;
                        break;
                    }
                if (!found)
                    clipCtx->mClippingMaskDrawIndexList.push_back(maskIdx);
            }
        }
        clipCtx->addClippedDrawData(drawId->str(), drawIdx);
        drawContextList[i]->mClipBufPre_clipContext = clipCtx;
    }
}

void ClippingManagerVulkan::calcClippedDrawTotalBounds(ModelContext* modelContext,
                                                       ClipContext* clip) {
    float minX = 0, minY = 0, maxX = 0, maxY = 0;

    int nVertOk = 0;
    for (int idx : clip->mClippedDrawIndexList) {
        auto* ctx = modelContext->getDrawContext(idx);
        if (!ctx->mAvailable)
            continue;

        auto& pts =
            ctx->mTransformedPoints.empty() ? ctx->mInterpolatedPoints : ctx->mTransformedPoints;
        if (pts.size() < 2)
            continue;
        nVertOk++;

        for (size_t j = VERTEX_OFFSET; j < pts.size(); j += VERTEX_STEP) {
            float x = pts[j], y = pts[j + 1];
            if (x < minX)
                minX = x;
            if (y < minY)
                minY = y;
            if (x > maxX)
                maxX = x;
            if (y > maxY)
                maxY = y;
        }
    }

    if (nVertOk == 0) {
        clip->mIsUsing = false;
    } else {
        clip->mAllClippedDrawRect[0] = minX;
        clip->mAllClippedDrawRect[1] = minY;
        clip->mAllClippedDrawRect[2] = maxX - minX;
        clip->mAllClippedDrawRect[3] = maxY - minY;
        clip->mIsUsing = true;
    }
}

void ClippingManagerVulkan::setupLayoutBounds(int count) {
    int rows = count / CHANNEL_COUNT;
    int remainder = count % CHANNEL_COUNT;
    int idx = 0;

    for (int ch = 0; ch < CHANNEL_COUNT; ch++) {
        int n = rows + (ch < remainder ? 1 : 0);
        if (n == 1) {
            auto& clip = mClipContextList[idx++];
            clip->mLayoutChannelNo = ch;
            clip->mLayoutBounds = {0, 0, 1, 1};
        } else if (n == 2) {
            for (int i = 0; i < n; i++) {
                auto& clip = mClipContextList[idx++];
                clip->mLayoutChannelNo = ch;
                clip->mLayoutBounds = {(float)(i % 2) * 0.5f, 0, 0.5f, 1};
            }
        } else if (n <= 4) {
            for (int i = 0; i < n; i++) {
                auto& clip = mClipContextList[idx++];
                clip->mLayoutChannelNo = ch;
                clip->mLayoutBounds = {(float)(i % 2) * 0.5f, (float)(i / 2) * 0.5f, 0.5f, 0.5f};
            }
        } else if (n <= 9) {
            for (int i = 0; i < n; i++) {
                auto& clip = mClipContextList[idx++];
                clip->mLayoutChannelNo = ch;
                clip->mLayoutBounds = {(float)(i % 3) / 3.0f, (float)(i / 3) / 3.0f, 1.0f / 3.0f,
                                       1.0f / 3.0f};
            }
        }
    }
}

// 裁剪矩阵推导与 ClippingManagerOpenGL::buildClipMatrix 逐行一致
static void buildClipMatrix(std::array<float, 16>& out, ClipContext* clip, bool forMask) {
    out.fill(0);
    float bx = clip->mLayoutBounds[0], by = clip->mLayoutBounds[1];
    float bw = clip->mLayoutBounds[2], bh = clip->mLayoutBounds[3];
    float cx = clip->mAllClippedDrawRect[0], cy = clip->mAllClippedDrawRect[1];
    float cw = clip->mAllClippedDrawRect[2], ch = clip->mAllClippedDrawRect[3];

    if (cw <= 0 || ch <= 0) {
        out[0] = out[5] = out[10] = out[15] = 1;
        return;
    }

    float pad = 0.05f;
    float ex = cw * pad, ey = ch * pad;
    float ew = cw + ex * 2, eh = ch + ey * 2;
    float sx = bw / ew;
    float sy = bh / eh;

    if (forMask) {
        out[0] = 2.0f * sx;
        out[5] = 2.0f * sy;
        out[10] = 1.0f;
        out[12] = -1.0f + 2.0f * bx - 2.0f * sx * (cx - ex);
        out[13] = -1.0f + 2.0f * by - 2.0f * sy * (cy - ey);
        out[15] = 1.0f;
    } else {
        out[0] = sx;
        out[5] = sy;
        out[10] = 1.0f;
        out[12] = bx - sx * (cx - ex);
        out[13] = by - sy * (cy - ey);
        out[15] = 1.0f;
    }
}

bool ClippingManagerVulkan::setupClip(ModelContext* modelContext, VkCommandBuffer cmd) {
    int activeCount = 0;
    for (auto& clip : mClipContextList) {
        calcClippedDrawTotalBounds(modelContext, clip.get());
        if (clip->mIsUsing)
            activeCount++;
    }
    if (activeCount == 0)
        return false;

    setupLayoutBounds(activeCount);

    for (auto& clip : mClipContextList) {
        if (!clip->mIsUsing)
            continue;

        buildClipMatrix(clip->mMatrixForMask, clip.get(), true);
        buildClipMatrix(clip->mMatrixForDraw, clip.get(), false);

        for (int idx : clip->mClippedDrawIndexList) {
            auto* ctx = modelContext->getDrawContext(idx);
            std::memcpy(ctx->mClipMatrixForDraw, clip->mMatrixForDraw.data(), 16 * sizeof(float));
        }

        // 在 mask pass 中渲染遮罩源 drawable（normal 管线 + maskFlag=1，matrixForMask 作 mvp）
        for (int idx : clip->mClippingMaskDrawIndexList) {
            auto* mesh = static_cast<Mesh*>(modelContext->getDrawData(idx));
            auto* ctx = modelContext->getDrawContext(idx);
            mRenderer.setClipMatrix(clip->mMatrixForMask.data());
            mRenderer.setClipChannel(clip->mLayoutChannelNo);
            mRenderer.setClipMaskMode(true);
            mesh->draw(&mRenderer, modelContext, ctx);
            mRenderer.setClipMaskMode(false);
        }
    }
    return true;
}

}   // namespace V2
}   // namespace Live2D
