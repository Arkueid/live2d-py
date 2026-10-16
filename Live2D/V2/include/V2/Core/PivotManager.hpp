#pragma once
#include <memory>

#include "ISerializable.hpp"
#include "Id.hpp"
#include <cstdint>
#include <vector>

namespace Live2D {
namespace V2 {

class Moc;

/**
 * PivotManager — pivot 插值表求值器。
 * 每个 deformer/drawable 持有一个，内含作用在其上的全部参数的 pivot
 * 键位表（Pivot），负责参数变化检测与多维插值索引计算。
 * （V3 无对应 Framework 类，等价逻辑在 Core 的 keyform 绑定求值内部。）
 */
class PivotManager final : public ISerializable {
public:
    /// 单个参数的 pivot 键位（参数 id + 分段点值 + 运行时求值缓存）
    class Pivot final : public ISerializable {
    public:
        static constexpr int PARAM_INDEX_NOT_INIT = -2;

        Pivot() = default;

        void Read(class BinaryReader& br) override;

        int GetParamIndex(int initVersion);
        void SetParamIndex(int index, int initVersion) {
            mParamIndex = index;
            mInitVersion = initVersion;
        }

        const Id* GetId() const { return mParamId; }
        int GetPivotCount() const { return mPivotCount; }
        const std::vector<float>& GetPivotValues() const { return mPivotValues; }

        int GetTmpPivotIndex() const { return mTmpPivotIndex; }
        void SetTmpPivotIndex(int index) { mTmpPivotIndex = index; }
        float GetTmpT() const { return mTmpT; }
        void SetTmpT(float value) { mTmpT = value; }

    private:
        int mPivotCount = 0;
        const Id* mParamId = nullptr;
        std::vector<float> mPivotValues;
        int mParamIndex = PARAM_INDEX_NOT_INIT;
        int mInitVersion = -1;
        int mTmpPivotIndex = 0;
        float mTmpT = 0.0f;
    };

    PivotManager() = default;
    ~PivotManager() override;

    void Read(class BinaryReader& br) override;

    int CalcPivotValues(Moc* moc, bool& outRet);
    void CalcPivotIndices(std::vector<int16_t>& indexArray, std::vector<float>& tArray,
                          int interpolationCount);

    int GetParamCount() const { return static_cast<int>(mParamPivotTable.size()); }

private:
    std::vector<std::unique_ptr<Pivot>> mParamPivotTable;
};

}   // namespace V2
}   // namespace Live2D
