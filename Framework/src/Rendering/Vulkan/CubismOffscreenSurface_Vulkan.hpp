/**
 * Copyright(c) Live2D Inc. All rights reserved.
 *
 * Use of this source code is governed by the Live2D Open Software license
 * that can be found at https://www.live2d.com/eula/live2d-open-software-license-agreement_en.html.
 */

#pragma once
#include <vulkan/vulkan.h>
#include "CubismFramework.hpp"
#include "CubismClass_Vulkan.hpp"

//------------ LIVE2D NAMESPACE ------------
namespace Live2D { namespace Cubism { namespace Framework { namespace Rendering {
/**
 * @brief  オフスクリーン描画用構造体
 */
class CubismOffscreenSurface_Vulkan
{
public:
    CubismOffscreenSurface_Vulkan();

    /**
     * @brief   レンダリングターゲットのクリア
     *           呼ぶ場合はBeginDrawの後で呼ぶこと
     *
     * @param[in]  commandBuffer   -> コマンドバッファ
     * @param[in]   r              -> 赤(0.0~1.0)
     * @param[in]   g              -> 緑(0.0~1.0)
     * @param[in]   b              -> 青(0.0~1.0)
     * @param[in]   a              -> α(0.0~1.0)
     */
    void BeginDraw(VkCommandBuffer commandBuffer, csmFloat32 r, csmFloat32 g, csmFloat32 b, csmFloat32 a);

    /**
     * @brief   描画終了
     *
     *  @param[in]  commandBuffer                 -> コマンドバッファ
     */
    void EndDraw(VkCommandBuffer commandBuffer);

    /**
     *  @brief  CubismOffscreenSurfaceを作成する。
     *
     *  @param[in]  device                 -> 論理デバイス
     *  @param[in]  physicalDevice         -> 物理デバイス
     *  @param[in]  displayBufferWidth     -> オフスクリーンの横幅
     *  @param[in]  displayBufferHeight    -> オフスクリーンの縦幅
     *  @param[in]  surfaceFormat          -> サーフェスフォーマット
     *  @param[in]  depthFormat            -> 深度フォーマット
     */
    void CreateOffscreenSurface(
        VkDevice device, VkPhysicalDevice physicalDevice,
        csmUint32 displayBufferWidth, csmUint32 displayBufferHeight,
        VkFormat surfaceFormat, VkFormat depthFormat
    );

    /**
     * @brief   CubismOffscreenSurfaceの削除
     *
     *  @param[in]  device                 -> デバイス
     */
    void DestroyOffscreenSurface(VkDevice device);

    /**
     * @brief   イメージへのアクセッサ
     */
    VkImage GetTextureImage() const;

    /**
     * @brief   テクスチャビューへのアクセッサ
     */
    VkImageView GetTextureView() const;

    /**
     * @brief   テクスチャサンプラーへのアクセッサ
     */
    VkSampler GetTextureSampler() const;

    /**
     * @brief   バッファ幅を返す
     */
    csmUint32 GetBufferWidth() const;

    /**
     * @brief   バッファ高さを返す
     */
    csmUint32 GetBufferHeight() const;

    /**
     * @brief   現在有効かどうかを返す
     */
    bool IsValid() const;

private:
    csmUint32 _bufferWidth; ///< オフスクリーンの横幅
    csmUint32 _bufferHeight; ///< オフスクリーンの縦幅
    CubismImageVulkan* _colorImage = VK_NULL_HANDLE; ///< カラーバッファ
    CubismImageVulkan* _depthImage = VK_NULL_HANDLE; ///< 深度バッファ
};
}}}}

//------------ LIVE2D NAMESPACE ------------
