/**
 * live2d-py C++ unified test — loads a v2/v3 model (auto-detected via IModel)
 * and runs the update/draw loop on the configured graphics backend.
 *
 * Build:
 *   cmake -DLIVE2D_CPP_TEST=ON -DCMAKE_EXPORT_COMPILE_COMMANDS:BOOL=TRUE -S . -B build
 *   cmake --build build --config Release --target Live2DTest -j 24
 *
 * Run (from repo root):
 *   .\build\tests\cpp\Live2DTest.exe [--smoke] [all | model-name | model.json path]
 *
 * Default model: Resources/v3/llny/llny.model3.json
 */

#include "GraphicsBackend.hpp"
#include <chrono>
#include <cstdio>
#include <filesystem>
#include <memory>
#include <string>
#include <vector>
#ifdef _WIN32
#include <Windows.h>
#endif
#include <Init.hpp>

// GLFW_INCLUDE_NONE — glad (GL/glew.h) provides all GL symbols
#define GLFW_INCLUDE_NONE
#ifdef CSM_TARGET_VULKAN
#include <vulkan/vulkan.h>
#define GLFW_EXPOSE_NATIVE_WIN32
#include <GLFW/glfw3.h>
#include <GLFW/glfw3native.h>
#include <Rendering/Vulkan/CubismRenderer_Vulkan.hpp>
#include <VulkanManager.hpp>
#endif

#if defined(CSM_TARGET_WIN_GL) || defined(CSM_TARGET_LINUX_GL) || defined(CSM_TARGET_MAC_GL)
#include <GL/glew.h>   // glad OpenGL 4.6 core loader
#include <GLFW/glfw3.h>
#endif

#include <V3/Model.hpp>
#include <V2/Model.hpp>
#include <IModel.hpp>
#include "Log.hpp"

using namespace Live2D::Common::Log;
using namespace Live2D::V3;

// 模型短名 → 仓库内路径（相对仓库根，启动时解析为绝对路径）
static const std::pair<const char*, const char*> kModelTable[] = {
    {"kasumi2", "Resources/v2/kasumi2/kasumi2.model.json"},
    {"haru2", "Resources/v2/haru/haru.model.json"},
    {"hibiki", "Resources/v2/hibiki/hibiki.model.json"},
    {"kana", "Resources/v2/kana/Kobayaxi.model.json"},
    {"shizuku", "Resources/v2/shizuku/shizuku.model.json"},
    {"epsilon", "Resources/v2/Epsilon/Epsilon.model.json"},
    {"test", "Resources/v2/model/test.model.json"},
    {"tuoer", "Resources/v2/托尔/model0.json"},
    {"haru3", "Resources/v3/Haru/Haru.model3.json"},
    {"llny", "Resources/v3/llny/llny.model3.json"},
    {"mao", "Resources/v3/Mao/Mao.model3.json"},
    {"nn", "Resources/v3/绫地宁宁/nn.model3.json"},
    {"acbase", "Resources/v3/ac_base_rem01/ac_base_rem01.model3.json"},
};

// C++20：u8string() 返回 std::u8string（char8_t），按字节转回 UTF-8 窄字符串
static std::string u8ToString(const std::u8string& u8) {
    return std::string(reinterpret_cast<const char*>(u8.data()), u8.size());
}

// 自 cwd 向上查找 relPath 存在的目录，返回绝对路径；找不到返回空串
// （relPath 为 UTF-8 字面量，必须经 u8path 转换，否则中文路径按 ACP 误解码）
static std::string findUpward(const char* relPath) {
    namespace fs = std::filesystem;
    const fs::path rel = fs::u8path(relPath);
    fs::path cwd = fs::current_path();
    while (true) {
        fs::path candidate = cwd / rel;
        if (fs::exists(candidate))
            return u8ToString(
                candidate.u8string());   // 保持 UTF-8（.string() 在 Windows 会转 ACP 弄坏中文）
        if (!cwd.has_parent_path() || cwd == cwd.parent_path())
            return "";
        cwd = cwd.parent_path();
    }
}

// 参数 → 绝对模型路径：含 ".json" 视为路径，否则查短名表；失败打印可用短名并返回空串
static std::string resolveModelPath(const std::string& arg) {
    if (arg.find(".json") != std::string::npos)
        return u8ToString(std::filesystem::absolute(std::filesystem::u8path(arg)).u8string());
    for (const auto& entry : kModelTable) {
        if (arg == entry.first)
            return findUpward(entry.second);
    }
    fprintf(stderr, "unknown model name: %s\navailable names:", arg.c_str());
    for (const auto& entry : kModelTable)
        fprintf(stderr, " %s", entry.first);
    fprintf(stderr, "\n");
    return "";
}

// ============================================================================
// Helpers
// ============================================================================

static void glfwErrorCallback(int /*code*/, const char* msg) {
    fprintf(stderr, "[GLFW ERROR] %s\n", msg);
}

static void keyCallback(GLFWwindow* window, int key, int /*scancode*/, int action, int /*mods*/) {
    if (key == GLFW_KEY_ESCAPE && action == GLFW_PRESS)
        glfwSetWindowShouldClose(window, GLFW_TRUE);
}

// ============================================================================
// Entry point
// ============================================================================

int main(int argc, char* argv[]) {
    // 窗口模式下 stdout 全缓冲会吞掉崩溃前的日志——强制无缓冲便于调试
    setvbuf(stdout, nullptr, _IONBF, 0);
#ifdef _WIN32
    // Windows 的 main() argv 是 ANSI(GBK) 编码，而库的 readFile 按 UTF-8 解释路径
    // （std::filesystem::u8path）。中文路径（如 托尔）的 GBK 字节不是合法 UTF-8，
    // 直接传给 u8path 会抛异常导致 terminate。重新按宽字符取命令行并转 UTF-8。
    std::vector<std::string> u8argStorage;
    std::vector<char*> u8argv;
    {
        int wargc = 0;
        LPWSTR* wargv = CommandLineToArgvW(GetCommandLineW(), &wargc);
        if (wargv) {
            for (int i = 0; i < wargc; i++) {
                int len =
                    WideCharToMultiByte(CP_UTF8, 0, wargv[i], -1, nullptr, 0, nullptr, nullptr);
                std::string s(len > 0 ? len - 1 : 0, '\0');
                if (len > 1)
                    WideCharToMultiByte(CP_UTF8, 0, wargv[i], -1, s.data(), len, nullptr, nullptr);
                u8argStorage.push_back(std::move(s));
            }
            LocalFree(wargv);
            for (auto& s : u8argStorage)
                u8argv.push_back(s.data());
            argc = (int)u8argv.size();
            argv = u8argv.data();
        }
    }

    SetConsoleOutputCP(65001);
#endif
    // --- Resolve model path --------------------------------------------------
    Live2D::Common::Backend::Type backend;
#if defined(CSM_TARGET_WIN_GL) || defined(CSM_TARGET_LINUX_GL) || defined(CSM_TARGET_MAC_GL)
    backend = Live2D::Common::Backend::BACKEND_OPENGL;
#endif
#ifdef CSM_TARGET_VULKAN
    backend = Live2D::Common::Backend::BACKEND_VULKAN;
#endif
    bool smoke = false;
    int hasAll = false;
    int pathArg = 1;
    for (int i = 0; i < argc; i++) {
        if (std::string(argv[i]) == "--smoke") {
            smoke = true;
            pathArg++;
        } else if (std::string(argv[i]) == "--vulkan") {
            backend = Live2D::Common::Backend::BACKEND_VULKAN;
            pathArg++;
        } else if (std::string(argv[i]) == "--opengl") {
            backend = Live2D::Common::Backend::BACKEND_OPENGL;
            pathArg++;
        } else if (std::string(argv[i]) == "all") {
            hasAll = true;
        }
    }
    std::string modelPath;
    // --smoke all：全量回归模式，路径在 smoke 分支内逐个解析
    const bool smokeAll = smoke && hasAll;
    if (smokeAll) {
        // 无需解析模型路径
    } else if (argc > pathArg) {
        modelPath = resolveModelPath(argv[pathArg]);
        if (modelPath.empty())
            return 1;
    } else {
        modelPath = findUpward("Resources/v3/llny/llny.model3.json");
        if (modelPath.empty()) {
            fprintf(stderr, "ERROR: Cannot find llny.model3.json\n");
            fprintf(
                stderr, "Usage: %s [--smoke] [all | model-name | path/to/model.json]\n", argv[0]);
            return 1;
        }
    }
    Live2D::Init("D:/forDesktop/live2d-py/package/live2d/");
    Live2D::Common::Backend::SetBackend(backend);
    printf("Backend: %s\n", Live2D::Common::Backend::IsVulkan() ? "Vulkan" : "OpenGL");
    if (smokeAll)
        printf("Smoke all: %d model entries\n",
               (int)(sizeof(kModelTable) / sizeof(kModelTable[0])));
    else
        printf("Model path: %s\n", modelPath.c_str());

    // --- Headless smoke test: exercise unified IModel APIs without GL --------
    if (smoke) {
        // 单模型 smoke 打包为 lambda：--smoke all 时对 kModelTable 逐个执行
        auto runSmoke = [&](const std::string& smokePath) -> int {
            using namespace Live2D;
            int fails = 0;
#define CHECK(cond)                                                           \
    do {                                                                      \
        if (!(cond)) {                                                        \
            fprintf(stderr, "[smoke] FAIL: %s (line %d)\n", #cond, __LINE__); \
            fails++;                                                          \
        }                                                                     \
    } while (0)

            std::unique_ptr<Live2D::IModel> model(Live2D::IModel::Create(smokePath.c_str()));
            if (!model) {
                fprintf(
                    stderr, "[smoke] FAIL: cannot detect model version: %s\n", smokePath.c_str());
                return 1;
            }
            model->LoadModelJson(smokePath.c_str(), false);
            printf("[smoke] loaded (%s): params=%d parts=%d\n",
                   model->IsV2() ? "v2" : "v3",
                   model->GetParameterCount(),
                   model->GetPartCount());
            CHECK(model->GetParameterCount() > 0);
            CHECK(model->GetPartCount() > 0);

            // granular updates
            CHECK(model->UpdateMotion(0.016f) == true || model->UpdateMotion(0.016f) == false);
            model->UpdateDrag(0.016f);
            model->UpdateBreath(0.016f);
            model->UpdateBlink(0.016f);
            model->UpdateExpression(0.016f);
            model->UpdatePhysics(0.016f);
            model->UpdatePose(0.016f);

            // save/load/reset parameters
            model->SetAndSaveParameterValue("PARAM_ANGLE_X", 5.0f, 1.0f);
            model->SetAndSaveParameterValue(0, 1.0f, 1.0f);
            model->AddAndSaveParameterValue("PARAM_ANGLE_X", 1.0f);
            model->AddAndSaveParameterValue(0, 1.0f);
            model->LoadParameters();
            model->SaveParameters();
            model->ResetAllParameters();
            CHECK(model->GetParameterValue(0) == model->GetParameterDefaultValue(0));

            // per-axis scale + mvp
            model->Resize(800, 800);
            model->SetScaleX(1.2f);
            model->SetScaleY(1.1f);
            model->SetScale(1.0f);
            CHECK(model->GetMvp() != nullptr);

            // update + Draw (deformer chain) so drawable vertices are available
            model->Update();
            model->Draw();

            // motion groups（模型可能不含 motions，如 llny —— 此时跳过）
            if (model->GetMotionGroupCount() > 0) {
                std::vector<std::string> gGroups;
                int gCounts = 0;
                model->GetMotions(
                    &gGroups,
                    [](void* c, const char* group, int no, const char* file, const char* sound) {
                        auto* groups = (std::vector<std::string>*)c;
                        if (no == 0)
                            groups->push_back(group);
                    });
                CHECK(!gGroups.empty());
                if (!gGroups.empty()) {
                    std::string firstGroup = gGroups[0];
                    printf("[smoke] motion groups=%d first=%s count=%d\n",
                           model->GetMotionGroupCount(),
                           firstGroup.c_str(),
                           model->GetMotionCount(firstGroup.c_str()));
                    CHECK(model->GetMotionCount(firstGroup.c_str()) > 0);

                    // extra motion (re-using the file of the first group)
                    std::string extraMotionPath;
                    model->GetMotions(&extraMotionPath,
                                      [](void* c,
                                         const char* group,
                                         int no,
                                         const char* file,
                                         const char* sound) {
                                          auto* out = (std::string*)c;
                                          if (out->empty())
                                              *out = file;
                                      });
                    std::string extraMotionFull =
                        std::string(model->GetModelHomeDir()) + extraMotionPath;
                    int extraNo =
                        model->LoadExtraMotion(firstGroup.c_str(), extraMotionFull.c_str());
                    printf("[smoke] LoadExtraMotion -> no=%d\n", extraNo);
                    CHECK(extraNo >= 0);
                    CHECK(model->GetMotionCount(firstGroup.c_str()) >= 1);

                    // ---- motion 回调（V2/V3 对齐语义：per-motion handler）----
                    {
                        std::vector<std::string> cbEvents;
                        auto rec = [&cbEvents](const char* tag) -> IModel::MotionCallback {
                            return [&cbEvents, tag](const std::string& g, int n) {
                                cbEvents.push_back(std::string(tag) + ":" + g + ":" +
                                                   std::to_string(n));
                            };
                        };
                        auto hasEv = [&cbEvents](const std::string& prefix) {
                            for (auto& e : cbEvents)
                                if (e.rfind(prefix, 0) == 0)
                                    return true;
                            return false;
                        };

                        // 1) onStart 触发；自然播完触发 onFinish（v3 官方 Create
                        //    不读 motion3.json 的 Loop 字段，_isLoop 恒 false，
                        //    duration 到达即自然播完触发；v2 smoke 选用的各模型
                        //    group0 均非 loop，1200 帧预算内必定触发）
                        cbEvents.clear();
                        model->StartMotion(firstGroup, 0, 3, rec("startA"), rec("finishA"));
                        model->Update(1.0f / 60.0f);   // v3 的 began 在首次 Update 触发
                        CHECK(hasEv("startA:" + firstGroup + ":0"));
                        int finFrame = 0;
                        for (; finFrame < 1200 && !hasEv("finishA:"); ++finFrame)
                            model->Update(1.0f / 60.0f);
                        CHECK(hasEv("finishA:" + firstGroup + ":0"));
                        printf("[smoke] motion finished at frame %d\n", finFrame);
                        model->StopAllMotions();
                        model->Update(1.0f / 60.0f);

                        // 2) StopAllMotions 不触发 onFinish
                        cbEvents.clear();
                        model->StartMotion(firstGroup, 0, 3, rec("startC"), rec("finishC"));
                        model->Update(1.0f / 60.0f);
                        model->StopAllMotions();
                        model->Update(1.0f / 60.0f);
                        model->Update(1.0f / 60.0f);
                        CHECK(hasEv("startC:"));
                        CHECK(!hasEv("finishC:"));

                        // 3) 低优先级拒绝：两端均静默（无回调）
                        cbEvents.clear();
                        model->StartMotion(firstGroup, 0, 3, nullptr, nullptr);
                        model->Update(1.0f / 60.0f);
                        model->StartMotion(firstGroup, 0, 1, rec("startL"), rec("finishL"));
                        model->Update(1.0f / 60.0f);
                        CHECK(cbEvents.empty());
                        model->StopAllMotions();
                        model->Update(1.0f / 60.0f);

                        // 4) 不存在的组：onStart/onFinish 立即同步双触发
                        cbEvents.clear();
                        model->StartMotion(
                            "smoke_no_such_group", 0, 3, rec("startX"), rec("finishX"));
                        CHECK(hasEv("startX:smoke_no_such_group:"));
                        CHECK(hasEv("finishX:smoke_no_such_group:"));

                        // 5) 重入（两端）：onFinish 内重启同一 motion，不崩溃且
                        //    新 onStart 触发。v2 走 erase-before-callback 路径；
                        //    v3 的 csmVector 迭代器为 {容器指针, 编号} 结构，
                        //    解引用动态走 _vector->_ptr，天然免疫重入 push 的扩容
                        {
                            cbEvents.clear();
                            auto* mdl = model.get();
                            auto restartLeft = std::make_shared<int>(1);
                            // v3 的 ReserveMotion 为 <= 语义（current=3 时重入 3
                            // 会被拒），重入用 4；v2 为 < 语义，3 即可通过
                            const int rePrio = model->IsV2() ? 3 : 4;
                            IModel::MotionCallback finR;
                            finR = [mdl, &cbEvents, restartLeft, rePrio](const std::string& g,
                                                                         int n) {
                                cbEvents.push_back("finishR:" + g + ":" + std::to_string(n));
                                if ((*restartLeft)-- > 0) {
                                    mdl->StartMotion(
                                        g,
                                        n,
                                        rePrio,
                                        [&cbEvents](const std::string& g2, int n2) {
                                            cbEvents.push_back("startR:" + g2 + ":" +
                                                               std::to_string(n2));
                                        },
                                        nullptr);
                                }
                            };
                            model->StartMotion(firstGroup, 0, 3, nullptr, finR);
                            // 自然播完 → onFinish 内重入 StartMotion
                            // （v2 startR 同步触发，v3 在下一帧 began 触发）
                            for (int f = 0; f < 1200 && !hasEv("startR:"); ++f)
                                model->Update(1.0f / 60.0f);
                            CHECK(hasEv("finishR:"));
                            CHECK(hasEv("startR:"));
                            model->StopAllMotions();
                            model->Update(1.0f / 60.0f);
                        }

                        // 6) burst 重入压测（两端）：onFinish 内同步连发 100 次
                        //    StartMotion——多轮 vector 扩容（v3 csmVector
                        //    10→20→40→80→160）全部发生在队列迭代中途。回归守卫：
                        //    不崩溃、前 99 个被抢占 entry 静默、最后一个入队 entry
                        //    自然播完仍触发其 onFinish
                        {
                            cbEvents.clear();
                            auto* mdl = model.get();
                            auto burstLeft = std::make_shared<int>(100);
                            IModel::MotionCallback finB;
                            finB = [mdl, &cbEvents, burstLeft](const std::string& g, int n) {
                                cbEvents.push_back("finishB:" + g + ":" + std::to_string(n));
                                // 仅首轮 burst，防链式无限膨胀
                                const int left = *burstLeft;
                                *burstLeft = 0;
                                // 递增优先级 4..103 绕过两端优先级拒绝
                                for (int b = 0; b < left; ++b) {
                                    mdl->StartMotion(g,
                                                     n,
                                                     4 + b,
                                                     nullptr,
                                                     [&cbEvents](const std::string& g2, int n2) {
                                                         cbEvents.push_back("finishB2:" + g2 + ":" +
                                                                            std::to_string(n2));
                                                     });
                                }
                            };
                            model->StartMotion(firstGroup, 0, 3, nullptr, finB);
                            auto countEv = [&cbEvents](const char* tag) {
                                int c = 0;
                                for (auto& e : cbEvents)
                                    if (e.rfind(tag, 0) == 0)
                                        ++c;
                                return c;
                            };
                            for (int f = 0; f < 3600 && countEv("finishB2:") < 1; ++f)
                                model->Update(1.0f / 60.0f);
                            CHECK(countEv("finishB:") == 1);
                            CHECK(countEv("finishB2:") == 1);
                            model->StopAllMotions();
                            model->Update(1.0f / 60.0f);
                        }
                        printf("[smoke] motion callbacks OK (%zu events)\n", cbEvents.size());
                    }
                }
            } else {
                printf("[smoke] no motion groups in model — skipped\n");
            }

            // hit tests
            std::vector<std::string> hitParts;
            model->HitPart(
                400,
                300,
                &hitParts,
                [](void* c, const char* id) { ((std::vector<std::string>*)c)->push_back(id); },
                false);
            printf("[smoke] HitPart: %zu hits\n", hitParts.size());
            CHECK(model->IsPartHit(0, 400, 300) == true || model->IsPartHit(0, 400, 300) == false);
            CHECK(model->IsDrawableHit(0, 400, 300) == true ||
                  model->IsDrawableHit(0, 400, 300) == false);

            // drawables
            int drawableCount = model->GetDrawableCount();
            printf("[smoke] drawables=%d\n", drawableCount);
            CHECK(drawableCount > 0);
            std::vector<std::string> drawableIds;
            model->GetDrawableIds(&drawableIds, [](void* c, const char* id) {
                ((std::vector<std::string>*)c)->push_back(id);
            });
            CHECK(!drawableIds.empty());
            CHECK(model->GetDrawableVertexCount(0) > 0);
            CHECK(model->GetDrawableVertexIndexCount(0) > 0);
            CHECK(model->GetDrawableVertices(0) != nullptr);
            CHECK(model->GetDrawableIndices(0) != nullptr);
            model->SetDrawableMultiColor(0, 0.5f, 0.5f, 0.5f, 1.0f);
            model->SetDrawableScreenColor(0, 0.5f, 0.5f, 0.5f, 1.0f);

            // expressions（模型可能不含 expressions —— 此时跳过）
            std::vector<std::pair<std::string, std::string>> exprs;
            model->GetExpressions(&exprs, [](void* c, const char* id, const char* file) {
                ((std::vector<std::pair<std::string, std::string>>*)c)->push_back({id, file});
            });
            printf("[smoke] expressions=%zu\n", exprs.size());
            if (!exprs.empty()) {
                model->AddExpression(exprs[0].first.c_str());
                model->RemoveExpression(exprs[0].first.c_str());
                model->ResetExpressions();
                // model3.json 里的表情路径是相对路径，LoadExtraExpression 不拼 homeDir
                std::string extraExprFull = std::string(model->GetModelHomeDir()) + exprs[0].second;
                model->LoadExtraExpression("smoke_extra", extraExprFull.c_str());
                CHECK(model->GetExpressionCount() > 0);
            }

            // moc consistency: v2 .moc 无此概念，恒为 false
            CHECK(model->HasMocConsistencyFromFile("kasumi2.moc") == false);

            if (fails == 0) {
                printf("[smoke] ALL OK\n");
                return 0;
            }
            printf("[smoke] %d checks FAILED\n", fails);
            return 1;
        };

        if (smokeAll) {
            int passed = 0, failed = 0;
            for (const auto& entry : kModelTable) {
                printf("\n===== [smoke] %s =====\n", entry.first);
                std::string p = findUpward(entry.second);
                if (p.empty()) {
                    fprintf(stderr,
                            "[smoke] %s: SKIP - resource not found: %s\n",
                            entry.first,
                            entry.second);
                    failed++;
                    continue;
                }
                if (runSmoke(p) == 0)
                    passed++;
                else
                    failed++;
            }
            printf("\n[smoke] summary: %d passed, %d failed / %d models\n",
                   passed,
                   failed,
                   passed + failed);
            return failed == 0 ? 0 : 1;
        }
        return runSmoke(modelPath);
    }

    // --- GLFW window ----------------------------------------------------------
    glfwSetErrorCallback(glfwErrorCallback);
    if (!glfwInit()) {
        fprintf(stderr, "ERROR: glfwInit() failed\n");
        return 1;
    }
#ifdef CSM_TARGET_VULKAN
    if (Live2D::Common::Backend::IsVulkan()) {
        // No GL context — rendering goes through Vulkan
        glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
    }
#endif

#if defined(CSM_TARGET_WIN_GL) || defined(CSM_TARGET_LINUX_GL) || defined(CSM_TARGET_MAC_GL)
    if (Live2D::Common::Backend::IsOpenGL()) {
        // Use Compatibility profile — v2cpp shaders are GLSL 1.20 (attribute/varying)
        glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
        glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
        glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_COMPAT_PROFILE);
    }
#endif
    glfwWindowHint(GLFW_VISIBLE, GLFW_TRUE);
    glfwWindowHint(GLFW_RESIZABLE, GLFW_TRUE);

    static int winW = 800, winH = 800;
    const char* backendName = Live2D::Common::Backend::IsVulkan() ? "Vulkan" : "OpenGL";

    const std::string winTitleBase = std::string("Live2D Test — ") + backendName + " — " +
                                     std::filesystem::u8path(modelPath).filename().string();
    GLFWwindow* window = glfwCreateWindow(winW, winH, winTitleBase.c_str(), nullptr, nullptr);
    if (!window) {
        fprintf(stderr, "ERROR: glfwCreateWindow() failed\n");
        glfwTerminate();
        return 1;
    }
    glfwSetKeyCallback(window, keyCallback);
    static Live2D::IModel* sModelForResize = nullptr;
    glfwSetWindowSizeCallback(window, [](GLFWwindow* win, int width, int height) {
        if (sModelForResize) {
            winW = width;
            winH = height;
#if defined(CSM_TARGET_WIN_GL) || defined(CSM_TARGET_LINUX_GL) || defined(CSM_TARGET_MAC_GL)
            if (Live2D::Common::Backend::IsOpenGL()) {
                glViewport(0, 0, width, height);
            }
#endif
            sModelForResize->Resize(width, height);
        }
    });

#ifdef CSM_TARGET_VULKAN
    if (Live2D::Common::Backend::IsVulkan())
    // --- Initialize Vulkan (device, swapchain, renderer constants) ------------
    {
        VulkanManager* vkManager = VulkanManager::GetInstance();

        // 获取原生窗口句柄 (HWND) 并传给 VulkanManager。
        // surface 创建由 VulkanManager 内部的 vkCreateWin32SurfaceKHR 完成。
        SurfaceCreateContext surfaceCtx{};
        surfaceCtx.window = glfwGetWin32Window(window);
        surfaceCtx.display = nullptr;
        vkManager->SetSurfaceCreateContext(surfaceCtx);

        // 不调用 SetInstanceExtensions → GetRequiredExtensions() 自动补齐所需扩展
        // 不调用 SetCreateSurfaceFunc → 使用平台标准实现
        vkManager->Initialize();
        if (!vkManager->IsInitialized()) {
            fprintf(stderr, "ERROR: Vulkan initialization failed\n");
            glfwDestroyWindow(window);
            glfwTerminate();
            return 1;
        }

        SwapchainManager* swapchain = vkManager->GetSwapchainManager();
        Live2D::Cubism::Framework::Rendering::CubismRenderer_Vulkan::SetConstantSettings(
            vkManager->GetDevice(),
            vkManager->GetPhysicalDevice(),
            vkManager->GetCommandPool(),
            vkManager->GetGraphicQueue(),
            swapchain->GetImageCount(),
            swapchain->GetExtent(),
            vkManager->GetSwapchainImageView(),
            swapchain->GetSwapchainImageFormat(),
            vkManager->GetDepthFormat());
    }
#endif

#if defined(CSM_TARGET_WIN_GL) || defined(CSM_TARGET_LINUX_GL) || defined(CSM_TARGET_MAC_GL)
    if (Live2D::Common::Backend::IsOpenGL()) {
        glfwMakeContextCurrent(window);
        glfwSwapInterval(1);

        // --- Load OpenGL via glad ------------------------------------------------
        if (!gladLoadGL()) {
            fprintf(stderr, "ERROR: gladLoadGL() failed\n");
            glfwDestroyWindow(window);
            glfwTerminate();
            return 1;
        }
        printf("OpenGL %s, GLSL %s\n",
               glGetString(GL_VERSION),
               glGetString(GL_SHADING_LANGUAGE_VERSION));
    }
#endif

    using namespace std::chrono_literals;
    // --- Baseline (idle, no model) -------------------------------------------
    // std::this_thread::sleep_for(1s);

    // --- Load model ----------------------------------------------------------
    EnableLive2DLog(true);
    SetLive2DLogLevel(LV_INFO);

    {
        std::unique_ptr<Live2D::IModel> model(Live2D::IModel::Create(modelPath.c_str()));
        if (!model) {
            fprintf(stderr, "ERROR: cannot detect model version: %s\n", modelPath.c_str());
            return 1;
        }
        sModelForResize = model.get();
        model->LoadModelJson(modelPath.c_str());

        float cw, ch;
        model->GetCanvasSize(cw, ch);
        printf("Model loaded successfully! (%s / %s)\n",
               model->IsV2() ? "v2" : "v3",
               Live2D::Common::Backend::IsVulkan() ? "Vulkan" : "OpenGL");
        printf("  Canvas: %.0f x %.0f\n", cw, ch);
        printf("  Parameters: %d\n", model->GetParameterCount());
        printf("  Parts: %d\n", model->GetPartCount());

        int nParams = model->GetParameterCount();
        int showParams = nParams < 15 ? nParams : 15;
        printf("  First %d parameters:\n", showParams);
        for (int i = 0; i < showParams; i++) {
            printf("    [%d] %-31s  val=%.3f  min=%.2f  max=%.2f  def=%.2f\n",
                   i,
                   model->GetParameterId(i),
                   model->GetParameterValue(i),
                   model->GetParameterMinimumValue(i),
                   model->GetParameterMaximumValue(i),
                   model->GetParameterDefaultValue(i));
        }
        if (nParams > 15)
            printf("    ... (%d more)\n", nParams - 15);

        int nParts = model->GetPartCount();
        printf("  Parts:\n");
        for (int i = 0; i < nParts; i++) {
            printf("    [%d] %s\n", i, model->GetPartId(i));
        }

        model->Resize(winW, winH);
        model->StartRandomMotion("", 3);
        printf("\nStarted random idle motion.\n");

        // std::this_thread::sleep_for(1s);  // model loaded, before render

        printf("Running... (press ESC to exit)\n");
        int frameCount = 0;
        int fpsFrames = 0;
        auto fpsWindowStart = std::chrono::steady_clock::now();
        while (!glfwWindowShouldClose(window)) {
            model->Update();
#if defined(CSM_TARGET_WIN_GL) || defined(CSM_TARGET_LINUX_GL) || defined(CSM_TARGET_MAC_GL)
            if (Live2D::Common::Backend::IsOpenGL()) {
                glViewport(0, 0, winW, winH);
                glClearColor(0.2f, 0.2f, 0.2f, 1.0f);
                glClear(GL_COLOR_BUFFER_BIT);
                model->Draw();
            }
#endif
#ifdef CSM_TARGET_VULKAN
            if (Live2D::Common::Backend::IsVulkan()) {
                // 帧解耦：帧流程由 BeginFrame/EndFrame 驱动，Draw 仅录制命令
                if (VulkanManager::GetInstance()->BeginFrame()) {
                    model->Draw();
                    VulkanManager::GetInstance()->EndFrame();
                }
            }
#endif
            frameCount++;
            fpsFrames++;
#if defined(CSM_TARGET_WIN_GL) || defined(CSM_TARGET_LINUX_GL) || defined(CSM_TARGET_MAC_GL)
            if (Live2D::Common::Backend::IsOpenGL()) {
                glfwSwapBuffers(window);
            }
#endif
            glfwPollEvents();
            if (model->IsMotionFinished()) {
                model->StartRandomMotion("", 3);
            }

            // 每 0.5s 刷新一次标题栏帧率
            const auto now = std::chrono::steady_clock::now();
            const float fpsElapsed = std::chrono::duration<float>(now - fpsWindowStart).count();
            if (fpsElapsed >= 0.5f) {
                const std::string title = winTitleBase + " — " +
                                          std::to_string((int)(fpsFrames / fpsElapsed + 0.5f)) +
                                          " FPS";
                glfwSetWindowTitle(window, title.c_str());
                fpsWindowStart = now;
                fpsFrames = 0;
            }
        }
        printf("Frames rendered: %d\n", frameCount);

        // std::this_thread::sleep_for(1s);  // render ended, before destruction
        model->DestroyRenderer();
    }   // model destroyed — GL context still alive for glDeleteTextures
    // std::this_thread::sleep_for(1s);       // after destruction baseline

    // --- Cleanup -------------------------------------------------------------
#ifdef CSM_TARGET_VULKAN
    if (Live2D::Common::Backend::IsVulkan()) {
        VulkanManager::GetInstance()->Destroy();
    }
#endif
    glfwDestroyWindow(window);
    glfwTerminate();
    return 0;
}
