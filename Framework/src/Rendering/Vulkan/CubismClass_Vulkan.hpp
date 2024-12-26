/**
 * Copyright(c) Live2D Inc. All rights reserved.
 *
 * Use of this source code is governed by the Live2D Open Software license
 * that can be found at https://www.live2d.com/eula/live2d-open-software-license-agreement_en.html.
 */

#pragma once
#include <fstream>
#include <vulkan/vulkan.h>
#include "CubismFramework.hpp"
#include "Type/csmVector.hpp"

namespace Live2D { namespace Cubism { namespace Framework {
/**
 * @brief   バッファを扱うクラス
 */
class CubismBufferVulkan
{
public:
    /**
     * @brief   コンストラクタ
     */
    CubismBufferVulkan();

    /**
     * @brief   物理デバイスのメモリタイプのインデックスを探す
     *
     * @param[in]  physicalDevice -> 物理デバイス
     * @param[in]  typeFilter     -> メモリタイプが存在していたら設定されるビットマスク
     * @param[in]  properties     -> メモリがデバイスにアクセスするときのタイプ
     */
    csmUint32 FindMemoryType(VkPhysicalDevice physicalDevice, csmUint32 typeFilter, VkMemoryPropertyFlags properties);

    /**
     * @brief   バッファを作成する
     *
     * @param[in]  device          -> デバイス
     * @param[in]  physicalDevice  -> 物理デバイス
     * @param[in]  size            -> バッファサイズ
     * @param[in]  usage           -> バッファの使用法を指定するビットマスク
     * @param[in]  properties      -> メモリがデバイスにアクセスする際のタイプ
     */
    void CreateBuffer(VkDevice device, VkPhysicalDevice physicalDevice, VkDeviceSize size, VkBufferUsageFlags usage,
                      VkMemoryPropertyFlags properties);

    /**
     * @brief   メモリをアドレス空間にマップし、そのアドレスポインタを取得する
     *
     * @param[in]  device  -> デバイス
     * @param[in]  size    -> マップするサイズ
     */
    void Map(VkDevice device, VkDeviceSize size);

    /**
     * @brief   メモリブロックをコピーする
     *
     * @param[in]  src  -> コピーするデータ
     * @param[in]  size -> コピーするサイズ
     */
    void MemCpy(const void* src, VkDeviceSize size) const;

    /**
     * @brief   メモリのマップを解除する
     *
     * @param[in]  device  -> デバイス
     */
    void UnMap(VkDevice device) const;

    /**
     * @brief   リソースを破棄する
     *
     * @param[in]  device  -> デバイス
     */
    void Destroy(VkDevice device);

    /**
     * @brief   バッファを取得する
     *
     * @return バッファ
     */
    VkBuffer GetBuffer() const { return buffer; }

private:
    VkBuffer buffer; ///< バッファ
    VkDeviceMemory memory; ///< メモリ
    void* mapped; ///< マップ領域へのアドレス
};

/**
 * @brief   イメージを扱うクラス
 */
class CubismImageVulkan
{
public:
    /**
     * @brief   コンストラクタ
     */
    CubismImageVulkan();

    /**
     * @brief   物理デバイスのメモリタイプのインデックスを探す
     *
     * @param[in]  physicalDevice -> 物理デバイス
     * @param[in]  typeFilter     -> メモリタイプが存在していたら設定されるビットマスク
     * @param[in]  properties     -> メモリがデバイスにアクセスするときのタイプ
     */
    csmUint32 FindMemoryType(VkPhysicalDevice physicalDevice, csmUint32 typeFilter, VkMemoryPropertyFlags properties);

    /**
     * @brief   イメージを作成する
     *
     * @param[in]  device          -> デバイス
     * @param[in]  physicalDevice  -> 物理デバイス
     * @param[in]  w           -> 横幅
     * @param[in]  h          -> 高さ
     * @param[in]  mipLevel        -> ミップマップのレベル
     * @param[in]  format          -> フォーマット
     * @param[in]  tiling          -> タイリング配置の設定
     * @param[in]  usage           -> イメージの使用目的を指定するビットマスク
     */
    void CreateImage(VkDevice device, VkPhysicalDevice physicalDevice,
                     csmInt32 w, csmInt32 h,
                     csmInt32 mipLevel, VkFormat format, VkImageTiling tiling, VkImageUsageFlags usage);

    /**
     * @brief   ビューを作成する
     *
     * @param[in]  device          -> デバイス
     * @param[in]  format          -> フォーマット
     * @param[in]  aspectFlags     -> どのアスペクトマスクがビューに含まれるかを指定するビットマスク
     * @param[in]  mipLevel        -> ミップマップのレベル
     */
    void CreateView(VkDevice device, VkFormat format, VkImageAspectFlags aspectFlags, csmInt32 mipLevel);

    /**
     * @brief   サンプラーを作成する
     *
     * @param[in]  device          -> デバイス
     * @param[in]  maxAnistropy    -> 異方性の値の最大値
     * @param[in]  mipLevel        -> ミップマップのレベル
     */
    void CreateSampler(VkDevice device, csmFloat32 maxAnistropy, csmUint32 mipLevel);

    /**
     * @brief   イメージのレイアウトを変更する
     *
     * @param[in]  commandBuffer  -> コマンドバッファ
     * @param[in]  newLayout      -> 新しいレイアウト
     * @param[in]  mipLevels      -> ミップマップのレベル
     */
    void SetImageLayout(VkCommandBuffer commandBuffer, VkImageLayout newLayout, csmUint32 mipLevels, VkImageAspectFlags aspectMask);

    /**
     * @brief   vkCmdEndRendering後のパイプラインバリアで変わってしまうイメージレイアウトを保存する
     *
     * @param[in]  newLayout  -> 新しいレイアウト
     */
    void SetCurrentLayout(VkImageLayout newLayout);

    /**
     * @brief   リソースを破棄する
     *
     * @param[in]  device  -> デバイス
     */
    void Destroy(VkDevice device);

    /**
     * @brief   イメージを取得する
     *
     * @return イメージ
     */
    VkImage GetImage() const { return image; }

    /**
     * @brief   ビューを取得する
     *
     * @return ビュー
     */
    VkImageView GetView() const { return view; }

    /**
     * @brief   サンプラーを取得する
     *
     * @return サンプラー
     */
    VkSampler GetSampler() const { return sampler; }

    /**
     * @brief   widthを取得する
     *
     * @return width
     */
    csmInt32 GetWidth() const { return width; }

    /**
     * @brief   heightを取得する
     *
     * @return height
     */
    csmInt32 GetHeight() const { return height; }

private:
    VkImage image; ///< バッファ
    VkDeviceMemory memory; ///< メモリ
    VkImageView view; ///< ビュー
    VkSampler sampler; ///< サンプラー
    VkImageLayout currentLayout; ///< 現在のイメージレイアウト
    csmInt32 width, height;
};
}}}
