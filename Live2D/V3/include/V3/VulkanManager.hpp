/**
 * Copyright(c) Live2D Inc. All rights reserved.
 *
 * Use of this source code is governed by the Live2D Open Software license
 * that can be found at https://www.live2d.com/eula/live2d-open-software-license-agreement_en.html.
 */

#pragma once
#include <vulkan/vulkan.h>
#include "Type/csmMap.hpp"
#include "SwapchainManager.hpp"

 /**
  * @brief   vulkanに必要なリソースを扱うクラス
  */
class VulkanManager
{
public:
    // キューファミリの情報を格納する構造体
    struct QueueFamilyIndices
    {
        // 描画用と表示用のキューファミリは必ずしも一致するとは限らないので分ける
        Csm::csmInt32 graphicsFamily = -1; // 描画コマンドに使用するキューファミリ
        Csm::csmInt32 presentFamily = -1; // 表示に使用するキューファミリ

        // 対応するキューファミリがあるか
        bool isComplete()
        {
            return graphicsFamily != -1 && presentFamily != -1;
        }
    }indices;

    const char* deviceExtensions[2] = {
        VK_KHR_SWAPCHAIN_EXTENSION_NAME,
        VK_EXT_EXTENDED_DYNAMIC_STATE_EXTENSION_NAME
    };

    const char* validationLayers[1] = {
        "VK_LAYER_KHRONOS_validation"
    };

    static VulkanManager* GetInstance();

    /**
     * @brief   コンストラクタ
     * @param[in]   wind             ->  ウィンドウ
     */
    VulkanManager(void* wind);

    /**
     * @brief   デストラクタ
     */
    ~VulkanManager();

    /**
     * @brief   ウィンドウハンドルをセットする（Initialize 前に呼ぶ）
     */
    void SetWindow(void* window) { _window = window; }

    /**
     * @brief   インスタンス拡張を外部から注入する（Initialize 前に呼ぶ）
     *          glfwGetRequiredInstanceExtensions 等の結果を渡す
     */
    void SetInstanceExtensions(const char** extensions, Csm::csmUint32 count);

    /**
     * @brief   サーフェス作成フック
     *          glfwCreateWindowSurface 等、ライブラリ非依存にするため外部から注入する
     */
    typedef VkSurfaceKHR (*CreateSurfaceFunc)(VkInstance instance, void* window);
    void SetCreateSurfaceFunc(CreateSurfaceFunc func) { _createSurfaceFunc = func; }

    /**
     * @brief   初期化済みか（論理デバイスが作成されているか）
     */
    bool IsInitialized() const { return _device != VK_NULL_HANDLE; }

    /**
     * @brief   検証レイヤーのサポートを確認する
     */
    bool CheckValidationLayerSupport();

    /**
     * @brief   必要な拡張機能を取得する
     *
     * @return  必要な拡張の配列
     */
    Csm::csmVector<const char*> GetRequiredExtensions();

    /**
     * @brief   デバッグメッセージを有効にする
     *
     * @param[in]   createInfo             ->  デバッグメッセンジャーオブジェクトの情報
     */
    void populateDebugMessengerCreateInfo(VkDebugUtilsMessengerCreateInfoEXT& createInfo);

    /**
     * @brief   インスタンスを生成する
     */
    void CreateInstance();

    /**
     * @brief   デバッグメッセージを有効にする
     */
    void SetupDebugMessenger();

    /**
     * @brief   デバイスの拡張をチェックする
     *
     * @param[in]   physicalDevice ->  物理デバイス
     */
    bool CheckDeviceExtensionSupport(VkPhysicalDevice physicalDevice);

    /**
     * @brief    サーフェスを作る
     */
    void CreateSurface();

    /**
     * @brief   キューファミリを見つける
     *
     * @param[in]   queueFlags ->  物理デバイス
     */
    void FindQueueFamilies(VkPhysicalDevice device);

    /**
     * @brief   デバイスが使えるか確認する
     *
     * @param[in]   device ->  物理デバイス
     */
    bool IsDeviceSuitable(VkPhysicalDevice device);

    /**
     * @brief   物理デバイスを取得する
     */
    void PickPhysicalDevice();

    /**
     * @brief   論理デバイスを作成する
     */
    void CreateLogicalDevice();

    /**
     * @brief   深度フォーマットを作成する
     */
    void ChooseSupportedDepthFormat();

    /**
     * @brief   コマンドプールを作成する
     */
    void CreateCommandPool();

    /**
     * @brief   同期オブジェクトを作成する
     */
    void CreateSyncObjects();

    /**
     * @brief   初期化する
     */
    void Initialize();

    /**
     * @brief   コマンドの記録を開始する
     */
    VkCommandBuffer BeginSingleTimeCommands();

    /**
     * @brief   コマンドを提出する
     *
     * @param[in]   commandBuffer     ->  コマンドバッファ
     * @param[in]   isDraw            ->  描画コマンドか
     * @param[in]   isFirstDraw       ->  最初の描画コマンドか
     */
    void SubmitCommand(VkCommandBuffer commandBuffer, bool isFirstDraw = false);

    /**
     * @brief   描画する
     */
    void UpdateDrawFrame();

    /**
     * @brief   描画完了の追加処理
     */
    void PostDraw();

    /**
     * @brief   スワップチェーンを再構成する
     */
    void RecreateSwapchain();

    /**
     * @brief   リソースを破棄する
     */
    void Destroy();

    /**
     * @brief   デバイスを取得する
     * @return  デバイス
     */
    VkDevice GetDevice() { return _device; }

    /**
     * @brief   物理デバイスを取得する
     * @return  物理デバイス
     */
    VkPhysicalDevice GetPhysicalDevice() { return _physicalDevice; }

    /**
     * @brief   モデル描画用コマンドバッファを取得する
     * @return  コマンドバッファの配列
     */
    VkQueue GetGraphicQueue() { return _graphicQueue; }

    /**
     * @brief   コマンドプールを取得する
     * @return  コマンドプール
     */
    VkCommandPool GetCommandPool() { return _commandPool; }

    /**
     * @brief   スワップチェーンマネージャーを取得する
     * @return  スワップチェーンマネージャー
     */
    SwapchainManager* GetSwapchainManager() { return _swapchainManager; }

    /**
     * @brief   ウィンドウサイズが変更されたかのフラグを取得する
     * @return  ウィンドウサイズの変更フラグ
     */
    bool GetIsWindowSizeChanged() { return _isSwapchainInvalid; }

    /**
     * @brief   ウィンドウサイズが変更されたかのフラグをセットする
     * @param[in]   flag     ->  セットするフラグ
     */
    void SetIsWindowSizeChanged(bool flag) { _isSwapchainInvalid = flag; }

    /**
     * @brief    フレームバッファのフラグを更新する
     *
     */
    void SetFrameBufferResized(bool flag) { _framebufferResized = flag; }

    /**
     * @brief   深度フォーマットを取得する
     * @return  深度フォーマット
     */
    VkFormat GetDepthFormat() const { return _depthFormat; }

    /**
     * @brief    サーフェスのフォーマットを取得する
     *
     * @return  フォーマット
     */
    VkFormat GetImageFormat() { return _surfaceFormat; }

    /**
     * @brief    スワップチェーンイメージを取得する
     *
     * @return  スワップチェーンイメージ
     */
    VkImage GetSwapchainImage() { return GetSwapchainManager()->GetImages()[_imageIndex]; }

    /**
     * @brief    スワップチェーンイメージを取得する
     *
     * @return  スワップチェーンイメージ
     */
    VkImageView GetSwapchainImageView() { return GetSwapchainManager()->GetImageViews()[_imageIndex]; }

private:
    void* _window;                                        // ウィンドウ
    Csm::csmVector<const char*> _instanceExtensions;      // 外部から注入されたインスタンス拡張
    CreateSurfaceFunc _createSurfaceFunc = nullptr;       // サーフェス作成フック
    VkInstance _instance;                                       // ライブラリを初期化するインスタンス
    VkSurfaceKHR _surface;                                      // ウィンドウシステムに描画ために必要なサーフェス
    VkPhysicalDevice _physicalDevice;                           // 物理デバイス
    VkDevice _device;                                           // 論理デバイス
    VkQueue _graphicQueue;                                      // 描画コマンドを積むキュー
    VkQueue _presentQueue;                                      // 表示コマンドに使用するキュー
    VkCommandPool _commandPool;                                 // コマンドバッファの作成に必要なコマンドプール
    VkSemaphore _imageAvailableSemaphore;                       // セマフォ
    SwapchainManager* _swapchainManager;                        // スワップチェーンの管理を行うスワップチェーンマネージャー
    bool _isSwapchainInvalid = false;                           // ウィンドウサイズが変更されたかのフラグ
    VkDebugUtilsMessengerEXT _debugMessenger;                   // デバッグメッセージを出力するオブジェクト
    Csm::csmUint32 _imageIndex = 0;                             // 現在のイメージインデックス
    VkFormat _depthFormat;                                      // 深度フォーマット
    const VkFormat _surfaceFormat = VK_FORMAT_R8G8B8A8_UNORM;   // イメージフォーマット
    bool _framebufferResized = false;                           // フレームバッファのサイズが変わったか
#ifdef CSM_DEBUG
    bool _enableValidationLayers = true;                        // 検証レイヤーを有効にするか
#else
    bool _enableValidationLayers = false;
#endif // CSM_DEBUG
};
