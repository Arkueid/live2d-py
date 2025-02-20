/**
 * Copyright(c) Live2D Inc. All rights reserved.
 *
 * Use of this source code is governed by the Live2D Open Software license
 * that can be found at https://www.live2d.com/eula/live2d-open-software-license-agreement_en.html.
 */

#include "CubismOffscreenSurface_Vulkan.hpp"

//------------ LIVE2D NAMESPACE ------------
namespace Live2D { namespace Cubism { namespace Framework { namespace Rendering {
CubismOffscreenSurface_Vulkan::CubismOffscreenSurface_Vulkan()
    : _bufferWidth(0)
    , _bufferHeight(0)
{ }

void CubismOffscreenSurface_Vulkan::BeginDraw(VkCommandBuffer commandBuffer, csmFloat32 r, csmFloat32 g, csmFloat32 b,
                                            csmFloat32 a)
{
    _colorImage->SetImageLayout(commandBuffer, VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL, 1, VK_IMAGE_ASPECT_COLOR_BIT);
    VkRenderingAttachmentInfoKHR colorAttachment{};
    colorAttachment.sType = VK_STRUCTURE_TYPE_RENDERING_ATTACHMENT_INFO_KHR;
    colorAttachment.imageView = _colorImage->GetView();
    colorAttachment.imageLayout = VK_IMAGE_LAYOUT_ATTACHMENT_OPTIMAL_KHR;
    colorAttachment.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
    colorAttachment.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
    colorAttachment.clearValue = {{r, g, b, a}};
    _depthImage->SetImageLayout(commandBuffer, VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL, 1, VK_IMAGE_ASPECT_DEPTH_BIT | VK_IMAGE_ASPECT_STENCIL_BIT);
    VkRenderingAttachmentInfoKHR depthStencilAttachment{};
    depthStencilAttachment.sType = VK_STRUCTURE_TYPE_RENDERING_ATTACHMENT_INFO_KHR;
    depthStencilAttachment.imageView = _depthImage->GetView();
    depthStencilAttachment.imageLayout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;
    depthStencilAttachment.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
    depthStencilAttachment.storeOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
    depthStencilAttachment.clearValue = {1.0f, 0};

    VkExtent2D extent;
    extent.height = _bufferHeight;
    extent.width = _bufferWidth;

    VkRenderingInfo renderingInfo{};
    renderingInfo.sType = VK_STRUCTURE_TYPE_RENDERING_INFO;
    renderingInfo.renderArea = {{0, 0}, {extent}};
    renderingInfo.layerCount = 1;
    renderingInfo.colorAttachmentCount = 1;
    renderingInfo.pColorAttachments = &colorAttachment;
    renderingInfo.pDepthAttachment = &depthStencilAttachment;

    vkCmdBeginRendering(commandBuffer, &renderingInfo);
}

void CubismOffscreenSurface_Vulkan::EndDraw(VkCommandBuffer commandBuffer)
{
    vkCmdEndRendering(commandBuffer);

    // レイアウト変更
    VkImageMemoryBarrier memoryBarrier{};
    memoryBarrier.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
    memoryBarrier.srcAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;
    memoryBarrier.oldLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
    memoryBarrier.newLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
    memoryBarrier.image = _colorImage->GetImage();
    memoryBarrier.subresourceRange = {VK_IMAGE_ASPECT_COLOR_BIT, 0, 1, 0, 1};

    vkCmdPipelineBarrier(commandBuffer, VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT,
                         VK_PIPELINE_STAGE_BOTTOM_OF_PIPE_BIT, 0, 0, nullptr, 0, nullptr, 1, &memoryBarrier);
    _colorImage->SetCurrentLayout(VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL);

    memoryBarrier.oldLayout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;
    memoryBarrier.newLayout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;
    memoryBarrier.image = _depthImage->GetImage();
    memoryBarrier.subresourceRange = { VK_IMAGE_ASPECT_DEPTH_BIT | VK_IMAGE_ASPECT_STENCIL_BIT, 0, 1, 0, 1 };
    vkCmdPipelineBarrier(commandBuffer, VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT,
        VK_PIPELINE_STAGE_BOTTOM_OF_PIPE_BIT, 0, 0, nullptr, 0, nullptr, 1, &memoryBarrier);
    _depthImage->SetCurrentLayout(VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL);
}

void CubismOffscreenSurface_Vulkan::CreateOffscreenSurface(
    VkDevice device, VkPhysicalDevice physicalDevice,
    csmUint32 displayBufferWidth, csmUint32 displayBufferHeight,
    VkFormat surfaceFormat, VkFormat depthFormat)
{
    _colorImage = CSM_NEW CubismImageVulkan;
    _colorImage->CreateImage(device, physicalDevice, displayBufferWidth, displayBufferHeight,
                             1, surfaceFormat, VK_IMAGE_TILING_OPTIMAL,
                             VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT | VK_IMAGE_USAGE_SAMPLED_BIT |
                             VK_IMAGE_USAGE_TRANSFER_DST_BIT);
    _colorImage->CreateView(device, surfaceFormat, VK_IMAGE_ASPECT_COLOR_BIT, 1);
    _colorImage->CreateSampler(device, 1.0, 1);

    _depthImage = CSM_NEW CubismImageVulkan;
    _depthImage->CreateImage(device, physicalDevice, displayBufferWidth, displayBufferHeight,
                             1, depthFormat, VK_IMAGE_TILING_OPTIMAL,
                             VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT);
    _depthImage->CreateView(device, depthFormat, VK_IMAGE_ASPECT_DEPTH_BIT | VK_IMAGE_ASPECT_STENCIL_BIT, 1);

    _bufferWidth = displayBufferWidth;
    _bufferHeight = displayBufferHeight;
}

void CubismOffscreenSurface_Vulkan::DestroyOffscreenSurface(VkDevice device)
{
    if (_colorImage != VK_NULL_HANDLE)
    {
        _colorImage->Destroy(device);
        CSM_DELETE(_colorImage);
        _colorImage = VK_NULL_HANDLE;
    }
    if (_depthImage != VK_NULL_HANDLE)
    {
        _depthImage->Destroy(device);
        CSM_DELETE(_depthImage);
        _depthImage = VK_NULL_HANDLE;
    }
}

VkImage CubismOffscreenSurface_Vulkan::GetTextureImage() const
{
    return _colorImage->GetImage();
}

VkImageView CubismOffscreenSurface_Vulkan::GetTextureView() const
{
    return _colorImage->GetView();
}

VkSampler CubismOffscreenSurface_Vulkan::GetTextureSampler() const
{
    return _colorImage->GetSampler();
}

csmUint32 CubismOffscreenSurface_Vulkan::GetBufferWidth() const
{
    return _bufferWidth;
}

csmUint32 CubismOffscreenSurface_Vulkan::GetBufferHeight() const
{
    return _bufferHeight;
}

csmBool CubismOffscreenSurface_Vulkan::IsValid() const
{
    if (_colorImage == VK_NULL_HANDLE || _depthImage == VK_NULL_HANDLE)
    {
        return false;
    }

    return true;
}
}}}}

//------------ LIVE2D NAMESPACE ------------
