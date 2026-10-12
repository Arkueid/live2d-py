# BY DEEPSEEK

# 源文件路径
set(INPUT_FILE ${LIVE2D_ROOT}/V3/Framework/src/Rendering/Vulkan/CubismRenderer_Vulkan.cpp)
# 备份文件路径
set(BACKUP_FILE ${INPUT_FILE}.bak)

if(NOT EXISTS "${BACKUP_FILE}")
  # 一次性读入整个文件，保留所有格式
  file(READ "${INPUT_FILE}" FILE_CONTENTS)

  set(ORIGINAL_BLOCK
"VkShaderModule CubismPipeline_Vulkan::PipelineResource::CreateShaderModule(VkDevice device, csmString filename)
{
    std::ifstream file(filename.GetRawString(), std::ios::ate | std::ios::binary);

    if (!file.is_open())
    {
        CubismLogError(\"failed to open file!\");
        return NULL;
    }

    csmInt32 fileSize = (csmInt32)file.tellg();
    csmVector<char> buffer(fileSize);

    file.seekg(0);
    file.read(buffer.GetPtr(), fileSize);
    file.close();

    VkShaderModuleCreateInfo createInfo{};
    createInfo.sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;
    createInfo.codeSize = fileSize;
    createInfo.pCode = reinterpret_cast<const csmUint32*>(buffer.GetPtr());

    VkShaderModule shaderModule;
    if (vkCreateShaderModule(device, &createInfo, nullptr, &shaderModule) != VK_SUCCESS)
    {
        CubismLogError(\"failed to create shader module!\");
        return NULL;
    }

    return shaderModule;
}")

  # 替换后: 经 LoadFileFunction 加载，启用 SHADER_DIR 重定向
  set(PATCHED_BLOCK
"VkShaderModule CubismPipeline_Vulkan::PipelineResource::CreateShaderModule(VkDevice device, csmString filename)
{
    csmLoadFileFunction fileLoader = Csm::CubismFramework::GetLoadFileFunction();
    csmReleaseBytesFunction bytesReleaser = Csm::CubismFramework::GetReleaseBytesFunction();

    if (!fileLoader || !bytesReleaser)
    {
        CubismLogError(\"File loader is not set.\");
        return NULL;
    }

    csmSizeInt fileSize = 0;
    csmByte* buffer = fileLoader(filename.GetRawString(), &fileSize);
    if (buffer == NULL)
    {
        CubismLogError(\"failed to open file!\");
        return NULL;
    }

    VkShaderModuleCreateInfo createInfo{};
    createInfo.sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;
    createInfo.codeSize = fileSize;
    createInfo.pCode = reinterpret_cast<const csmUint32*>(buffer);

    VkShaderModule shaderModule;
    if (vkCreateShaderModule(device, &createInfo, nullptr, &shaderModule) != VK_SUCCESS)
    {
        CubismLogError(\"failed to create shader module!\");
        bytesReleaser(buffer);
        return NULL;
    }

    bytesReleaser(buffer);
    return shaderModule;
}")

  # 检查目标块是否存在
  string(FIND "${FILE_CONTENTS}" "${ORIGINAL_BLOCK}" POS)
  if(POS EQUAL -1)
    message(WARNING "Patch not applied: pattern not found in ${INPUT_FILE}")
  else()
    # 原地替换，其余内容一个字节都不动
    string(REPLACE "${ORIGINAL_BLOCK}" "${PATCHED_BLOCK}" FILE_CONTENTS "${FILE_CONTENTS}")

    # 备份原文件
    file(COPY_FILE "${INPUT_FILE}" "${BACKUP_FILE}")

    # 写回
    file(WRITE "${INPUT_FILE}" "${FILE_CONTENTS}")
    message(STATUS "Successfully patched ${INPUT_FILE}")
  endif()
endif()
