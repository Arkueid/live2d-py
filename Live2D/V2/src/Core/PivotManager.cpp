#include "PivotManager.hpp"
#include "Moc.hpp"
#include "BinaryReader.hpp"
#include "DEF.hpp"
#include <cstdio>
#include <stdexcept>

namespace Live2D {
namespace V2 {

// -- Pivot --

void PivotManager::Pivot::Read(BinaryReader& br) {
    mParamId = br.ReadObject<const Id*>();
    mPivotCount = br.ReadInt32();
    mPivotValues = br.ReadObject<std::vector<float>>();
}

int PivotManager::Pivot::GetParamIndex(int initVersion) {
    if (mInitVersion != initVersion) {
        mParamIndex = PARAM_INDEX_NOT_INIT;
    }
    return mParamIndex;
}

// -- PivotManager --

PivotManager::~PivotManager() = default;

void PivotManager::Read(BinaryReader& br) {
    auto raw = br.ReadObject<std::vector<Pivot*>>();
    mParamPivotTable.reserve(raw.size());
    for (auto* p : raw)
        mParamPivotTable.emplace_back(p);
}

int PivotManager::CalcPivotValues(Moc* moc, bool& outRet) {
    int paramCount = static_cast<int>(mParamPivotTable.size());
    int initVersion = moc->GetInitVersion();
    int interpolationCount = 0;

    for (int i = 0; i < paramCount; i++) {
        auto& pp = mParamPivotTable[i];
        int paramIndex = pp->GetParamIndex(initVersion);
        if (paramIndex == Pivot::PARAM_INDEX_NOT_INIT) {
            paramIndex = moc->GetParamIndex(pp->GetId());
            pp->SetParamIndex(paramIndex, initVersion);
        }

        float paramValue = (paramIndex < 0) ? 0.0f : moc->GetParamFloat(paramIndex);
        int pivotCount = pp->GetPivotCount();
        auto& pivotValues = pp->GetPivotValues();

        int pivotIndex = -1;
        float t = 0.0f;

        if (pivotCount < 1) {
            // nothing
        } else if (pivotCount == 1) {
            float pivotValue = pivotValues[0];
            if (pivotValue - GOSA < paramValue && paramValue < pivotValue + GOSA) {
                pivotIndex = 0;
                t = 0.0f;
            } else {
                pivotIndex = 0;
                outRet = true;
            }
        } else {
            float pivotValue = pivotValues[0];
            if (paramValue < pivotValue - GOSA) {
                pivotIndex = 0;
                outRet = true;
            } else if (paramValue < pivotValue + GOSA) {
                pivotIndex = 0;
            } else {
                bool found = false;
                for (int j = 1; j < pivotCount; j++) {
                    float nextPivotValue = pivotValues[j];
                    if (paramValue < nextPivotValue + GOSA) {
                        if (nextPivotValue - GOSA < paramValue) {
                            pivotIndex = j;
                        } else {
                            pivotIndex = j - 1;
                            t = (paramValue - pivotValue) / (nextPivotValue - pivotValue);
                            interpolationCount++;
                        }
                        found = true;
                        break;
                    }
                    pivotValue = nextPivotValue;
                }
                if (!found) {
                    pivotIndex = pivotCount - 1;
                    t = 0.0f;
                    outRet = true;
                }
            }
        }

        pp->SetTmpPivotIndex(pivotIndex);
        pp->SetTmpT(t);
    }
    return interpolationCount;
}

void PivotManager::CalcPivotIndices(std::vector<int16_t>& indexArray, std::vector<float>& tArray,
                                    int interpolationCount) {
    int tableSize = 1 << interpolationCount;
    if (tableSize + 1 > PIVOT_TABLE_SIZE) {
        LOGE("err 23245\n");
    }

    int paramCount = static_cast<int>(mParamPivotTable.size());
    int stride = 1, divisor = 1, tIndex = 0;

    for (int i = 0; i < tableSize; i++)
        indexArray[i] = 0;

    for (int i = 0; i < paramCount; i++) {
        auto& pp = mParamPivotTable[i];
        if (pp->GetTmpT() == 0.0f) {
            int offset = pp->GetTmpPivotIndex() * stride;
            for (int j = 0; j < tableSize; j++)
                indexArray[j] += offset;
        } else {
            int offset1 = stride * pp->GetTmpPivotIndex();
            int offset2 = stride * (pp->GetTmpPivotIndex() + 1);
            for (int j = 0; j < tableSize; j++)
                indexArray[j] += ((j / divisor) % 2 == 0) ? offset1 : offset2;
            tArray[tIndex] = pp->GetTmpT();
            tIndex++;
            divisor *= 2;
        }
        stride *= pp->GetPivotCount();
    }
    indexArray[tableSize] = 65535u;
    tArray[tIndex] = -1.0f;
}

}   // namespace V2
}   // namespace Live2D
