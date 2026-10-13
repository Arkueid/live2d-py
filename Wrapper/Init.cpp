#ifndef CSM_TARGET_VULKAN
#include <GL/glew.h>
#endif

#include <Init.hpp>
#include <Log.hpp>
#ifdef CSM_TARGET_VULKAN
#include <vulkan/vulkan.h>
#include <VulkanManager.hpp>
#include <Rendering/Vulkan/CubismRenderer_Vulkan.hpp>
#include <vector>
#else
#include <Rendering/OpenGL/CubismShader_OpenGLES2.hpp>
#endif
#include <cstdlib>

#ifdef _WIN32
#include <Windows.h>
#endif

#ifdef DEBUG_ENABLE_CALLSTACK
#include <Debug.hpp>
using namespace Live2D::Common::Debug;
#endif

#include "PyModel.hpp"
using namespace Live2D;
using namespace Live2D::Common::Log;

#ifdef DEBUG_ENABLE_CALLSTACK
#ifndef CSM_TARGET_VULKAN
static void GLAPIENTRY glDebugCallback(GLenum source, GLenum type, GLuint id, GLenum severity,
                                       GLsizei length, const GLchar* message,
                                       const void* userParam) {
    // 过滤掉通知级别的消息，只关注错误和警告
    if (severity == GL_DEBUG_SEVERITY_NOTIFICATION)
        return;

    LOGE("[GL] %s type = 0x%x, severity = 0x%x, message = %s\n",
         (type == GL_DEBUG_TYPE_ERROR ? "** GL ERROR **" : ""),
         type,
         severity,
         message);

    // 让程序崩溃，便于调试器捕获
    if (severity == GL_DEBUG_SEVERITY_HIGH) {
        char* arr = nullptr;
        arr[10] = 10;
    }
}
#endif
#endif


static PyObject* live2d_init_internal(PyObject* self, PyObject* args) {
    const char* path;
    if (PyArg_ParseTuple(args, "s", &path) < 0) {
        PyErr_SetString(PyExc_TypeError, "Invalid params (str)");
        return NULL;
    }

    Live2D::Init(path);
    Py_RETURN_NONE;
}

static PyObject* live2d_dispose() {
    Live2D::Dispose();
    Py_RETURN_NONE;
}


static PyObject* live2d_glInit() {
#ifndef CSM_TARGET_VULKAN
    static bool sGLInitialized = false;
    if (sGLInitialized) {
        LOGE("live2d.glInit double called");
        Py_RETURN_NONE;
    }

    if (!gladLoadGL()) {
        LOGE("Can't initilize glad.");
    }
    sGLInitialized = true;

#ifdef DEBUG_ENABLE_CALLSTACK
    glEnable(GL_DEBUG_OUTPUT);
    glEnable(GL_DEBUG_OUTPUT_SYNCHRONOUS);
    glDebugMessageCallback(glDebugCallback, NULL);

    // 先全部关闭
    glDebugMessageControl(GL_DONT_CARE, GL_DONT_CARE, GL_DONT_CARE, 0, NULL, GL_FALSE);

    // 只打开 HIGH 严重级别（通常是真正的错误）
    glDebugMessageControl(GL_DONT_CARE, GL_DONT_CARE, GL_DEBUG_SEVERITY_HIGH, 0, NULL, GL_TRUE);
#endif
#else
    LOGE("live2d.glInit Vulkan build not supported");
    abort();
#endif   // CSM_TARGET_VULKAN
    Py_RETURN_NONE;
}

static PyObject* live2d_glRelease() {
#ifndef CSM_TARGET_VULKAN
    Csm::Rendering::CubismRenderer::StaticRelease();
#else
    LOGE("live2d.glRelease Vulkan build not supported");
    abort();
#endif   // CSM_TARGET_VULKAN
    Py_RETURN_NONE;
}

static PyObject* live2d_clear_buffer(PyObject* self, PyObject* args) {
#ifndef CSM_TARGET_VULKAN
    // 默认为黑色
    float r = 0.0, g = 0.0, b = 0.0, a = 0.0;

    // 解析传入的参数，允许指定颜色
    if (!PyArg_ParseTuple(args, "|ffff", &r, &g, &b, &a)) {
        return NULL;
    }

    glClearColor(r, g, b, a);
    glClear(GL_COLOR_BUFFER_BIT);
    glClearDepth(1.0);
#else
    LOGE("live2d.clearBuffer Vulkan build not supported");
    abort();
#endif   // CSM_TARGET_VULKAN
    Py_RETURN_NONE;
}

// ---- vkInit / vkDestroy ---------------------------------------------------
// Python 侧始终可见；非 Vulkan 构建时 LOGE + abort，避免误调用。
// Vulkan 构建时 surface 创建由 VulkanManager 内部的平台标准实现
// (vkCreate*Surface) 完成，pyd 不依赖 glfw。
// 实例扩展 (VK_KHR_surface / VK_KHR_*_surface / debug_utils) 全部由
// VulkanManager::GetRequiredExtensions() 在 native 层确定，Python 无需传入。
// vkInit(window_ptr: int, display_ptr: int = 0)
// - window_ptr : 原生窗口句柄 (Windows: HWND)
// - display_ptr: 显示连接 (X11/Wayland 使用，Windows 传 0)
// shader 经 FileUtils 的 LoadFileAsBytes 加载，并被重定向到 SHADER_DIR。
static PyObject* live2d_vk_init(PyObject* /*self*/, PyObject* args) {
#ifdef CSM_TARGET_VULKAN
    unsigned long long windowAddr = 0;
    unsigned long long displayAddr = 0;
    if (!PyArg_ParseTuple(args, "K|K", &windowAddr, &displayAddr)) {
        return NULL;
    }

    SurfaceCreateContext surfaceCtx{};
    surfaceCtx.window = reinterpret_cast<void*>(static_cast<uintptr_t>(windowAddr));
    surfaceCtx.display = reinterpret_cast<void*>(static_cast<uintptr_t>(displayAddr));

    VulkanManager* vkManager = VulkanManager::GetInstance();
    vkManager->SetSurfaceCreateContext(surfaceCtx);
    // 不调用 SetInstanceExtensions → GetRequiredExtensions() 自动补齐所需扩展
    // 不调用 SetCreateSurfaceFunc → 使用 VulkanManager 内部的平台标准实现
    vkManager->Initialize();
    if (!vkManager->IsInitialized()) {
        PyErr_SetString(PyExc_RuntimeError, "Vulkan initialization failed");
        return NULL;
    }

    // 设置 Framework Vulkan 渲染器的设备常量 (CreateRenderer 前必须)
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
#else
    LOGE("vkInit called but Vulkan backend is not enabled in this build");
    std::abort();
    // unreachable
#endif
    Py_RETURN_NONE;
}

static PyObject* live2d_vk_destroy(PyObject* /*self*/, PyObject* /*args*/) {
#ifdef CSM_TARGET_VULKAN
    VulkanManager::GetInstance()->Destroy();
#else
    LOGE("vkDestroy called but Vulkan backend is not enabled in this build");
    std::abort();
#endif
    Py_RETURN_NONE;
}

static PyObject* live2d_enable_log(PyObject* self, PyObject* args) {
    bool enable;
    if (!PyArg_ParseTuple(args, "b", &enable)) {
        PyErr_SetString(PyExc_TypeError, "invalid param");
        return NULL;
    }

    EnableLive2DLog(enable);

    Py_RETURN_NONE;
}

static PyObject* live2d_is_log_enabled(PyObject* self, PyObject* args) {
    if (IsLive2DLogEnabled())
        Py_RETURN_TRUE;
    Py_RETURN_FALSE;
}

static PyObject* live2d_set_log_level(PyObject* self, PyObject* args) {
    int level;
    if (!PyArg_ParseTuple(args, "i", &level)) {
        PyErr_SetString(PyExc_TypeError, "invalid param");
        return NULL;
    }

    SetLive2DLogLevel(level);

    if (IsLive2DLogEnabled()) {
        switch (GetLive2DLogLevel()) {
            case LV_DEBUG:
                LOGD("[Log] Level=DEBUG");
                break;
            case LV_INFO:
                LOGI("[Log] Level=INFO");
                break;
            case LV_WARN:
                LOGW("[Log] Level=WARN");
                break;
            case LV_ERROR:
                LOGE("[Log] Level=Error");
                break;
            default:
                break;
        }
    }

    Py_RETURN_NONE;
}

static PyObject* live2d_get_log_level(PyObject* self, PyObject* args) {
    return Py_BuildValue("i", GetLive2DLogLevel());
}

// 定义live2d模块的方法
static PyMethodDef live2d_methods[] = {
    {"init_internal", (PyCFunction)live2d_init_internal, METH_VARARGS, ""},
    {"dispose", (PyCFunction)live2d_dispose, METH_VARARGS, ""},
    {"glInit", (PyCFunction)live2d_glInit, METH_VARARGS, ""},
    {"glRelease", (PyCFunction)live2d_glRelease, METH_VARARGS, ""},
    {"clearBuffer", (PyCFunction)live2d_clear_buffer, METH_VARARGS, ""},
    {"vkInit", (PyCFunction)live2d_vk_init, METH_VARARGS, ""},
    {"vkDestroy", (PyCFunction)live2d_vk_destroy, METH_VARARGS, ""},
    {"enableLog", (PyCFunction)live2d_enable_log, METH_VARARGS, ""},
    {"isLogEnabled", (PyCFunction)live2d_is_log_enabled, METH_VARARGS, ""},
    {"setLogLevel", (PyCFunction)live2d_set_log_level, METH_VARARGS, ""},
    {"getLogLevel", (PyCFunction)live2d_get_log_level, METH_VARARGS, ""},
    {NULL, NULL, 0, NULL}};

// 定义live2d模块
static PyModuleDef liv2d_module = {
    PyModuleDef_HEAD_INIT, "live2d", "Module that creates live2d objects", -1, live2d_methods};

// 模块初始化函数的实现
PyMODINIT_FUNC PyInit__live2d(void) {
#ifdef DEBUG_ENABLE_CALLSTACK
    InstallCrashHandler();
#endif
    PyObject* m = PyModule_Create(&liv2d_module);
    if (!m) {
        return NULL;
    }

    if (PyModule_AddObject(m, "Model", PyType_FromSpec(&PyModel_Spec)) < 0) {
        Py_DECREF(m);
        return NULL;
    }

#ifdef _WIN32
    SetConsoleOutputCP(65001);
#endif
    return m;
}
