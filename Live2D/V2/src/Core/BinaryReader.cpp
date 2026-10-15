#include "BinaryReader.hpp"
#include "DEF.hpp"
#include "ISerializable.hpp"
#include "Id.hpp"
#include "ObjectFactory.hpp"
#include <cstring>

namespace Live2D {
namespace V2 {

// -- Helper for big-endian conversion --
namespace {

inline float ReadBigFloat(const uint8_t* buf, size_t offset) {
    uint32_t raw;
    memcpy(&raw, buf + offset, 4);
    // Big-endian to native
    raw = ((raw & 0x000000FFU) << 24) | ((raw & 0x0000FF00U) << 8) | ((raw & 0x00FF0000U) >> 8) |
          ((raw & 0xFF000000U) >> 24);
    float result;
    memcpy(&result, &raw, 4);
    return result;
}

inline int32_t ReadBigInt32(const uint8_t* buf, size_t offset) {
    uint32_t raw;
    memcpy(&raw, buf + offset, 4);
    return static_cast<int32_t>(((raw & 0x000000FFU) << 24) | ((raw & 0x0000FF00U) << 8) |
                                ((raw & 0x00FF0000U) >> 8) | ((raw & 0xFF000000U) >> 24));
}

inline int16_t ReadBigShort(const uint8_t* buf, size_t offset) {
    uint16_t raw;
    memcpy(&raw, buf + offset, 2);
    return static_cast<int16_t>(((raw & 0x00FFU) << 8) | ((raw & 0xFF00U) >> 8));
}

inline double readBigDouble(const uint8_t* buf, size_t offset) {
    uint64_t raw;
    memcpy(&raw, buf + offset, 8);
    raw = ((raw & 0x00000000000000FFULL) << 56) | ((raw & 0x000000000000FF00ULL) << 40) |
          ((raw & 0x0000000000FF0000ULL) << 24) | ((raw & 0x00000000FF000000ULL) << 8) |
          ((raw & 0x000000FF00000000ULL) >> 8) | ((raw & 0x0000FF0000000000ULL) >> 24) |
          ((raw & 0x00FF000000000000ULL) >> 40) | ((raw & 0xFF00000000000000ULL) >> 56);
    double result;
    memcpy(&result, &raw, 8);
    return result;
}

}   // anonymous namespace

// -- BinaryReader implementation --

BinaryReader::~BinaryReader() {
    for (auto& del : mDeleters)
        del();
}

int BinaryReader::ReadNumber() {
    int b1 = ReadByte();
    if ((b1 & 128) == 0)
        return b1 & 255;

    int b2 = ReadByte();
    if ((b2 & 128) == 0)
        return ((b1 & 127) << 7) | (b2 & 127);

    int b3 = ReadByte();
    if ((b3 & 128) == 0)
        return ((b1 & 127) << 14) | ((b2 & 127) << 7) | (b3 & 255);

    int b4 = ReadByte();
    if ((b4 & 128) == 0)
        return ((b1 & 127) << 21) | ((b2 & 127) << 14) | ((b3 & 127) << 7) | (b4 & 255);

    LOGE("number parse error");
    return 0;
}

double BinaryReader::ReadDouble() {
    CheckBits();
    double result = readBigDouble(mBuf.data(), mOffset);
    mOffset += 8;
    return result;
}

float BinaryReader::ReadFloat32() {
    CheckBits();
    float result = ReadBigFloat(mBuf.data(), mOffset);
    mOffset += 4;
    return result;
}

int32_t BinaryReader::ReadInt32() {
    CheckBits();
    int32_t result = ReadBigInt32(mBuf.data(), mOffset);
    mOffset += 4;
    return result;
}

uint8_t BinaryReader::ReadByte() {
    CheckBits();
    return mBuf[mOffset++];
}

int16_t BinaryReader::ReadUShort() {
    CheckBits();
    int16_t result = ReadBigShort(mBuf.data(), mOffset);
    mOffset += 2;
    return result;
}

bool BinaryReader::ReadBoolean() {
    CheckBits();
    return mBuf[mOffset++] != 0;
}

std::string BinaryReader::ReadUTF8String() {
    CheckBits();
    int length = ReadType();
    std::string result(reinterpret_cast<const char*>(&mBuf[mOffset]), length);
    mOffset += length;
    return result;
}

std::vector<int32_t> BinaryReader::ReadInt32Array() {
    CheckBits();
    int count = ReadType();
    std::vector<int32_t> result(count);
    for (int i = 0; i < count; i++) {
        result[i] = ReadInt32();
    }
    return result;
}

std::vector<float> BinaryReader::ReadFloat32Array() {
    CheckBits();
    int count = ReadType();
    std::vector<float> result(count);
    for (int i = 0; i < count; i++) {
        result[i] = ReadFloat32();
    }
    return result;
}

std::vector<double> BinaryReader::ReadFloat64Array() {
    CheckBits();
    int count = ReadType();
    std::vector<double> result(count);
    for (int i = 0; i < count; i++) {
        result[i] = ReadDouble();
    }
    return result;
}

bool BinaryReader::ReadBit() {
    if (mOffset8Bit == 0) {
        mCurrent8Bit = ReadByte();
    } else if (mOffset8Bit == 8) {
        mCurrent8Bit = ReadByte();
        mOffset8Bit = 0;
    }

    bool result = ((mCurrent8Bit >> (7 - mOffset8Bit)) & 1) == 1;
    mOffset8Bit++;
    return result;
}

void BinaryReader::CheckBits() {
    if (mOffset8Bit != 0) {
        mOffset8Bit = 0;
    }
}

void* BinaryReader::ReadObjectRaw(int type) {
    return ReadObjectUntyped(type);
}

void* BinaryReader::ReadObjectUntyped(int type) {
    CheckBits();
    if (type < 0)
        type = ReadType();

    if (type == 0) {
        mObjects.push_back(nullptr);
        return nullptr;
    }

    if (type == OBJECT_REF) {
        int index = ReadInt32();
        if (index < 0 || index >= static_cast<int>(mObjects.size())) {
            LOGE("Invalid object reference index: %d (size=%zu)", index, mObjects.size());
            return nullptr;
        }
        return mObjects[index];
    }

    void* obj = ReadKnownTypeObject(type);
    mObjects.push_back(obj);
    return obj;
}

void* BinaryReader::ReadKnownTypeObject(int type) {
    if (type == 0) {
        return nullptr;
    }
    // String-to-Id types: 50, 51, 134, 60
    if (type == 50 || type == 51 || type == 134 || type == 60) {
        std::string idStr = ReadUTF8String();
        return const_cast<Id*>(&Id::GetID(idStr));
    }
    // Factory-created ISerializable subclasses (type >= 48)
    if (type >= 48) {
        auto obj = ObjectFactory::Create(type);
        obj->Read(*this);   // may throw — unique_ptr still owns
        ISerializable* rawPtr = obj.release();
        return rawPtr;   // ownership transferred to caller
    }
    // Primitive types
    if (type == 1) {
        auto* s = new std::string(ReadUTF8String());
        mDeleters.push_back([s]() { delete s; });
        return s;
    }
    if (type == 15) {
        int count = ReadType();
        auto* arr = new std::vector<void*>();
        mDeleters.push_back([arr]() { delete arr; });
        arr->reserve(count);
        for (int i = 0; i < count; i++) {
            arr->push_back(ReadObjectUntyped());
        }
        return arr;
    }
    if (type == 16 || type == 25) {
        auto* arr = new std::vector<int32_t>(ReadInt32Array());
        mDeleters.push_back([arr]() { delete arr; });
        return arr;
    }
    if (type == 26) {
        auto* arr = new std::vector<double>(ReadFloat64Array());
        mDeleters.push_back([arr]() { delete arr; });
        return arr;
    }
    if (type == 27) {
        auto* arr = new std::vector<float>(ReadFloat32Array());
        mDeleters.push_back([arr]() { delete arr; });
        return arr;
    }
    if (type == 23) {
        LOGE("type not implemented");
        return nullptr;
    }

    LOGE("type error: %d", type);
    return nullptr;
}

}   // namespace V2
}   // namespace Live2D
