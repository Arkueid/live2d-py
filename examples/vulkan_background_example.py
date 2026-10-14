"""Vulkan 背景图注入示例（Python 侧，pip 包 `vulkan` 实现）。

演示你的 Python Vulkan 封装如何借助帧解耦接口绘制一张全屏背景图：
背景录制进当前帧命令缓冲（clear 之后、Live2D 模型之前），模型的
premultiplied-alpha 会正确叠加在背景之上。

前置条件 —— native 侧帧解耦接口（Vulkan 构建的 live2d pyd 已实现）：
    1. live2d.vkGetHandles() -> dict
       {"physical_device": int, "device": int, "color_format": int,
        "frames_in_flight": int}
       vkInit 之后调用；Vulkan 句柄本质是整数/指针，可直接传给 vulkan 包。
    2. live2d.vkBeginFrame() -> (cmd, frame_index, width, height) | None
       acquire → 帧命令缓冲复位并开始录制 → 打开主 rendering 区间（清屏为透明）。
       返回 None 表示交换链失效已重建，本帧应跳过。
    3. live2d.vkEndFrame()
       结束主区间 → 提交（等待 acquire 信号量）→ present。每帧 Begin 后必须调用。
    4. live2d.vkSubmitOnce(fn)
       native 分配一次性命令缓冲并调用 fn(cmd: int)，随后 submit 并等待完成。
       用于纹理上传等必须在 rendering 区间之外执行的 GPU 操作。

使用流程（每帧主循环）：
    frame = live2d.vkBeginFrame()
    if frame is not None:
        cmd, frame_index, width, height = frame
        bg.on_frame(cmd, width, height)   # 先录背景（z 序最底层）
        model.Draw()                       # 模型仅录制；多模型可继续插入
        live2d.vkEndFrame()

    bg.destroy()                                      # vkDestroy 之前

依赖：pip install vulkan
注意：首次运行前请按所用 vulkan 包版本微调个别返回值解包
      （如 vkMapMemory / vkCreateGraphicsPipelines）。
"""

import base64
import ctypes

import vulkan as vk

import live2d

# glslc 编译的 SPIR-V（GLSL 源码见文件末尾，改动后用 glslc 重编并替换 base64 即可）
_VERT_SPV = base64.b64decode(
    "AwIjBwAAAQALAA0AKwAAAAAAAAARAAIAAQAAAAsABgABAAAAR0xTTC5zdGQuNDUwAAAAAA4AAwAAAAAAAQAAAA8ACAAAAAAABAAAAG1haW4AAAAACQAAAAwAAAAdAAAAAwADAAIAAADCAQAABAAKAEdMX0dPT0dMRV9jcHBfc3R5bGVfbGluZV9kaXJlY3RpdmUAAAQACABHTF9HT09HTEVfaW5jbHVkZV9kaXJlY3RpdmUABQAEAAQAAABtYWluAAAAAAUAAwAJAAAAdlVWAAUABgAMAAAAZ2xfVmVydGV4SW5kZXgAAAUABgAbAAAAZ2xfUGVyVmVydGV4AAAAAAYABgAbAAAAAAAAAGdsX1Bvc2l0aW9uAAYABwAbAAAAAQAAAGdsX1BvaW50U2l6ZQAAAAAGAAcAGwAAAAIAAABnbF9DbGlwRGlzdGFuY2UABgAHABsAAAADAAAAZ2xfQ3VsbERpc3RhbmNlAAUAAwAdAAAAAAAAAEcABAAJAAAAHgAAAAAAAABHAAQADAAAAAsAAAAqAAAARwADABsAAAACAAAASAAFABsAAAAAAAAACwAAAAAAAABIAAUAGwAAAAEAAAALAAAAAQAAAEgABQAbAAAAAgAAAAsAAAADAAAASAAFABsAAAADAAAACwAAAAQAAAATAAIAAgAAACEAAwADAAAAAgAAABYAAwAGAAAAIAAAABcABAAHAAAABgAAAAIAAAAgAAQACAAAAAMAAAAHAAAAOwAEAAgAAAAJAAAAAwAAABUABAAKAAAAIAAAAAEAAAAgAAQACwAAAAEAAAAKAAAAOwAEAAsAAAAMAAAAAQAAACsABAAKAAAADgAAAAEAAAArAAQACgAAABAAAAACAAAAFwAEABcAAAAGAAAABAAAABUABAAYAAAAIAAAAAAAAAArAAQAGAAAABkAAAABAAAAHAAEABoAAAAGAAAAGQAAAB4ABgAbAAAAFwAAAAYAAAAaAAAAGgAAACAABAAcAAAAAwAAABsAAAA7AAQAHAAAAB0AAAADAAAAKwAEAAoAAAAeAAAAAAAAACsABAAGAAAAIAAAAAAAAEArAAQABgAAACIAAAAAAIA/KwAEAAYAAAAlAAAAAAAAACAABAApAAAAAwAAABcAAAA2AAUAAgAAAAQAAAAAAAAAAwAAAPgAAgAFAAAAPQAEAAoAAAANAAAADAAAAMQABQAKAAAADwAAAA0AAAAOAAAAxwAFAAoAAAARAAAADwAAABAAAABvAAQABgAAABIAAAARAAAAPQAEAAoAAAATAAAADAAAAMcABQAKAAAAFAAAABMAAAAQAAAAbwAEAAYAAAAVAAAAFAAAAFAABQAHAAAAFgAAABIAAAAVAAAAPgADAAkAAAAWAAAAPQAEAAcAAAAfAAAACQAAAI4ABQAHAAAAIQAAAB8AAAAgAAAAUAAFAAcAAAAjAAAAIgAAACIAAACDAAUABwAAACQAAAAhAAAAIwAAAFEABQAGAAAAJgAAACQAAAAAAAAAUQAFAAYAAAAnAAAAJAAAAAEAAABQAAcAFwAAACgAAAAmAAAAJwAAACUAAAAiAAAAQQAFACkAAAAqAAAAHQAAAB4AAAA+AAMAKgAAACgAAAD9AAEAOAABAA==")
_FRAG_SPV = base64.b64decode(
    "AwIjBwAAAQALAA0AFAAAAAAAAAARAAIAAQAAAAsABgABAAAAR0xTTC5zdGQuNDUwAAAAAA4AAwAAAAAAAQAAAA8ABwAEAAAABAAAAG1haW4AAAAACQAAABEAAAAQAAMABAAAAAcAAAADAAMAAgAAAMIBAAAEAAoAR0xfR09PR0xFX2NwcF9zdHlsZV9saW5lX2RpcmVjdGl2ZQAABAAIAEdMX0dPT0dMRV9pbmNsdWRlX2RpcmVjdGl2ZQAFAAQABAAAAG1haW4AAAAABQAFAAkAAABvdXRDb2xvcgAAAAAFAAQADQAAAHVUZXgAAAAABQADABEAAAB2VVYARwAEAAkAAAAeAAAAAAAAAEcABAANAAAAIQAAAAAAAABHAAQADQAAACIAAAAAAAAARwAEABEAAAAeAAAAAAAAABMAAgACAAAAIQADAAMAAAACAAAAFgADAAYAAAAgAAAAFwAEAAcAAAAGAAAABAAAACAABAAIAAAAAwAAAAcAAAA7AAQACAAAAAkAAAADAAAAGQAJAAoAAAAGAAAAAQAAAAAAAAAAAAAAAAAAAAEAAAAAAAAAGwADAAsAAAAKAAAAIAAEAAwAAAAAAAAACwAAADsABAAMAAAADQAAAAAAAAAXAAQADwAAAAYAAAACAAAAIAAEABAAAAABAAAADwAAADsABAAQAAAAEQAAAAEAAAA2AAUAAgAAAAQAAAAAAAAAAwAAAPgAAgAFAAAAPQAEAAsAAAAOAAAADQAAAD0ABAAPAAAAEgAAABEAAABXAAUABwAAABMAAAAOAAAAEgAAAD4AAwAJAAAAEwAAAP0AAQA4AAEA")


def _create_shader_module(device, spv: bytes):
    code = (ctypes.c_uint32 * (len(spv) // 4)).from_buffer_copy(spv)
    return vk.vkCreateShaderModule(device, vk.VkShaderModuleCreateInfo(
        codeSize=len(spv), pCode=code), None)


class VulkanBackground:
    """在 Live2D 帧回调中绘制的全屏背景图（资源全静态，无需按 frame_index 轮转）。"""

    def __init__(self, rgba: bytes, width: int, height: int):
        handles = live2d.vkGetHandles()
        self._pd = handles["physical_device"]
        self._dev = handles["device"]
        self._format = handles["color_format"]
        self._create_texture(rgba, width, height)
        self._create_descriptors()
        self._create_pipeline()

    # ---------- 工具 ----------

    def _find_memory_type(self, type_bits: int, required: int) -> int:
        props = vk.vkGetPhysicalDeviceMemoryProperties(self._pd)
        for i in range(props.memoryTypeCount):
            mt = props.memoryTypes[i]
            if (type_bits & (1 << i)) and (mt.propertyFlags & required) == required:
                return i
        raise RuntimeError("no suitable memory type")

    # ---------- GPU 资源 ----------

    def _create_texture(self, rgba: bytes, width: int, height: int):
        dev = self._dev
        fmt = vk.VK_FORMAT_R8G8B8A8_UNORM
        subresource_range = vk.VkImageSubresourceRange(
            aspectMask=vk.VK_IMAGE_ASPECT_COLOR_BIT, baseMipLevel=0, levelCount=1,
            baseArrayLayer=0, layerCount=1)

        # 目标图像（DEVICE_LOCAL）
        self._image = vk.vkCreateImage(dev, vk.VkImageCreateInfo(
            imageType=vk.VK_IMAGE_TYPE_2D, format=fmt,
            extent=vk.VkExtent3D(width=width, height=height, depth=1),
            mipLevels=1, arrayLayers=1, samples=vk.VK_SAMPLE_COUNT_1_BIT,
            tiling=vk.VK_IMAGE_TILING_OPTIMAL,
            usage=vk.VK_IMAGE_USAGE_TRANSFER_DST_BIT | vk.VK_IMAGE_USAGE_SAMPLED_BIT,
            sharingMode=vk.VK_SHARING_MODE_EXCLUSIVE,
            initialLayout=vk.VK_IMAGE_LAYOUT_UNDEFINED), None)
        req = vk.vkGetImageMemoryRequirements(dev, self._image)
        self._image_mem = vk.vkAllocateMemory(dev, vk.VkMemoryAllocateInfo(
            allocationSize=req.size,
            memoryTypeIndex=self._find_memory_type(
                req.memoryTypeBits, vk.VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT)), None)
        vk.vkBindImageMemory(dev, self._image, self._image_mem, 0)

        # staging（HOST_VISIBLE + COHERENT）
        staging = vk.vkCreateBuffer(dev, vk.VkBufferCreateInfo(
            size=len(rgba), usage=vk.VK_BUFFER_USAGE_TRANSFER_SRC_BIT,
            sharingMode=vk.VK_SHARING_MODE_EXCLUSIVE), None)
        sreq = vk.vkGetBufferMemoryRequirements(dev, staging)
        staging_mem = vk.vkAllocateMemory(dev, vk.VkMemoryAllocateInfo(
            allocationSize=sreq.size,
            memoryTypeIndex=self._find_memory_type(
                sreq.memoryTypeBits,
                vk.VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT |
                vk.VK_MEMORY_PROPERTY_HOST_COHERENT_BIT)), None)
        vk.vkBindBufferMemory(dev, staging, staging_mem, 0)
        ptr = vk.vkMapMemory(dev, staging_mem, 0, len(rgba), 0, 0)
        ctypes.memmove(ptr, rgba, len(rgba))
        vk.vkUnmapMemory(dev, staging_mem)

        # 上传：layout 迁移 + 拷贝（在 rendering 区间之外，经 native 一次性命令缓冲）
        to_transfer = vk.VkImageMemoryBarrier(
            srcAccessMask=0, dstAccessMask=vk.VK_ACCESS_TRANSFER_WRITE_BIT,
            oldLayout=vk.VK_IMAGE_LAYOUT_UNDEFINED,
            newLayout=vk.VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
            srcQueueFamilyIndex=vk.VK_QUEUE_FAMILY_IGNORED,
            dstQueueFamilyIndex=vk.VK_QUEUE_FAMILY_IGNORED,
            image=self._image, subresourceRange=subresource_range)
        region = vk.VkBufferImageCopy(
            bufferOffset=0, bufferRowLength=0, bufferImageHeight=0,
            imageSubresource=vk.VkImageSubresourceLayers(
                aspectMask=vk.VK_IMAGE_ASPECT_COLOR_BIT, mipLevel=0,
                baseArrayLayer=0, layerCount=1),
            imageOffset=vk.VkOffset3D(x=0, y=0, z=0),
            imageExtent=vk.VkExtent3D(width=width, height=height, depth=1))
        to_shader_read = vk.VkImageMemoryBarrier(
            srcAccessMask=vk.VK_ACCESS_TRANSFER_WRITE_BIT,
            dstAccessMask=vk.VK_ACCESS_SHADER_READ_BIT,
            oldLayout=vk.VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
            newLayout=vk.VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL,
            srcQueueFamilyIndex=vk.VK_QUEUE_FAMILY_IGNORED,
            dstQueueFamilyIndex=vk.VK_QUEUE_FAMILY_IGNORED,
            image=self._image, subresourceRange=subresource_range)

        def record(cmd):
            vk.vkCmdPipelineBarrier(
                cmd, vk.VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT,
                vk.VK_PIPELINE_STAGE_TRANSFER_BIT, 0, 0, None, 0, None, 1, [to_transfer])
            vk.vkCmdCopyBufferToImage(
                cmd, staging, self._image,
                vk.VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, 1, [region])
            vk.vkCmdPipelineBarrier(
                cmd, vk.VK_PIPELINE_STAGE_TRANSFER_BIT,
                vk.VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT, 0, 0, None, 0, None, 1,
                [to_shader_read])

        live2d.vkSubmitOnce(record)
        vk.vkDestroyBuffer(dev, staging, None)
        vk.vkFreeMemory(dev, staging_mem, None)

        # view + sampler
        self._view = vk.vkCreateImageView(dev, vk.VkImageViewCreateInfo(
            image=self._image, viewType=vk.VK_IMAGE_VIEW_TYPE_2D, format=fmt,
            subresourceRange=subresource_range), None)
        self._sampler = vk.vkCreateSampler(dev, vk.VkSamplerCreateInfo(
            magFilter=vk.VK_FILTER_LINEAR, minFilter=vk.VK_FILTER_LINEAR,
            mipmapMode=vk.VK_SAMPLER_MIPMAP_MODE_NEAREST,
            addressModeU=vk.VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE,
            addressModeV=vk.VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE,
            addressModeW=vk.VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE), None)

    def _create_descriptors(self):
        dev = self._dev
        self._set_layout = vk.vkCreateDescriptorSetLayout(
            dev, vk.VkDescriptorSetLayoutCreateInfo(
                bindingCount=1, pBindings=[vk.VkDescriptorSetLayoutBinding(
                    binding=0, descriptorType=vk.VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER,
                    descriptorCount=1, stageFlags=vk.VK_SHADER_STAGE_FRAGMENT_BIT)]), None)
        self._pool = vk.vkCreateDescriptorPool(dev, vk.VkDescriptorPoolCreateInfo(
            maxSets=1, poolSizeCount=1, pPoolSizes=[vk.VkDescriptorPoolSize(
                type=vk.VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, descriptorCount=1)]), None)
        self._set = vk.vkAllocateDescriptorSets(dev, vk.VkDescriptorSetAllocateInfo(
            descriptorPool=self._pool, descriptorSetCount=1,
            pSetLayouts=[self._set_layout]))[0]
        vk.vkUpdateDescriptorSets(dev, 1, [vk.VkWriteDescriptorSet(
            dstSet=self._set, dstBinding=0, dstArrayElement=0, descriptorCount=1,
            descriptorType=vk.VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER,
            pImageInfo=[vk.VkDescriptorImageInfo(
                sampler=self._sampler, imageView=self._view,
                imageLayout=vk.VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL)])], 0, None)

    def _create_pipeline(self):
        dev = self._dev
        vs = _create_shader_module(dev, _VERT_SPV)
        fs = _create_shader_module(dev, _FRAG_SPV)
        self._pipeline_layout = vk.vkCreatePipelineLayout(
            dev, vk.VkPipelineLayoutCreateInfo(
                setLayoutCount=1, pSetLayouts=[self._set_layout]), None)

        # 动态渲染：声明与 Live2D 主 pass 一致的 color 格式（无 render pass）
        rendering_ci = vk.VkPipelineRenderingCreateInfo(
            colorAttachmentCount=1, pColorAttachmentFormats=[self._format])

        color_write = (vk.VK_COLOR_COMPONENT_R_BIT | vk.VK_COLOR_COMPONENT_G_BIT |
                       vk.VK_COLOR_COMPONENT_B_BIT | vk.VK_COLOR_COMPONENT_A_BIT)
        self._pipeline = vk.vkCreateGraphicsPipelines(dev, vk.VK_NULL_HANDLE, 1, [
            vk.VkGraphicsPipelineCreateInfo(
                pNext=rendering_ci,
                stageCount=2, pStages=[
                    vk.VkPipelineShaderStageCreateInfo(
                        stage=vk.VK_SHADER_STAGE_VERTEX_BIT, module=vs, pName="main"),
                    vk.VkPipelineShaderStageCreateInfo(
                        stage=vk.VK_SHADER_STAGE_FRAGMENT_BIT, module=fs, pName="main")],
                # 无顶点缓冲：全屏三角形由 gl_VertexIndex 生成
                pVertexInputState=vk.VkPipelineVertexInputStateCreateInfo(
                    vertexBindingDescriptionCount=0, vertexAttributeDescriptionCount=0),
                pInputAssemblyState=vk.VkPipelineInputAssemblyStateCreateInfo(
                    topology=vk.VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST),
                pViewportState=vk.VkPipelineViewportStateCreateInfo(
                    viewportCount=1, scissorCount=1),
                pRasterizationState=vk.VkPipelineRasterizationStateCreateInfo(
                    polygonMode=vk.VK_POLYGON_MODE_FILL, cullMode=vk.VK_CULL_MODE_NONE,
                    frontFace=vk.VK_FRONT_FACE_COUNTER_CLOCKWISE, lineWidth=1.0),
                pMultisampleState=vk.VkPipelineMultisampleStateCreateInfo(
                    rasterizationSamples=vk.VK_SAMPLE_COUNT_1_BIT),
                pDepthStencilState=None,
                pColorBlendState=vk.VkPipelineColorBlendStateCreateInfo(
                    attachmentCount=1, pAttachments=[vk.VkPipelineColorBlendAttachmentState(
                        blendEnable=vk.VK_FALSE, colorWriteMask=color_write)]),
                pDynamicState=vk.VkPipelineDynamicStateCreateInfo(
                    dynamicStateCount=2, pDynamicStates=[
                        vk.VK_DYNAMIC_STATE_VIEWPORT, vk.VK_DYNAMIC_STATE_SCISSOR]),
                layout=self._pipeline_layout,
                renderPass=vk.VK_NULL_HANDLE)], None)[0]
        vk.vkDestroyShaderModule(dev, vs, None)
        vk.vkDestroyShaderModule(dev, fs, None)

    # ---------- 帧录制 ----------

    def on_frame(self, cmd: int, width: int, height: int):
        """在 vkBeginFrame 之后、model.Draw() 之前调用：向帧命令缓冲录制背景。

        此时主 rendering 区间已打开（清屏为透明），交换链 image 已绑定为
        color attachment。动态 viewport/scissor → 绑管线 → 绑纹理 → 画全屏三角形。
        正高度 viewport 下 Vulkan NDC y 向下，UV(0,0) 对应屏幕左上，
        与按行优先（顶行在前）的 RGBA 数据一致，无需翻转。
        """
        vk.vkCmdSetViewport(cmd, 0, 1, [vk.VkViewport(
            x=0.0, y=0.0, width=float(width), height=float(height),
            minDepth=0.0, maxDepth=1.0)])
        vk.vkCmdSetScissor(cmd, 0, 1, [vk.VkRect2D(
            offset=vk.VkOffset2D(x=0, y=0),
            extent=vk.VkExtent2D(width=width, height=height))])
        vk.vkCmdBindPipeline(cmd, vk.VK_PIPELINE_BIND_POINT_GRAPHICS, self._pipeline)
        vk.vkCmdBindDescriptorSets(cmd, vk.VK_PIPELINE_BIND_POINT_GRAPHICS,
                                   self._pipeline_layout, 0, 1, [self._set], 0, None)
        vk.vkCmdDraw(cmd, 3, 1, 0, 0)

    # ---------- 释放 ----------

    def destroy(self):
        dev = self._dev
        vk.vkDeviceWaitIdle(dev)
        vk.vkDestroyPipeline(dev, self._pipeline, None)
        vk.vkDestroyPipelineLayout(dev, self._pipeline_layout, None)
        vk.vkDestroyDescriptorPool(dev, self._pool, None)
        vk.vkDestroyDescriptorSetLayout(dev, self._set_layout, None)
        vk.vkDestroySampler(dev, self._sampler, None)
        vk.vkDestroyImageView(dev, self._view, None)
        vk.vkDestroyImage(dev, self._image, None)
        vk.vkFreeMemory(dev, self._image_mem, None)


# ---------- 内嵌 SPIR-V 的 GLSL 源码（glslc xxx -o xxx.spv 重编）----------
#
# bg.vert:
#   #version 450
#   // 全屏三角形：3 个顶点覆盖整个屏幕，无需顶点缓冲
#   layout(location = 0) out vec2 vUV;
#   void main() {
#       vUV = vec2((gl_VertexIndex << 1) & 2, gl_VertexIndex & 2);
#       gl_Position = vec4(vUV * 2.0 - 1.0, 0.0, 1.0);
#   }
#
# bg.frag:
#   #version 450
#   layout(set = 0, binding = 0) uniform sampler2D uTex;
#   layout(location = 0) in vec2 vUV;
#   layout(location = 0) out vec4 outColor;
#   void main() { outColor = texture(uTex, vUV); }
