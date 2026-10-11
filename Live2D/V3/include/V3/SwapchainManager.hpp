/**
 * Copyright(c) Live2D Inc. All rights reserved.
 *
 * Use of this source code is governed by the Live2D Open Software license
 * that can be found at https://www.live2d.com/eula/live2d-open-software-license-agreement_en.html.
 */

#pragma once
#include <vulkan/vulkan.h>
#include <Type/CubismBasicType.hpp>
#include <Type/csmVector.hpp>
#include "LAppPal.hpp"


/**
 * @brief   スワップチェーンを扱うクラス
 */
class SwapchainManager
{
public:
    struct SwapchainSupportDetails
    {
        VkSurfaceCapabilitiesKHR capabilities;         //基本的な機能
        Csm::csmVector<VkSurfaceFormatKHR> formats;    //利用可能なフォーマット
        Csm::csmVector<VkPresentModeKHR> presentModes; //利用可能な表示モード
    };

    /**
     * @brief   コンストラクタ
     * @param[in]   _window             ->  ウィンドウ
     * @param[in]   physicalDevice      ->  物理デバイス
     * @param[in]   device              ->  デバイス
     * @param[in]   surface             ->  サーフェス
     * @param[in]   graphicsFamily      ->  描画コマンドに使うキューファミリ
     * @param[in]   presentFamily       ->  表示コマンドに使うキューファミリ
     */
    SwapchainManager(void* _window, VkPhysicalDevice physicalDevice, VkDevice device, VkSurfaceKHR surface, Csm::csmUint32 graphicsFamily, Csm::csmUint32 presentFamily);

    /**
     * @brief    物理デバイスのサポートを確認する
     *
     * @param[in]   physicalDevice      ->  物理デバイス
     * @param[in]   surface             ->  スワップチェーンサーフェス
     * @return  物理デバイスのサポート情報
     */
    static SwapchainSupportDetails QuerySwapchainSupport(VkPhysicalDevice& physicalDevice, VkSurfaceKHR surface);

    /**
     * @brief    サーフェスフォーマットを選択する
     * @param[in]   availableFormats    ->  使えるサーフェスフォーマット
     * @return  選択するサーフェスフォーマット
     */
    VkSurfaceFormatKHR ChooseSwapSurfaceFormat(const Csm::csmVector<VkSurfaceFormatKHR>& availableFormats);

    /**
     * @brief    表示モードを選択する
     * @param[in]   availableFormats    ->  使えるフォーマット
     */
    VkPresentModeKHR ChooseSwapPresentMode(const Csm::csmVector<VkPresentModeKHR>& availablePresentModes);

    /**
     * @brief   スワップチェーンイメージの解像度を選択する
     *
     * @param[in]   window              ->  ウィンドウ
     * @param[in]   capabilities        ->  サーフェスの機能を保持する構造体
     * @return  スワップチェーンイメージの解像度
     */
    VkExtent2D ChooseSwapExtent(void* window, const VkSurfaceCapabilitiesKHR& capabilities);

    /**
     * @brief   スワップチェーンを作成する
     *
     * @param[in]   _window             ->  ウィンドウ
     * @param[in]   physicalDevice      ->  物理デバイス
     * @param[in]   device              ->  デバイス
     * @param[in]   surface             ->  サーフェス
     * @param[in]   graphicsFamily      ->  描画コマンドに使うキューファミリ
     * @param[in]   presentFamily       ->  表示コマンドに使うキューファミリ
     */
    void CreateSwapchain(void* window, VkPhysicalDevice physicalDevice, VkDevice device, VkSurfaceKHR surface, Csm::csmUint32 graphicsFamily, Csm::csmUint32 presentFamily);

    /**
     * @brief    表示コマンドをキューに積む
     *
     * @param[in]   queue          ->  キュー
     * @param[in]   imageIndex     ->  イメージのインデックス
     * @return  成功したか
     */
    VkResult QueuePresent(VkQueue& queue, Csm::csmUint32 imageIndex);

    /**
     * @brief    スワップチェーンのレイアウトの変更
     *
     * @param[in]   device         ->  論理デバイス
     * @param[in]   commandPool    ->  コマンドプール
     * @param[in]   queue          ->  キュー
     */
    void ChangeLayout(VkDevice device, VkCommandPool commandPool, VkQueue queue);

    /**
     * @brief    リソースを破棄＆再作成する
     *
     * @param[in]   device              ->  デバイス
     */
    void Cleanup(VkDevice device);

    /**
     * @brief    extentを取得する
     *
     * @return  extent
     */
    VkExtent2D GetExtent() { return extent; }

    /**
     * @brief    イメージ数を取得する
     *
     * @return  イメージ数
     */
    Csm::csmInt32 GetImageCount() { return imageCount; }

    /**
     * @brief    スワップチェーンイメージビューを取得する
     *
     * @return  スワップチェーンイメージビュー
     */
    Csm::csmVector<VkImageView> GetViews() { return imageViews; }

    /**
     * @brief    スワップチェーンを取得する
     *
     * @return  スワップチェーン
     */
    VkSwapchainKHR GetSwapchain() { return swapchain; }

    /**
     * @brief    スワップチェーンのフォーマットを取得する
     *
     * @return  フォーマット
     */
    VkFormat GetSwapchainImageFormat() { return swapchainFormat; }

    /**
     * @brief    スワップチェーンのイメージを取得する
     *
     * @return  イメージ
     */
    Csm::csmVector<VkImage> GetImages() { return images; }

    /**
     * @brief    スワップチェーンのイメージを取得する
     *
     * @return  イメージ
     */
    Csm::csmVector<VkImageView> GetImageViews() { return imageViews; }

private:
    VkExtent2D extent = {0, 0};                  ///< イメージの解像度
    Csm::csmUint32 imageCount = 0;                             ///< イメージ数
    Csm::csmVector<VkImage> images;                            ///< スワップチェーンのイメージ
    Csm::csmVector<VkImageView> imageViews;                    ///< スワップチェーンのイメージビュー
    VkSwapchainKHR swapchain;                                  ///< スワップチェーン
    const VkFormat swapchainFormat = VK_FORMAT_R8G8B8A8_UNORM; ///< スワップチェーンのフォーマット
};
