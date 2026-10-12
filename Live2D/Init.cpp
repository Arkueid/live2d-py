#include "Init.hpp"

#include <FileUtils.hpp>
#include <V3/LAppAllocator.hpp>

#include <CubismFramework.hpp>

namespace Live2D {
namespace {
V3::LAppAllocator sAllocator;
Csm::CubismFramework::Option sOption;
bool sInitialized = false;
}   // namespace

void Init(const char* shaderDir, int logLevel) {
    if (sInitialized) {
        return;
    }

    Common::FileUtils::InitShaderDir(shaderDir ? shaderDir : "");

    sOption.LogFunction = Common::FileUtils::PrintLn;
    sOption.LoggingLevel = static_cast<Csm::CubismFramework::Option::LogLevel>(logLevel);
    sOption.LoadFileFunction = Common::FileUtils::LoadFileAsBytes;
    sOption.ReleaseBytesFunction = Common::FileUtils::ReleaseBytes;

    Csm::CubismFramework::StartUp(&sAllocator, &sOption);
    Csm::CubismFramework::Initialize();

    sInitialized = true;
}

void Dispose() {
    if (!sInitialized) {
        return;
    }
    Csm::CubismFramework::Dispose();
    sInitialized = false;
}

}   // namespace Live2D
