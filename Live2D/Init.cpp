#include "Init.hpp"

#include <V3/LAppPal.hpp>
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

    V3::LAppPal::InitShaderDir(shaderDir ? shaderDir : "");

    sOption.LogFunction = V3::LAppPal::PrintLn;
    sOption.LoggingLevel = static_cast<Csm::CubismFramework::Option::LogLevel>(logLevel);
    sOption.LoadFileFunction = V3::LAppPal::LoadFileAsBytes;
    sOption.ReleaseBytesFunction = V3::LAppPal::ReleaseBytes;

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
