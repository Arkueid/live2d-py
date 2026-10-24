#include "Cli.hpp"

#include <cstdio>
#include <string>

#include "ModelCatalog.hpp"

bool ParseOptions(int argc, char* argv[], AppOptions& out) {
    // 未指定时按编译期后端决定默认值；Dual 构建下 --opengl / --vulkan 可覆盖
#if defined(CSM_TARGET_WIN_GL) || defined(CSM_TARGET_LINUX_GL) || defined(CSM_TARGET_MAC_GL)
    out.backend = Live2D::Common::Backend::BACKEND_OPENGL;
#endif
#ifdef CSM_TARGET_VULKAN
    out.backend = Live2D::Common::Backend::BACKEND_VULKAN;
#endif

    int pathArg = 1;
    bool hasAll = false;
    for (int i = 0; i < argc; i++) {
        const std::string arg = argv[i];
        if (arg == "--smoke") {
            out.smoke = true;
            pathArg++;
        } else if (arg == "--vulkan") {
            out.backend = Live2D::Common::Backend::BACKEND_VULKAN;
            pathArg++;
        } else if (arg == "--opengl") {
            out.backend = Live2D::Common::Backend::BACKEND_OPENGL;
            pathArg++;
        } else if (arg == "all") {
            hasAll = true;
        }
    }
    out.smokeAll = out.smoke && hasAll;

    // --smoke all：路径在 RunSmokeAll 内逐个解析
    if (out.smokeAll)
        return true;

    if (argc > pathArg) {
        out.modelPath = ResolveModelPath(argv[pathArg]);
        return !out.modelPath.empty();
    }

    out.modelPath = DefaultModelPath();
    if (out.modelPath.empty()) {
        fprintf(stderr, "ERROR: Cannot find llny.model3.json\n");
        PrintUsage(argc > 0 ? argv[0] : "Live2DTest.exe");
        return false;
    }
    return true;
}

void PrintUsage(const char* exePath) {
    fprintf(stderr,
            "Usage: %s [--smoke] [--opengl|--vulkan] [all | model-name | path/to/model.json]\n",
            exePath);
}
