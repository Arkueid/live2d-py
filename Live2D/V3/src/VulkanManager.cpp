#include "VulkanManager.hpp"
#include <iostream>
#include <set>

#include "Log.hpp"
#include "Rendering/Vulkan/CubismRenderer_Vulkan.hpp"
using namespace Csm;

/**
* @brief   デバッグ用コールバック
*/
static VKAPI_ATTR VkBool32 VKAPI_CALL debugCallback(VkDebugUtilsMessageSeverityFlagBitsEXT messageSeverity,
                                                    VkDebugUtilsMessageTypeFlagsEXT messageType,
                                                    const VkDebugUtilsMessengerCallbackDataEXT* pCallbackData,
                                                    void* pUserData)
{
    // LappPal::PrintLogは最大文字数を増やせば使える
    // LAppPal::PrintLog("validation layer: %s\n", pCallbackData->pMessage);
    std::cout << "validation layer: " << pCallbackData->pMessage << std::endl;
    return VK_FALSE;
}

/**
* @brief   デバッグ用拡張関数を使えるようにする
*/
VkResult CreateDebugUtilsMessengerEXT(VkInstance instance, const VkDebugUtilsMessengerCreateInfoEXT* pCreateInfo,
                                      const VkAllocationCallbacks* pAllocator,
                                      VkDebugUtilsMessengerEXT* pDebugMessenger)
{
    auto func = (PFN_vkCreateDebugUtilsMessengerEXT)vkGetInstanceProcAddr(instance, "vkCreateDebugUtilsMessengerEXT");
    if (func != nullptr)
    {
        return func(instance, pCreateInfo, pAllocator, pDebugMessenger);
    }
    else
    {
        return VK_ERROR_EXTENSION_NOT_PRESENT;
    }
}

/**
* @brief   デバッグメッセンジャーの破棄
*/
void DestroyDebugUtilsMessengerEXT(VkInstance instance, VkDebugUtilsMessengerEXT debugMessenger,
                                   const VkAllocationCallbacks* pAllocator)
{
    auto func = (PFN_vkDestroyDebugUtilsMessengerEXT)vkGetInstanceProcAddr(instance, "vkDestroyDebugUtilsMessengerEXT");
    if (func != nullptr)
    {
        func(instance, debugMessenger, pAllocator);
    }
}

static VulkanManager* sInstance = nullptr;

VulkanManager* VulkanManager::GetInstance() {
    if (sInstance == nullptr) {
        sInstance = new VulkanManager(nullptr);
    }
    return sInstance;
}

VulkanManager::VulkanManager(void* wind):
                                              _instance(VK_NULL_HANDLE)
                                              , _physicalDevice(VK_NULL_HANDLE)
                                              , _device(VK_NULL_HANDLE)
                                              , _swapchainManager()
                                              , _surface()
                                              , _graphicQueue(VK_NULL_HANDLE)
                                              , _presentQueue(VK_NULL_HANDLE)
                                              , _commandPool(VK_NULL_HANDLE)
                                              , _debugMessenger()
{
    _window = wind;
}

VulkanManager::~VulkanManager()
{
    Destroy();
}

bool VulkanManager::CheckValidationLayerSupport()
{
    csmUint32 layerCount;
    vkEnumerateInstanceLayerProperties(&layerCount, nullptr);

    csmVector<VkLayerProperties> availableLayers;
    availableLayers.Resize(layerCount);
    vkEnumerateInstanceLayerProperties(&layerCount, availableLayers.GetPtr());

    for (const char* layerName : validationLayers)
    {
        bool layerFound = false;

        for (csmUint32 i = 0; i < availableLayers.GetSize(); i++)
        {
            if (strcmp(layerName, availableLayers[i].layerName) == 0)
            {
                layerFound = true;
                break;
            }
        }

        if (!layerFound)
        {
            return false;
        }
    }

    return true;
}

void VulkanManager::SetInstanceExtensions(const char** extensions, Csm::csmUint32 count)
{
    _instanceExtensions.Clear();
    for (Csm::csmUint32 i = 0; i < count; i++)
    {
        _instanceExtensions.PushBack(extensions[i]);
    }
}

csmVector<const char*> VulkanManager::GetRequiredExtensions()
{
    csmVector<const char*> extensions;
    for (Csm::csmUint32 i = 0; i < _instanceExtensions.GetSize(); i++)
    {
        extensions.PushBack(_instanceExtensions[i]);
    }

    if (_enableValidationLayers)
    {
        extensions.PushBack(VK_EXT_DEBUG_UTILS_EXTENSION_NAME);
    }

    return extensions;
}

void VulkanManager::populateDebugMessengerCreateInfo(VkDebugUtilsMessengerCreateInfoEXT& createInfo)
{
    createInfo = {};
    createInfo.sType = VK_STRUCTURE_TYPE_DEBUG_UTILS_MESSENGER_CREATE_INFO_EXT;
    createInfo.messageSeverity = VK_DEBUG_UTILS_MESSAGE_SEVERITY_VERBOSE_BIT_EXT |
            VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT;
    createInfo.messageType = VK_DEBUG_UTILS_MESSAGE_TYPE_GENERAL_BIT_EXT |
            VK_DEBUG_UTILS_MESSAGE_TYPE_VALIDATION_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_TYPE_PERFORMANCE_BIT_EXT;
    createInfo.pfnUserCallback = debugCallback;
}

void VulkanManager::CreateInstance()
{
    //検証レイヤーが有効のときに使えるか確認
    if (_enableValidationLayers && !CheckValidationLayerSupport())
    {
        //検証レイヤーをサポートしていないのでログを出して使えないようにする
        LOGE("validation layers requested, but not available!");
        _enableValidationLayers = false;
    }

    VkApplicationInfo appInfo{};
    appInfo.sType = VK_STRUCTURE_TYPE_APPLICATION_INFO;
    appInfo.pApplicationName = "SAMPLE";
    appInfo.pEngineName = "SAMPLE";
    appInfo.engineVersion = VK_MAKE_VERSION(1, 0, 0);
    appInfo.apiVersion = VK_API_VERSION_1_3;

    VkInstanceCreateInfo createInfo{};
    createInfo.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
    createInfo.pApplicationInfo = &appInfo;

    csmVector<const char*> extensions = GetRequiredExtensions();
    createInfo.enabledExtensionCount = static_cast<csmUint32>(extensions.GetSize());
    createInfo.ppEnabledExtensionNames = extensions.GetPtr();

    VkDebugUtilsMessengerCreateInfoEXT debugCreateInfo{};
    if (_enableValidationLayers)
    {
        createInfo.enabledLayerCount = static_cast<csmUint32>(sizeof(validationLayers) / sizeof(validationLayers[0]));
        createInfo.ppEnabledLayerNames = validationLayers;
        // vkDestroyInstanceでデバッグコールが呼ばれるために次の行が必要
        populateDebugMessengerCreateInfo(debugCreateInfo);
        createInfo.pNext = (VkDebugUtilsMessengerCreateInfoEXT*)&debugCreateInfo;
    }
    else
    {
        createInfo.enabledLayerCount = 0;
    }
    if (vkCreateInstance(&createInfo, nullptr, &_instance) != VK_SUCCESS)
    {
        LOGE("failed to create _instance!");
    }
}

void VulkanManager::SetupDebugMessenger()
{
    if (!_enableValidationLayers) return;

    VkDebugUtilsMessengerCreateInfoEXT createInfo;
    populateDebugMessengerCreateInfo(createInfo);

    if (CreateDebugUtilsMessengerEXT(_instance, &createInfo, nullptr, &_debugMessenger) != VK_SUCCESS)
    {
        LOGE("failed to set up debug messenger!");
    }
}

bool VulkanManager::CheckDeviceExtensionSupport(VkPhysicalDevice physicalDevice)
{
    csmUint32 extensionCount;
    vkEnumerateDeviceExtensionProperties(physicalDevice, nullptr, &extensionCount, nullptr);

    csmVector<VkExtensionProperties> availableExtensions;
    availableExtensions.Resize(extensionCount);
    vkEnumerateDeviceExtensionProperties(physicalDevice, nullptr, &extensionCount, availableExtensions.GetPtr());

    for (csmUint32 i = 0; i < sizeof(deviceExtensions) / sizeof(deviceExtensions[0]); i++)
    {
        bool found = false;
        for (csmUint32 j = 0; j < availableExtensions.GetSize(); j++)
        {
            if (strcmp(deviceExtensions[i], availableExtensions[j].extensionName) == 0)
            {
                found = true;
                break;
            }
        }
        if (found == false)
        {
            return false;
        }
    }
    return true;
}

void VulkanManager::CreateSurface()
{
    if (_createSurfaceFunc != nullptr)
    {
        _surface = _createSurfaceFunc(_instance, _window);
        if (_surface == VK_NULL_HANDLE)
        {
            LOGE("failed to create window surface!");
        }
    }
    else
    {
        LOGE("no surface creation hook set (SetCreateSurfaceFunc)");
    }
}

void VulkanManager::FindQueueFamilies(VkPhysicalDevice device)
{
    csmUint32 queueFamilyCount = 0;
    vkGetPhysicalDeviceQueueFamilyProperties(device, &queueFamilyCount, nullptr);
    csmVector<VkQueueFamilyProperties> queueFamilies;
    queueFamilies.Resize(queueFamilyCount);
    vkGetPhysicalDeviceQueueFamilyProperties(device, &queueFamilyCount, queueFamilies.GetPtr());

    for (csmUint32 i = 0; i < queueFamilies.GetSize(); i++)
    {
        if (queueFamilies[i].queueFlags & VK_QUEUE_GRAPHICS_BIT)
        {
            indices.graphicsFamily = i;
        }

        VkBool32 presentSupport = false;
        vkGetPhysicalDeviceSurfaceSupportKHR(device, i, _surface, &presentSupport);
        if (presentSupport)
        {
            indices.presentFamily = i;
        }

        if (indices.isComplete())
        {
            break;
        }
    }
}

bool VulkanManager::IsDeviceSuitable(VkPhysicalDevice device)
{
    FindQueueFamilies(device);
    bool extensionsSupported = CheckDeviceExtensionSupport(device);
    bool swapChainAdequate = false;
    //デバイスの拡張機能がサポートされていたら、スワップチェインをサポートしているか確認する
    if (extensionsSupported)
    {
        SwapchainManager::SwapchainSupportDetails swapchainSupport = SwapchainManager::QuerySwapchainSupport(
            device, _surface);
        swapChainAdequate = swapchainSupport.formats.GetSize() != 0 && swapchainSupport.presentModes.GetSize() != 0;
    }

    VkPhysicalDeviceFeatures supportedFeatures;
    vkGetPhysicalDeviceFeatures(device, &supportedFeatures);

    return indices.isComplete() && swapChainAdequate && supportedFeatures.samplerAnisotropy;
}

void VulkanManager::PickPhysicalDevice()
{
    csmUint32 deviceCount = 0;
    vkEnumeratePhysicalDevices(_instance, &deviceCount, nullptr);

    if (deviceCount == 0)
    {
        LOGE("failed to find GPUs with Vulkan support!");
    }

    csmVector<VkPhysicalDevice> devices;
    devices.Resize(deviceCount);
    vkEnumeratePhysicalDevices(_instance, &deviceCount, devices.GetPtr());

    // 一番最初に取得した物理デバイスを使用する
    for (csmUint32 i = 0; i < devices.GetSize(); i++)
    {
        if (IsDeviceSuitable(devices[i]))
        {
            _physicalDevice = devices[i];
            break;
        }
    }

    if (_physicalDevice == VK_NULL_HANDLE)
    {
        LOGE("failed to find a suitable GPU!");
    }
}

void VulkanManager::CreateLogicalDevice()
{
    FindQueueFamilies(_physicalDevice);

    csmVector<csmInt32> uniqueQueueFamilies;
    if (indices.graphicsFamily == indices.presentFamily)
    {
        uniqueQueueFamilies.PushBack(indices.graphicsFamily);
    }
    else
    {
        uniqueQueueFamilies.PushBack(indices.graphicsFamily);
        uniqueQueueFamilies.PushBack(indices.presentFamily);
    }

    float queuePriority = 1.0f;
    csmVector<VkDeviceQueueCreateInfo> queueCreateInfos;
    for (csmUint32 i = 0; i < uniqueQueueFamilies.GetSize(); i++)
    {
        VkDeviceQueueCreateInfo queueCreateInfo{};
        queueCreateInfo.sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
        queueCreateInfo.queueFamilyIndex = uniqueQueueFamilies[i];
        queueCreateInfo.queueCount = 1;
        queueCreateInfo.pQueuePriorities = &queuePriority;
        queueCreateInfos.PushBack(queueCreateInfo);
    }

    VkDeviceCreateInfo createInfo{};
    createInfo.sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;
    createInfo.queueCreateInfoCount = queueCreateInfos.GetSize();
    createInfo.pQueueCreateInfos = queueCreateInfos.GetPtr();
    createInfo.enabledExtensionCount = sizeof(deviceExtensions) / sizeof(deviceExtensions[0]);
    createInfo.ppEnabledExtensionNames = deviceExtensions;

    if (_enableValidationLayers)
    {
        createInfo.enabledLayerCount = sizeof(validationLayers) / sizeof(validationLayers[0]);
        createInfo.ppEnabledLayerNames = validationLayers;
    }
    else
    {
        createInfo.enabledLayerCount = 0;
    }

    VkPhysicalDeviceExtendedDynamicStateFeaturesEXT dynamicStateF{};
    dynamicStateF.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_EXTENDED_DYNAMIC_STATE_FEATURES_EXT;
    dynamicStateF.extendedDynamicState = VK_TRUE;
    dynamicStateF.pNext = nullptr;

    VkPhysicalDeviceVulkan13Features deviceVulkan13Features{};
    deviceVulkan13Features.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_VULKAN_1_3_FEATURES;
    deviceVulkan13Features.synchronization2 = VK_TRUE;
    deviceVulkan13Features.dynamicRendering = VK_TRUE;
    deviceVulkan13Features.pNext = &dynamicStateF;

    VkPhysicalDeviceFeatures2 deviceFeatures2{};
    deviceFeatures2.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_FEATURES_2;
    deviceFeatures2.pNext = &deviceVulkan13Features;
    vkGetPhysicalDeviceFeatures2(_physicalDevice, &deviceFeatures2);
    createInfo.pNext = &deviceFeatures2;

    if (vkCreateDevice(_physicalDevice, &createInfo, nullptr, &_device) != VK_SUCCESS)
    {
        LOGE("failed to create logical _device!");
    }

    // キューハンドルを取得
    vkGetDeviceQueue(_device, indices.graphicsFamily, 0, &_graphicQueue);
    vkGetDeviceQueue(_device, indices.presentFamily, 0, &_presentQueue);
}

void VulkanManager::ChooseSupportedDepthFormat()
{
    VkFormat depthFormats[5] = {
        VK_FORMAT_D32_SFLOAT_S8_UINT,
        VK_FORMAT_D32_SFLOAT,
        VK_FORMAT_D24_UNORM_S8_UINT,
        VK_FORMAT_D16_UNORM_S8_UINT,
        VK_FORMAT_D16_UNORM
    };

    for (csmInt32 i = 0; i < sizeof(depthFormats) / sizeof(depthFormats[0]); i++)
    {
        VkFormatProperties formatProps;
        vkGetPhysicalDeviceFormatProperties(_physicalDevice, depthFormats[i], &formatProps);

        if (formatProps.optimalTilingFeatures & VK_FORMAT_FEATURE_DEPTH_STENCIL_ATTACHMENT_BIT)
        {
            _depthFormat = depthFormats[i];
            return;
        }
    }
    CubismLogError("can't find depth format!");
    _depthFormat = depthFormats[0];
}

void VulkanManager::CreateCommandPool()
{
    VkCommandPoolCreateInfo poolInfo{};
    poolInfo.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
    poolInfo.flags = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT;
    poolInfo.queueFamilyIndex = indices.graphicsFamily;

    if (vkCreateCommandPool(_device, &poolInfo, nullptr, &_commandPool) != VK_SUCCESS)
    {
        LOGE("failed to create graphics command pool!");
    }
}

void VulkanManager::CreateSyncObjects()
{
    VkSemaphoreCreateInfo semaphoreInfo{};
    semaphoreInfo.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;
    vkCreateSemaphore(_device, &semaphoreInfo, nullptr, &_imageAvailableSemaphore);
}

void VulkanManager::Initialize()
{
    CreateInstance();
    SetupDebugMessenger();
    CreateSurface();
    PickPhysicalDevice();
    CreateLogicalDevice();
    ChooseSupportedDepthFormat();
    _swapchainManager = new SwapchainManager(_window, _physicalDevice, _device, _surface, indices.graphicsFamily,
                                            indices.presentFamily);
    CreateCommandPool();
    _swapchainManager->ChangeLayout(_device, _commandPool, _graphicQueue);
    CreateSyncObjects();
}

VkCommandBuffer VulkanManager::BeginSingleTimeCommands()
{
    VkCommandBufferAllocateInfo allocInfo{};
    allocInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
    allocInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
    allocInfo.commandPool = _commandPool;
    allocInfo.commandBufferCount = 1;

    VkCommandBuffer commandBuffer;
    vkAllocateCommandBuffers(_device, &allocInfo, &commandBuffer);

    VkCommandBufferBeginInfo beginInfo{};
    beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
    beginInfo.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;

    vkBeginCommandBuffer(commandBuffer, &beginInfo);

    return commandBuffer;
}

void VulkanManager::SubmitCommand(VkCommandBuffer commandBuffer, bool isFirstDraw)
{
    vkEndCommandBuffer(commandBuffer);

    VkSubmitInfo submitInfo{};

    if(isFirstDraw)
    {
        VkSubmitInfo submitInfo{};
        submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
        submitInfo.commandBufferCount = 1;
        submitInfo.pCommandBuffers = &commandBuffer;
        submitInfo.waitSemaphoreCount = 1;
        submitInfo.pWaitSemaphores = &_imageAvailableSemaphore;
        VkPipelineStageFlags waitStages[] = { VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT };
        submitInfo.pWaitDstStageMask = waitStages;
        vkQueueSubmit(_graphicQueue, 1, &submitInfo, VK_NULL_HANDLE);
    }
    else
    {
        VkSubmitInfo submitInfo{};
        submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
        submitInfo.commandBufferCount = 1;
        submitInfo.pCommandBuffers = &commandBuffer;
        VkPipelineStageFlags waitStages[] = { VK_PIPELINE_STAGE_VERTEX_INPUT_BIT  };
        submitInfo.pWaitDstStageMask = waitStages;
        vkQueueSubmit(_graphicQueue, 1, &submitInfo, VK_NULL_HANDLE);
    }

    // コマンドの実行終了まで待機
    vkQueueWaitIdle(_graphicQueue);
    vkFreeCommandBuffers(_device, _commandPool, 1, &commandBuffer);
}

void VulkanManager::UpdateDrawFrame()
{
    VkResult result = vkAcquireNextImageKHR(_device, _swapchainManager->GetSwapchain(), UINT64_MAX,
        _imageAvailableSemaphore, VK_NULL_HANDLE,
        &_imageIndex);
    if (result == VK_ERROR_OUT_OF_DATE_KHR)
    {
        _isSwapchainInvalid = true;
    }
    else if (result != VK_SUCCESS && result != VK_SUBOPTIMAL_KHR)
    {
        LOGE("failed to acquire swap chain image!");
    }
}

void VulkanManager::PostDraw()
{
    VkResult result = _swapchainManager->QueuePresent(_presentQueue, _imageIndex);
    if (result == VK_ERROR_OUT_OF_DATE_KHR || result == VK_SUBOPTIMAL_KHR || _framebufferResized) {
        _framebufferResized = false;
        _isSwapchainInvalid = true;
    }
    else if (result != VK_SUCCESS) {
        LOGE("failed to present swap chain image!");
    }
}

void VulkanManager::RecreateSwapchain()
{
    vkDeviceWaitIdle(_device);
    _swapchainManager->Cleanup(_device);
    _swapchainManager->CreateSwapchain(_window, _physicalDevice, _device, _surface, indices.graphicsFamily,
                                      indices.presentFamily);
    _swapchainManager->ChangeLayout(_device, _commandPool, _graphicQueue);
}

void VulkanManager::Destroy()
{
    if (!IsInitialized())
    {
        return;
    }
    vkDeviceWaitIdle(_device);

    if (_swapchainManager != NULL)
    {
        _swapchainManager->Cleanup(_device);
        delete _swapchainManager;
        _swapchainManager = NULL;
    }

    vkDestroySemaphore(_device, _imageAvailableSemaphore, nullptr);

    if (_enableValidationLayers)
    {
        DestroyDebugUtilsMessengerEXT(_instance, _debugMessenger, nullptr);
    }

    vkDestroyCommandPool(_device, _commandPool, nullptr);
    vkDestroyDevice(_device, nullptr);
    _device = VK_NULL_HANDLE;
    vkDestroySurfaceKHR(_instance, _surface, nullptr);
    vkDestroyInstance(_instance, nullptr);
    _instance = VK_NULL_HANDLE;
}
