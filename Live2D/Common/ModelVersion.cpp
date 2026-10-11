#include "ModelVersion.hpp"

#include <cstring>
#include <filesystem>
#include <fstream>
#include <string>

#include "Log.hpp"
#include "nlohmann/json.hpp"

namespace Live2D {

using json = nlohmann::json;
using namespace Live2D::Common::Log;

// 读取 json 同目录下的 moc 文件并核对魔数；magicLen 为魔数字节数
static bool checkMocMagic(const std::filesystem::path& jsonDir, const std::string& mocPath,
                          const char* magic, std::streamsize magicLen) {
    const std::filesystem::path fp = jsonDir / std::filesystem::u8path(mocPath);
    std::ifstream mocFile(fp, std::ios::binary);
    if (!mocFile) {
        LOGE("cannot open %s", mocPath.c_str());
        return false;
    }
    char buf[4] = {};
    mocFile.read(buf, magicLen);
    if (mocFile.gcount() != magicLen || memcmp(buf, magic, (size_t)magicLen) != 0) {
        LOGE("%s is not a valid %s", mocPath.c_str(), magicLen == 3 ? "moc" : "moc3");
        return false;
    }
    return true;
}

int DetectModelVersion(const char* modelJsonPath) {
    if (modelJsonPath == nullptr) {
        return -1;
    }
    const std::filesystem::path jp = std::filesystem::u8path(modelJsonPath);
    std::ifstream jsonFile(jp);
    json data = json::parse(jsonFile, nullptr, false);
    if (data.is_discarded() || !data.is_object()) {
        LOGE("%s is not a valid json", modelJsonPath);
        return -1;
    }
    const std::filesystem::path dir = jp.parent_path();

    // v2: 顶层 "model" 字段
    if (auto modelIt = data.find("model"); modelIt != data.end() && modelIt->is_string()) {
        return checkMocMagic(dir, modelIt->get<std::string>(), "moc", 3) ? 2 : -1;
    }

    // v3: FileReferences.Moc
    if (auto frIt = data.find("FileReferences"); frIt != data.end() && frIt->is_object()) {
        if (auto mocIt = frIt->find("Moc"); mocIt != frIt->end() && mocIt->is_string()) {
            return checkMocMagic(dir, mocIt->get<std::string>(), "MOC3", 4) ? 3 : -1;
        }
    }

    LOGE("cannot determine model version: %s", modelJsonPath);
    return -1;
}

}   // namespace Live2D
