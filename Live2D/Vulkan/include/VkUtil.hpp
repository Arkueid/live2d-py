/**
 * VkUtil — 共通 Vulkan ヘルパー（バッファ/イメージ/ミップマップ生成など）
 * V2 / V3 両方のレンダラから使用する。VulkanManager シングルトンに依存する。
 */

#pragma once
#include <vulkan/vulkan.h>

namespace VkUtil {

/// メモリタイプを検索する
uint32_t FindMemoryType(VkPhysicalDevice physicalDevice, uint32_t typeFilter,
                        VkMemoryPropertyFlags properties);

/// ホスト/デバイスバッファを作成する
void CreateBuffer(VkDevice device, VkPhysicalDevice physicalDevice, VkDeviceSize size,
                  VkBufferUsageFlags usage, VkMemoryPropertyFlags properties,
                  VkBuffer* buffer, VkDeviceMemory* memory);

/// イメージを作成する
void CreateImage(VkDevice device, VkPhysicalDevice physicalDevice, uint32_t width, uint32_t height,
                 uint32_t mipLevels, VkFormat format, VkImageTiling tiling,
                 VkImageUsageFlags usage, VkMemoryPropertyFlags properties,
                 VkImage* image, VkDeviceMemory* memory);

/// イメージビューを作成する
VkImageView CreateImageView(VkDevice device, VkImage image, VkFormat format,
                            VkImageAspectFlags aspectMask, uint32_t mipLevels);

/// イメージレイアウトを遷移する（単発コマンドバッファに記録する）
void TransitionImageLayout(VkCommandBuffer commandBuffer, VkImage image, VkImageLayout oldLayout,
                           VkImageLayout newLayout, uint32_t mipLevels);

/// バッファからイメージへコピーする（単発コマンドバッファに記録する）
void CopyBufferToImage(VkCommandBuffer commandBuffer, VkBuffer buffer, VkImage image,
                       uint32_t width, uint32_t height);

/// ミップマップを生成する（最終レイアウトは SHADER_READ_ONLY_OPTIMAL）
/// image は TRANSFER_SRC | TRANSFER_DST | SAMPLED usage を持つこと
void GenerateMipmaps(VkImage image, uint32_t texWidth, uint32_t texHeight, uint32_t mipLevels);

/// サンプラーを作成する
VkSampler CreateSampler(VkDevice device, VkSamplerAddressMode addressMode, VkFilter minFilter,
                        VkFilter magFilter, VkSamplerMipmapMode mipMode, float maxAnisotropy,
                        uint32_t mipLevels);

/// png ピクセル(RGBA8, top-down)からサンプラ付きテクスチャを作成する
/// 成功時 *outView / *outSampler を返す。staging バッファは内部で破棄する
bool CreateTextureFromPixels(VkDevice device, VkPhysicalDevice physicalDevice,
                             const unsigned char* pixels, int width, int height,
                             VkImage* outImage, VkDeviceMemory* outMemory,
                             VkImageView* outView, VkSampler* outSampler, float maxAnisotropy);

}   // namespace VkUtil
