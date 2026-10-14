#include "VkRenderer.hpp"
#include "ClippingManagerVulkan.hpp"
#include "Log.hpp"
#include "Mesh.hpp"
#include "MeshContext.hpp"
#include "ModelContext.hpp"
#include "PartsDataContext.hpp"
#include "VulkanManager.hpp"
#include "VkUtil.hpp"
#include <FileUtils.hpp>
#include <cstring>
#include <vector>

namespace Live2D {
namespace V2 {

// ---- shader 加载 ----
// 经 Common::FileUtils 加载，自动走 SHADER_DIR 重定向（与 v3 共用同一拦截）。

VkShaderModule VkRenderer::loadShaderModule(const char* path) {
    unsigned int size = 0;
    unsigned char* code = Live2D::Common::FileUtils::LoadFileAsBytes(path, &size);
    if (!code || size == 0) {
        LOGE("failed to load shader file: %s", path);
        return VK_NULL_HANDLE;
    }

    VkShaderModuleCreateInfo createInfo{};
    createInfo.sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;
    createInfo.codeSize = size;
    createInfo.pCode = reinterpret_cast<const uint32_t*>(code);

    VkShaderModule module;
    if (vkCreateShaderModule(mDevice, &createInfo, nullptr, &module) != VK_SUCCESS) {
        LOGE("failed to create shader module: %s", path);
        Live2D::Common::FileUtils::ReleaseBytes(code);
        return VK_NULL_HANDLE;
    }

    Live2D::Common::FileUtils::ReleaseBytes(code);
    return module;
}

// ---- pipeline ----

static VkPipelineColorBlendAttachmentState makeBlend(VkBlendOp op, VkBlendFactor srcColor,
                                                     VkBlendFactor dstColor, VkBlendFactor srcAlpha,
                                                     VkBlendFactor dstAlpha) {
    VkPipelineColorBlendAttachmentState state{};
    state.blendEnable = VK_TRUE;
    state.colorBlendOp = op;
    state.alphaBlendOp = op;
    state.srcColorBlendFactor = srcColor;
    state.dstColorBlendFactor = dstColor;
    state.srcAlphaBlendFactor = srcAlpha;
    state.dstAlphaBlendFactor = dstAlpha;
    state.colorWriteMask = VK_COLOR_COMPONENT_R_BIT | VK_COLOR_COMPONENT_G_BIT |
                           VK_COLOR_COMPONENT_B_BIT | VK_COLOR_COMPONENT_A_BIT;
    return state;
}

// 混合因子与 GLRenderer::drawTexture 的 glBlendFuncSeparate 逐项一致
static VkPipelineColorBlendAttachmentState blendFor(int compositionType) {
    switch (compositionType) {
        case VkRenderer::COLOR_COMPOSITION_SCREEN:
            // (GL_ONE, GL_ONE / GL_ZERO, GL_ONE)
            return makeBlend(VK_BLEND_OP_ADD, VK_BLEND_FACTOR_ONE, VK_BLEND_FACTOR_ONE,
                             VK_BLEND_FACTOR_ZERO, VK_BLEND_FACTOR_ONE);
        case VkRenderer::COLOR_COMPOSITION_MULTIPLY:
            // (GL_DST_COLOR, GL_ONE_MINUS_SRC_ALPHA / GL_ZERO, GL_ONE)
            return makeBlend(VK_BLEND_OP_ADD, VK_BLEND_FACTOR_DST_COLOR,
                             VK_BLEND_FACTOR_ONE_MINUS_SRC_ALPHA, VK_BLEND_FACTOR_ZERO,
                             VK_BLEND_FACTOR_ONE);
        case VkRenderer::COLOR_COMPOSITION_NORMAL:
        default:
            // (GL_ONE, GL_ONE_MINUS_SRC_ALPHA / GL_ONE, GL_ONE_MINUS_SRC_ALPHA)
            return makeBlend(VK_BLEND_OP_ADD, VK_BLEND_FACTOR_ONE,
                             VK_BLEND_FACTOR_ONE_MINUS_SRC_ALPHA, VK_BLEND_FACTOR_ONE,
                             VK_BLEND_FACTOR_ONE_MINUS_SRC_ALPHA);
    }
}

VkPipeline VkRenderer::createPipeline(VkShaderModule vert, VkShaderModule frag,
                                      const VkPipelineColorBlendAttachmentState& blend) {
    VkPipelineShaderStageCreateInfo stages[2]{};
    stages[0].sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
    stages[0].stage = VK_SHADER_STAGE_VERTEX_BIT;
    stages[0].module = vert;
    stages[0].pName = "main";
    stages[1].sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
    stages[1].stage = VK_SHADER_STAGE_FRAGMENT_BIT;
    stages[1].module = frag;
    stages[1].pName = "main";

    VkVertexInputBindingDescription bindings[2]{};
    bindings[0] = {0, sizeof(float) * 2, VK_VERTEX_INPUT_RATE_VERTEX};   // position
    bindings[1] = {1, sizeof(float) * 2, VK_VERTEX_INPUT_RATE_VERTEX};   // uv
    VkVertexInputAttributeDescription attrs[2]{};
    attrs[0] = {0, 0, VK_FORMAT_R32G32_SFLOAT, 0};
    attrs[1] = {1, 1, VK_FORMAT_R32G32_SFLOAT, 0};

    VkPipelineVertexInputStateCreateInfo vertexInput{};
    vertexInput.sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO;
    vertexInput.vertexBindingDescriptionCount = 2;
    vertexInput.pVertexBindingDescriptions = bindings;
    vertexInput.vertexAttributeDescriptionCount = 2;
    vertexInput.pVertexAttributeDescriptions = attrs;

    VkPipelineInputAssemblyStateCreateInfo inputAssembly{};
    inputAssembly.sType = VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO;
    inputAssembly.topology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST;

    VkPipelineViewportStateCreateInfo viewportState{};
    viewportState.sType = VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO;
    viewportState.viewportCount = 1;
    viewportState.scissorCount = 1;

    VkPipelineRasterizationStateCreateInfo rasterizer{};
    rasterizer.sType = VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO;
    rasterizer.polygonMode = VK_POLYGON_MODE_FILL;
    rasterizer.cullMode = VK_CULL_MODE_NONE;   // 动态设置
    rasterizer.frontFace = VK_FRONT_FACE_CLOCKWISE;
    rasterizer.lineWidth = 1.0f;

    VkPipelineMultisampleStateCreateInfo multisampling{};
    multisampling.sType = VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO;
    multisampling.rasterizationSamples = VK_SAMPLE_COUNT_1_BIT;

    VkPipelineDepthStencilStateCreateInfo depthStencil{};
    depthStencil.sType = VK_STRUCTURE_TYPE_PIPELINE_DEPTH_STENCIL_STATE_CREATE_INFO;
    depthStencil.depthTestEnable = VK_FALSE;
    depthStencil.depthWriteEnable = VK_FALSE;

    VkPipelineColorBlendStateCreateInfo colorBlend{};
    colorBlend.sType = VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO;
    colorBlend.attachmentCount = 1;
    colorBlend.pAttachments = &blend;

    VkDynamicState dynamicStates[] = {
        VK_DYNAMIC_STATE_VIEWPORT,
        VK_DYNAMIC_STATE_SCISSOR,
        VK_DYNAMIC_STATE_CULL_MODE_EXT,
        VK_DYNAMIC_STATE_FRONT_FACE_EXT,
    };
    VkPipelineDynamicStateCreateInfo dynamicState{};
    dynamicState.sType = VK_STRUCTURE_TYPE_PIPELINE_DYNAMIC_STATE_CREATE_INFO;
    dynamicState.dynamicStateCount = 4;
    dynamicState.pDynamicStates = dynamicStates;

    VkPipelineRenderingCreateInfo renderingInfo{};
    renderingInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_RENDERING_CREATE_INFO;
    renderingInfo.colorAttachmentCount = 1;
    renderingInfo.pColorAttachmentFormats = &mColorFormat;

    VkGraphicsPipelineCreateInfo pipelineInfo{};
    pipelineInfo.sType = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO;
    pipelineInfo.pNext = &renderingInfo;
    pipelineInfo.stageCount = 2;
    pipelineInfo.pStages = stages;
    pipelineInfo.pVertexInputState = &vertexInput;
    pipelineInfo.pInputAssemblyState = &inputAssembly;
    pipelineInfo.pViewportState = &viewportState;
    pipelineInfo.pRasterizationState = &rasterizer;
    pipelineInfo.pMultisampleState = &multisampling;
    pipelineInfo.pDepthStencilState = &depthStencil;
    pipelineInfo.pColorBlendState = &colorBlend;
    pipelineInfo.pDynamicState = &dynamicState;
    pipelineInfo.layout = mPipelineLayout;

    VkPipeline pipeline;
    if (vkCreateGraphicsPipelines(mDevice, VK_NULL_HANDLE, 1, &pipelineInfo, nullptr, &pipeline) !=
        VK_SUCCESS) {
        LOGE("failed to create graphics pipeline");
        return VK_NULL_HANDLE;
    }
    return pipeline;
}

void VkRenderer::createLayoutsAndPipelines() {
    // descriptor set layout: b0=动态UBO, b1=模型纹理, b2=遮罩纹理
    VkDescriptorSetLayoutBinding bindings[3]{};
    bindings[0].binding = 0;
    bindings[0].descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER_DYNAMIC;
    bindings[0].descriptorCount = 1;
    bindings[0].stageFlags = VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT;
    bindings[1].binding = 1;
    bindings[1].descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
    bindings[1].descriptorCount = 1;
    bindings[1].stageFlags = VK_SHADER_STAGE_FRAGMENT_BIT;
    bindings[2].binding = 2;
    bindings[2].descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
    bindings[2].descriptorCount = 1;
    bindings[2].stageFlags = VK_SHADER_STAGE_FRAGMENT_BIT;

    VkDescriptorSetLayoutCreateInfo layoutInfo{};
    layoutInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
    layoutInfo.bindingCount = 3;
    layoutInfo.pBindings = bindings;
    if (vkCreateDescriptorSetLayout(mDevice, &layoutInfo, nullptr, &mDescLayout) != VK_SUCCESS) {
        LOGE("failed to create descriptor set layout");
        return;
    }

    VkPipelineLayoutCreateInfo pipelineLayoutInfo{};
    pipelineLayoutInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
    pipelineLayoutInfo.setLayoutCount = 1;
    pipelineLayoutInfo.pSetLayouts = &mDescLayout;
    if (vkCreatePipelineLayout(mDevice, &pipelineLayoutInfo, nullptr, &mPipelineLayout) !=
        VK_SUCCESS) {
        LOGE("failed to create pipeline layout");
        return;
    }

    VkShaderModule vertNormal = loadShaderModule("FrameworkShaders/V2Normal.vert.spv");
    VkShaderModule fragNormal = loadShaderModule("FrameworkShaders/V2Normal.frag.spv");
    VkShaderModule vertMask = loadShaderModule("FrameworkShaders/V2Mask.vert.spv");
    VkShaderModule fragMask = loadShaderModule("FrameworkShaders/V2Mask.frag.spv");
    if (!vertNormal || !fragNormal || !vertMask || !fragMask) {
        LOGE("V2 Vulkan shaders not found under ./FrameworkShaders — copy V2*.spv next to the "
             "executable");
        return;
    }

    // normal 程序 × 3 混合
    for (int b = 0; b < 3; b++)
        mPipelines[b] = createPipeline(vertNormal, fragNormal, blendFor(b));
    // mask 程序 × 3 混合
    for (int b = 0; b < 3; b++)
        mPipelines[3 + b] = createPipeline(vertMask, fragMask, blendFor(b));

    vkDestroyShaderModule(mDevice, vertNormal, nullptr);
    vkDestroyShaderModule(mDevice, fragNormal, nullptr);
    vkDestroyShaderModule(mDevice, vertMask, nullptr);
    vkDestroyShaderModule(mDevice, fragMask, nullptr);
}

void VkRenderer::createMaskTarget() {
    // mask image 使用与 swapchain 相同的格式，管线 rendering info 即可对两种目标通用
    VkUtil::CreateImage(mDevice, mPhysicalDevice, MASK_SIZE, MASK_SIZE, 1, mColorFormat,
                        VK_IMAGE_TILING_OPTIMAL,
                        VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT | VK_IMAGE_USAGE_SAMPLED_BIT,
                        VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, &mMaskImage, &mMaskMemory);
    mMaskView = VkUtil::CreateImageView(mDevice, mMaskImage, mColorFormat,
                                        VK_IMAGE_ASPECT_COLOR_BIT, 1);
    mMaskSampler = VkUtil::CreateSampler(mDevice, VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE,
                                         VK_FILTER_LINEAR, VK_FILTER_LINEAR,
                                         VK_SAMPLER_MIPMAP_MODE_NEAREST, 1.0f, 1);
}

VkRenderer::VkRenderer(ModelContext* modelContext, int textureCount)
    : mDevice(VulkanManager::GetInstance()->GetDevice())
    , mPhysicalDevice(VulkanManager::GetInstance()->GetPhysicalDevice()) {
    mColorFormat = VulkanManager::GetInstance()->GetSwapchainManager()->GetSwapchainImageFormat();
    mTextures.resize(textureCount < 1 ? 1 : textureCount);

    // UBO 动态偏移对齐
    VkPhysicalDeviceProperties props;
    vkGetPhysicalDeviceProperties(mPhysicalDevice, &props);
    uint32_t align = (uint32_t)props.limits.minUniformBufferOffsetAlignment;
    mUboStride = (uint32_t)((sizeof(UBO) + align - 1) / align * align);

    // 帧级 arena：VBO 4MB / IBO 2MB / UBO (4096 draws)
    constexpr VkDeviceSize VB_SIZE = 4 << 20, IB_SIZE = 2 << 20;
    VkDeviceSize UB_SIZE = (VkDeviceSize)mUboStride * 4096;
    VkUtil::CreateBuffer(mDevice, mPhysicalDevice, VB_SIZE, VK_BUFFER_USAGE_VERTEX_BUFFER_BIT,
                         VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
                         &mVbBuffer, &mVbMemory);
    VkUtil::CreateBuffer(mDevice, mPhysicalDevice, IB_SIZE, VK_BUFFER_USAGE_INDEX_BUFFER_BIT,
                         VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
                         &mIbBuffer, &mIbMemory);
    VkUtil::CreateBuffer(mDevice, mPhysicalDevice, UB_SIZE, VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT,
                         VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
                         &mUboBuffer, &mUboMemory);
    vkMapMemory(mDevice, mVbMemory, 0, VK_WHOLE_SIZE, 0, &mVbMapped);
    vkMapMemory(mDevice, mIbMemory, 0, VK_WHOLE_SIZE, 0, &mIbMapped);
    vkMapMemory(mDevice, mUboMemory, 0, VK_WHOLE_SIZE, 0, &mUboMapped);

    // descriptor pool
    VkDescriptorPoolSize poolSizes[2]{};
    poolSizes[0].type = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
    poolSizes[0].descriptorCount = (uint32_t)(mTextures.size() * 2 + 4);
    poolSizes[1].type = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER_DYNAMIC;
    poolSizes[1].descriptorCount = (uint32_t)(mTextures.size() + 4);
    VkDescriptorPoolCreateInfo poolInfo{};
    poolInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
    poolInfo.poolSizeCount = 2;
    poolInfo.pPoolSizes = poolSizes;
    poolInfo.maxSets = (uint32_t)(mTextures.size() + 4);
    vkCreateDescriptorPool(mDevice, &poolInfo, nullptr, &mDescPool);

    // 全局 UBO 描述符只作为 pool 兜底；每纹理 set 在 setTexture 中分配并写入 b1/b2

    createLayoutsAndPipelines();
    createMaskTarget();

    mClipManager = std::make_shared<ClippingManagerVulkan>(*this);
    std::vector<MeshContext*> rawDrawCtxs;
    rawDrawCtxs.reserve(modelContext->mDrawContextList.size());
    for (auto& ctx : modelContext->mDrawContextList)
        rawDrawCtxs.push_back(ctx.get());
    mClipManager->init(modelContext, modelContext->mDrawDataList, rawDrawCtxs);
}

VkRenderer::~VkRenderer() {
    vkDeviceWaitIdle(mDevice);

    for (auto& t : mTextures) {
        if (!t.valid)
            continue;
        if (t.sampler) vkDestroySampler(mDevice, t.sampler, nullptr);
        if (t.view) vkDestroyImageView(mDevice, t.view, nullptr);
        if (t.image) vkDestroyImage(mDevice, t.image, nullptr);
        if (t.memory) vkFreeMemory(mDevice, t.memory, nullptr);
    }
    if (mMaskSampler) vkDestroySampler(mDevice, mMaskSampler, nullptr);
    if (mMaskView) vkDestroyImageView(mDevice, mMaskView, nullptr);
    if (mMaskImage) vkDestroyImage(mDevice, mMaskImage, nullptr);
    if (mMaskMemory) vkFreeMemory(mDevice, mMaskMemory, nullptr);

    if (mVbMapped) vkUnmapMemory(mDevice, mVbMemory);
    if (mIbMapped) vkUnmapMemory(mDevice, mIbMemory);
    if (mUboMapped) vkUnmapMemory(mDevice, mUboMemory);
    if (mVbBuffer) vkDestroyBuffer(mDevice, mVbBuffer, nullptr);
    if (mIbBuffer) vkDestroyBuffer(mDevice, mIbBuffer, nullptr);
    if (mUboBuffer) vkDestroyBuffer(mDevice, mUboBuffer, nullptr);
    if (mVbMemory) vkFreeMemory(mDevice, mVbMemory, nullptr);
    if (mIbMemory) vkFreeMemory(mDevice, mIbMemory, nullptr);
    if (mUboMemory) vkFreeMemory(mDevice, mUboMemory, nullptr);

    if (mDescPool) vkDestroyDescriptorPool(mDevice, mDescPool, nullptr);
    if (mPipelineLayout) vkDestroyPipelineLayout(mDevice, mPipelineLayout, nullptr);
    for (auto& p : mPipelines)
        if (p) vkDestroyPipeline(mDevice, p, nullptr);
    if (mDescLayout) vkDestroyDescriptorSetLayout(mDevice, mDescLayout, nullptr);
}

void VkRenderer::setMatrix(const float m[16]) {
    std::memcpy(mMatrix4x4.data(), m, 16 * sizeof(float));
}

void VkRenderer::setClipMatrix(const float m[16]) {
    std::memcpy(mClipMatrix.data(), m, 16 * sizeof(float));
}

void VkRenderer::resize(int w, int h) {
    (void)w;
    (void)h;
}

void VkRenderer::setTexture(int no, VkImage image, VkDeviceMemory memory, VkImageView view,
                            VkSampler sampler) {
    if (no < 0)
        return;
    if (no >= (int)mTextures.size())
        mTextures.resize(no + 1);
    auto& t = mTextures[no];
    t.image = image;
    t.memory = memory;
    t.view = view;
    t.sampler = sampler;
    t.valid = true;

    if (no >= (int)mDescSets.size())
        mDescSets.resize(no + 1, VK_NULL_HANDLE);
    if (mDescSets[no] == VK_NULL_HANDLE) {
        VkDescriptorSetAllocateInfo allocInfo{};
        allocInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
        allocInfo.descriptorPool = mDescPool;
        allocInfo.descriptorSetCount = 1;
        allocInfo.pSetLayouts = &mDescLayout;
        if (vkAllocateDescriptorSets(mDevice, &allocInfo, &mDescSets[no]) != VK_SUCCESS) {
            LOGE("failed to allocate descriptor set for texture %d", no);
            return;
        }
    }

    VkDescriptorBufferInfo uboInfo{mUboBuffer, 0, sizeof(UBO)};
    VkDescriptorImageInfo imgInfo{sampler, view, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL};
    VkDescriptorImageInfo maskInfo{mMaskSampler, mMaskView, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL};

    VkWriteDescriptorSet writes[3]{};
    writes[0].sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
    writes[0].dstSet = mDescSets[no];
    writes[0].dstBinding = 0;
    writes[0].descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER_DYNAMIC;
    writes[0].descriptorCount = 1;
    writes[0].pBufferInfo = &uboInfo;
    writes[1].sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
    writes[1].dstSet = mDescSets[no];
    writes[1].dstBinding = 1;
    writes[1].descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
    writes[1].descriptorCount = 1;
    writes[1].pImageInfo = &imgInfo;
    writes[2].sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
    writes[2].dstSet = mDescSets[no];
    writes[2].dstBinding = 2;
    writes[2].descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
    writes[2].descriptorCount = 1;
    writes[2].pImageInfo = &maskInfo;
    vkUpdateDescriptorSets(mDevice, 3, writes, 0, nullptr);
}

void VkRenderer::imageBarrier(VkCommandBuffer cmd, VkImage image, VkImageLayout oldLayout,
                              VkImageLayout newLayout) {
    VkImageMemoryBarrier barrier{};
    barrier.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
    barrier.oldLayout = oldLayout;
    barrier.newLayout = newLayout;
    barrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
    barrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
    barrier.image = image;
    barrier.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
    barrier.subresourceRange.levelCount = 1;
    barrier.subresourceRange.layerCount = 1;

    VkPipelineStageFlags srcStage, dstStage;
    VkAccessFlags srcAccess, dstAccess;
    if (newLayout == VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL) {
        srcStage = VK_PIPELINE_STAGE_ALL_COMMANDS_BIT;
        dstStage = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
        srcAccess = 0;
        dstAccess = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;
    } else {   // → SHADER_READ_ONLY
        srcStage = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
        dstStage = VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT;
        srcAccess = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;
        dstAccess = VK_ACCESS_SHADER_READ_BIT;
    }
    vkCmdPipelineBarrier(cmd, srcStage, dstStage, 0, 0, nullptr, 0, nullptr, 1, &barrier);
}

static void beginRendering(VkCommandBuffer cmd, VkImageView view, uint32_t width, uint32_t height,
                           bool clear) {
    VkClearValue clearValue{};
    clearValue.color = {{0.0f, 0.0f, 0.0f, 0.0f}};

    VkRenderingAttachmentInfo colorAttachment{};
    colorAttachment.sType = VK_STRUCTURE_TYPE_RENDERING_ATTACHMENT_INFO;
    colorAttachment.imageView = view;
    colorAttachment.imageLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
    colorAttachment.loadOp = clear ? VK_ATTACHMENT_LOAD_OP_CLEAR : VK_ATTACHMENT_LOAD_OP_LOAD;
    colorAttachment.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
    colorAttachment.clearValue = clearValue;

    VkRenderingInfo renderingInfo{};
    renderingInfo.sType = VK_STRUCTURE_TYPE_RENDERING_INFO;
    renderingInfo.renderArea.offset = {0, 0};
    renderingInfo.renderArea.extent = {width, height};
    renderingInfo.layerCount = 1;
    renderingInfo.colorAttachmentCount = 1;
    renderingInfo.pColorAttachments = &colorAttachment;
    vkCmdBeginRendering(cmd, &renderingInfo);
}

void VkRenderer::preDraw(ModelContext* context) {
    // 帧首复位 arena（帧内所有录制共用，EndFrame 提交后等待 GPU 空闲，覆盖安全）
    mVbOffset = 0;
    mIbOffset = 0;
    mUboIndex = 0;

    if (!mClipManager || !mMaskDirty)
        return;

    // mask 脏（本帧 Update 过或首次绘制）才重渲：模型自管 mask 阶段——
    // 挂起帧主区间 → 录制 mask 区间 → 恢复帧主区间（帧流程对 mask 无感知）
    VulkanManager* mgr = VulkanManager::GetInstance();
    VkCommandBuffer cmd = mgr->GetFrameCommandBuffer();

    mgr->SuspendMainRegion();
    // mask image: 上帧 SHADER_READ → COLOR_ATTACHMENT（每帧全量重写）
    imageBarrier(cmd, mMaskImage, VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL);
    beginRendering(cmd, mMaskView, MASK_SIZE, MASK_SIZE, /*clear=*/true);
    mCurrentCmd = cmd;
    mClipManager->setupClip(context, cmd);
    mCurrentCmd = VK_NULL_HANDLE;
    vkCmdEndRendering(cmd);
    imageBarrier(cmd, mMaskImage, VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL,
                 VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL);
    mgr->ResumeMainRegion();

    mMaskDirty = false;
}

void VkRenderer::draw(ModelContext* context) {
    if (context->mOrderListFirstDrawIndex.empty())
        return;

    // 纯录制：帧主区间已由 vkBeginFrame 打开（clear 过），直接向帧命令缓冲录制
    mCurrentCmd = VulkanManager::GetInstance()->GetFrameCommandBuffer();

    for (size_t i = 0; i < context->mOrderListFirstDrawIndex.size(); i++) {
        int idx = context->mOrderListFirstDrawIndex[i];
        if (idx == ModelContext::NOT_USED_ORDER)
            continue;
        while (true) {
            auto* dd = static_cast<Mesh*>(context->mDrawDataList[idx]);
            auto* ctx = context->mDrawContextList[idx].get();
            if (ctx->mAvailable) {
                ctx->mPartsOpacity = context->mPartsContextList[ctx->mPartsIndex]->getPartsOpacity();
                dd->draw(this, context, ctx);
            }
            int next = context->mNextListDrawIndex[idx];
            if (next <= idx || next == ModelContext::NO_NEXT)
                break;
            idx = next;
        }
    }

    mCurrentCmd = VK_NULL_HANDLE;
}

void VkRenderer::drawTexture(int texNo, const std::array<float, 4>& screenColor,
                             const std::vector<int16_t>& indices, const std::vector<float>& vertices,
                             const std::vector<float>& uvs, float opacity, int compositionType,
                             const std::array<float, 4>& multiplyColor) {
    if (opacity < 0.01f && !mClipMaskMode)
        return;
    if (mCurrentCmd == VK_NULL_HANDLE)
        return;
    if (texNo < 0 || texNo >= (int)mDescSets.size() || mDescSets[texNo] == VK_NULL_HANDLE) {
        LOGE("VkRenderer: texture %d not bound", texNo);
        return;
    }

    // 程序选择需在 mClipDrawMode 被消费重置前确定
    const bool useMaskProgram = mClipDrawMode;

    float baseR = 1.0f * opacity, baseG = 1.0f * opacity, baseB = 1.0f * opacity,
          baseA = 1.0f * opacity;

    UBO u{};
    if (mClipMaskMode) {
        // Mask RENDER：matrixForMask 作 mvp，baseColor=全图 rect，通道色打包
        std::memcpy(u.mvp, mClipMatrix.data(), 64);
        u.baseColor[0] = -1.0f;
        u.baseColor[1] = -1.0f;
        u.baseColor[2] = 1.0f;
        u.baseColor[3] = 1.0f;
        u.channelFlag[0] = (mClipChannel == 0) ? 1.0f : 0;
        u.channelFlag[1] = (mClipChannel == 1) ? 1.0f : 0;
        u.channelFlag[2] = (mClipChannel == 2) ? 1.0f : 0;
        u.channelFlag[3] = (mClipChannel == 3) ? 1.0f : 0;
        u.screenColor[0] = 0;
        u.screenColor[1] = 0;
        u.screenColor[2] = 0;
        u.screenColor[3] = 0;
        u.multiplyColor[0] = 1;
        u.multiplyColor[1] = 1;
        u.multiplyColor[2] = 1;
        u.multiplyColor[3] = 0;
        u.maskFlag = 1;
    } else if (mClipDrawMode) {
        // Clipped DRAW：mask 程序，clipMatrix = mMatrixForDraw
        std::memcpy(u.mvp, mMatrix4x4.data(), 64);
        std::memcpy(u.clipMatrix, mClipMatrix.data(), 64);
        u.baseColor[0] = baseR;
        u.baseColor[1] = baseG;
        u.baseColor[2] = baseB;
        u.baseColor[3] = baseA;
        std::memcpy(u.screenColor, screenColor.data(), 16);
        std::memcpy(u.multiplyColor, multiplyColor.data(), 16);
        u.channelFlag[0] = (mClipChannel == 0) ? 1.0f : 0;
        u.channelFlag[1] = (mClipChannel == 1) ? 1.0f : 0;
        u.channelFlag[2] = (mClipChannel == 2) ? 1.0f : 0;
        u.channelFlag[3] = (mClipChannel == 3) ? 1.0f : 0;
        u.maskFlag = 0;
        mClipDrawMode = false;   // 与 GLRenderer 一致：单次生效
    } else {
        std::memcpy(u.mvp, mMatrix4x4.data(), 64);
        u.baseColor[0] = baseR;
        u.baseColor[1] = baseG;
        u.baseColor[2] = baseB;
        u.baseColor[3] = baseA;
        std::memcpy(u.screenColor, screenColor.data(), 16);
        std::memcpy(u.multiplyColor, multiplyColor.data(), 16);
        u.channelFlag[0] = 0;
        u.channelFlag[1] = 0;
        u.channelFlag[2] = 0;
        u.channelFlag[3] = 0;
        u.maskFlag = 0;
    }

    // UBO ring
    if (mUboIndex * mUboStride + sizeof(UBO) > 4096ull * mUboStride) {
        LOGE("VkRenderer: UBO arena exhausted this frame");
        return;
    }
    uint32_t uboOffset = (uint32_t)(mUboIndex * mUboStride);
    mUboIndex++;
    std::memcpy((char*)mUboMapped + uboOffset, &u, sizeof(UBO));

    // 顶点/索引 arena
    VkDeviceSize vbBytes = vertices.size() * sizeof(float);
    VkDeviceSize uvBytes = uvs.size() * sizeof(float);
    VkDeviceSize ibBytes = indices.size() * sizeof(int16_t);
    if (mVbOffset + vbBytes + uvBytes > 4ull << 20 || mIbOffset + ibBytes > 2ull << 20) {
        LOGE("VkRenderer: vertex/index arena exhausted this frame");
        return;
    }
    VkDeviceSize posOffset = mVbOffset;
    std::memcpy((char*)mVbMapped + posOffset, vertices.data(), (size_t)vbBytes);
    mVbOffset += vbBytes;
    VkDeviceSize uvOffset = mVbOffset;
    std::memcpy((char*)mVbMapped + uvOffset, uvs.data(), (size_t)uvBytes);
    mVbOffset += uvBytes;
    VkDeviceSize idxOffset = mIbOffset;
    std::memcpy((char*)mIbMapped + idxOffset, indices.data(), (size_t)ibBytes);
    mIbOffset += ibBytes;

    // 管线选择：mask pass 固定 normal 程序 + NORMAL 混合；裁剪绘制用 mask 程序
    int prog = useMaskProgram ? 1 : 0;
    int blend = mClipMaskMode ? 0 : compositionType;
    if (blend < 0 || blend > 2)
        blend = 0;
    VkPipeline pipeline = mPipelines[prog * 3 + blend];
    if (pipeline == VK_NULL_HANDLE)
        return;
    vkCmdBindPipeline(mCurrentCmd, VK_PIPELINE_BIND_POINT_GRAPHICS, pipeline);

    // 动态状态：负高度 viewport 保持 GL NDC/y 语义；front face 与 GL_CW/GL_CCW 对应
    VulkanManager* mgr = VulkanManager::GetInstance();
    uint32_t vw, vh;
    bool maskPass = mClipMaskMode;
    if (maskPass) {
        vw = MASK_SIZE;
        vh = MASK_SIZE;
    } else {
        VkExtent2D ext = mgr->GetSwapchainManager()->GetExtent();
        vw = ext.width;
        vh = ext.height;
    }
    VkViewport viewport{0.0f, (float)vh, (float)vw, -(float)vh, 0.0f, 1.0f};
    VkRect2D scissor{{0, 0}, {vw, vh}};
    vkCmdSetViewport(mCurrentCmd, 0, 1, &viewport);
    vkCmdSetScissor(mCurrentCmd, 0, 1, &scissor);
    vkCmdSetFrontFace(mCurrentCmd,
                      maskPass ? VK_FRONT_FACE_COUNTER_CLOCKWISE : VK_FRONT_FACE_CLOCKWISE);
    vkCmdSetCullMode(mCurrentCmd, mCulling ? VK_CULL_MODE_BACK_BIT : VK_CULL_MODE_NONE);

    vkCmdBindDescriptorSets(mCurrentCmd, VK_PIPELINE_BIND_POINT_GRAPHICS, mPipelineLayout, 0, 1,
                            &mDescSets[texNo], 1, &uboOffset);

    VkBuffer vbuffers[2] = {mVbBuffer, mVbBuffer};
    VkDeviceSize voffsets[2] = {posOffset, uvOffset};
    vkCmdBindVertexBuffers(mCurrentCmd, 0, 2, vbuffers, voffsets);
    vkCmdBindIndexBuffer(mCurrentCmd, mIbBuffer, idxOffset, VK_INDEX_TYPE_UINT16);
    vkCmdDrawIndexed(mCurrentCmd, (uint32_t)indices.size(), 1, 0, 0, 0);
}

}   // namespace V2
}   // namespace Live2D
