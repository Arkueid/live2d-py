#include "FileUtils.hpp"

#include <Log.hpp>

#include <cstdio>
#include <cstring>
#include <fstream>
#include <filesystem>

using namespace Live2D::Common::Log;

namespace Live2D {
namespace Common {

std::string FileUtils::SHADER_DIR = "";

unsigned char* FileUtils::LoadFileAsBytes(const std::string filePath, unsigned int* outSize) {
    std::string pathStr = filePath;
    interceptShaderLoading(pathStr);
    std::filesystem::path path = std::filesystem::u8path(pathStr);

    size_t size = 0;
    if (std::filesystem::exists(path)) {
        size = std::filesystem::file_size(path);
        if (size == 0) {
            LOGI("Stat succeeded but file size is zero. path:%s", pathStr.c_str());
            return NULL;
        }
    } else {
        LOGI("Stat failed. path:%s", pathStr.c_str());
        return NULL;
    }

    std::fstream file;
    file.open(path, std::ios::in | std::ios::binary);
    if (!file.is_open()) {
        LOGI("File open failed. path:%s", pathStr.c_str());
        return NULL;
    }

    char* buf = new char[size];
    file.read(buf, size);
    file.close();

    if (outSize) {
        *outSize = static_cast<unsigned int>(size);
    }

    return reinterpret_cast<unsigned char*>(buf);
}

void FileUtils::ReleaseBytes(unsigned char* byteData) {
    delete[] byteData;
}

void FileUtils::PrintLn(const char* message) {
    if (IsLive2DLogEnabled()) {
        printf("%s\n", message);
    }
}

void FileUtils::InitShaderDir(const std::string& path) {
    SHADER_DIR = path;
    SHADER_DIR += std::filesystem::path::preferred_separator;
    LOGI("[FileUtils] Init Shader Dir: %s", SHADER_DIR.c_str());
}

void FileUtils::interceptShaderLoading(std::string& filePath) {
    if (filePath.substr(0, 17) == "FrameworkShaders/") {
        LOGD("[FileUtils] intercept for shader: %s", filePath.c_str());
        filePath = SHADER_DIR + (char)std::filesystem::path::preferred_separator + filePath;
    }
}

}  // namespace Common
}  // namespace Live2D
