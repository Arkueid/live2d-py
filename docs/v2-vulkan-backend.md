# V2（v2cpp）增加 Vulkan 渲染后端

## Context

V3 的 Vulkan 后端已在 Live2DTest 上跑通（`Model::Draw` 自包含完成 acquire→draw→present）。V2 库（Cubism 2.x 移植版，`Live2D::V2`）目前只有 OpenGL 渲染（`GLRenderer` + GLSL 1.20 + FBO 裁剪遮罩），Python Wrapper 加载 v2 模型（kasumi、haru 等）时无法走 Vulkan。

目标：为 V2 增加与 V3 对等的 Vulkan 后端，并在 Live2DTest 上用 v2 模型验证。

**用户决策**：新建独立 Vulkan 支持库（类似 glad 的地位），V2/V3/tests 共同链接，不拷贝、不入 Common。
**硬约束**：`Live2D/V3/Framework` 与 Core（官方 SDK，git-ignore）一行不能改。

**已核实事实**：

* `Live2D/CMakeLists.txt` L47 include V3.cmake（`add_compile_definitions(CSM_TARGET_VULKAN=1)`，目录作用域）早于 L48 include V2.cmake → **V2 源文件已能看到** **`CSM_TARGET_VULKAN`**，后端选择与 V3 一样用编译期宏。

* `VulkanManager`/`SwapchainManager` 仅依赖 Framework 的 `Type/csmVector.hpp` 等 Csm 类型 + Common 的 Log；`VulkanManager.cpp` 对 `CubismRenderer_Vulkan.hpp` 的 include 是死引用 → 可剥离 Framework 依赖。

* V2 渲染接缝是具体类：`Mesh.hpp` L29 `void draw(GLRenderer* dp, ...)`；`V2/Model.hpp` 持有 `std::unique_ptr<GLRenderer>`。

## 实施步骤

### 1. 新库 `Live2D::Vulkan`（LVulkan）

* 新建 `Live2D/Vulkan/{include,src}`，把以下文件**移动**进去（保持全局命名空间，V3 调用方零改动）：

  * `Live2D/V3/include/V3/VulkanManager.hpp` → `Live2D/Vulkan/include/VulkanManager.hpp`

  * `Live2D/V3/include/V3/SwapchainManager.hpp` → `Live2D/Vulkan/include/SwapchainManager.hpp`

  * `Live2D/V3/src/VulkanManager.cpp`、`Live2D/V3/src/SwapchainManager.cpp` → `Live2D/Vulkan/src/`

  * 仅改动：`Csm::csmVector<T>`→`std::vector<T>`、`Csm::csmUint32/csmInt32`→`uint32_t/int32_t`、删除 Framework 死 include。

* 新建 `Live2D/Vulkan/include/VkHelpers.hpp` + `src/VkHelpers.cpp`（namespace `VkUtil`）：`CreateBuffer/CreateImage/TransitionImageLayout/CopyBufferToImage/GenerateMipmaps/CreateImageView/CreateSampler`，逻辑照抄 `Live2D/V3/src/LAppTextureManager.cpp` L37–215 的 staging→copy→mipmap 模式。V3 的 LAppTextureManager 不动（后续可选重构去重）。

* 新建 `Live2D/Vulkan/CMakeLists.txt`：`add_library(LVulkan STATIC)`，`find_package(Vulkan REQUIRED)`，PUBLIC 链接 `${Vulkan_LIBRARIES}` + `Common`，PUBLIC include 自身 `include/` 与 Vulkan SDK include；`add_library(Live2D::Vulkan ALIAS LVulkan)`。

* `Live2D/CMakeLists.txt`：L45 后插 `add_subdirectory(Vulkan)`；L52 接口加 `Live2D::Vulkan`。

* `Live2D/V3/src/CMakeLists.txt`：源列表移除两个 .cpp；`Live2D/V3/cmake/V3.cmake`：`V3` PUBLIC 链接加 `Live2D::Vulkan`。

### 2. V2 渲染接缝抽象

* 新建 `Live2D/V2/include/V2/Core/IRenderer.hpp`：抽象基类，覆盖 GLRenderer 公开面（`setMatrix/setClipMatrix/preDraw/draw/drawTexture/setTexture/setupDraw/endDraw/resize` + clip 模式访问器）。

* `GLRenderer : public IRenderer`（`mClipChannel/mClipMaskMode/mClipDrawMode` 收为私有+访问器，`ClippingManagerOpenGL.cpp` L275–278 相应微调）。

* `Mesh.hpp/cpp`：`draw(GLRenderer*)`→`draw(IRenderer*)`；`V2/Model.hpp`：`mRenderer` 改 `std::unique_ptr<IRenderer>`。

### 3. VkRenderer + ClippingManagerVulkan

* 新建 `Live2D/V2/include/V2/Core/VkRenderer.hpp`、`Live2D/V2/src/Core/VkRenderer.cpp`，接口与 GLRenderer 镜像。

* **shader**：新建 `Live2D/V2/shaders/{normal,mask}.{vert,frag}`（GLSL 450），移植 `GLRenderer.cpp` L26–110：`attribute`→`layout(location) in`、`varying`→`out/in`、`gl_FragColor`→`layout(location=0) out`、`texture2D`→`texture`、uniform 收进一个 UBO 块；保留 texCoord.y 翻转逻辑。

* **uniform**：mask 管线合计 192B（2×mat4+4×vec4）超 push constant 128B 下限 → host-visible UBO 环形缓冲 + `UNIFORM_BUFFER_DYNAMIC`，每 draw 一个 dynamic offset。

* **管线**：单 descriptor set layout（b0=动态UBO, b1=s\_texture0, b2=s\_texture1）；2 shader × 3 混合 = 6 条 pipeline（动态渲染，格式取 `VulkanManager::GetImageFormat()`）；混合因子照抄 `GLRenderer.cpp` L424–458；动态 viewport/scissor；**负视口高度**保持 GL NDC 语义；culling 用 `vkCmdSetCullModeEXT`（设备已要求 extended\_dynamic\_state）。

* **顶点缓冲**：帧级环形 VBO/EBO（host-visible），帧首复位；禁止每 draw 建毁 VkBuffer。

* **descriptor**：`setTexture` 接收 `VkImageView+sampler`，每纹理一个 set；模型析构 `vkResetDescriptorPool`。

* **裁剪遮罩**：新建 `ClippingManagerVulkan.{hpp,cpp}`——`ClippingManagerOpenGL.cpp` 的裁剪数学（L97–234）逐行照抄；FBO 换成 256×256 RGBA8 offscreen image（COLOR\_ATTACHMENT|SAMPLED），mask pass（动态渲染 clear）→ 屏障 → 主 pass 采样；屏障时序照 V3 模式。

### 4. V2::Model 接入

* `Live2D/V2/src/Model.cpp` `CreateRenderer`（L716）：`#ifdef CSM_TARGET_VULKAN` → `VulkanManager::IsInitialized()` 则建 VkRenderer，纹理用 stb\_image 读像素 + `VkUtil` staging/mipmap 建 image/view/sampler；未初始化则告警跳过（保 `--smoke` 无头）。`#else` 保持 GL 现状。

* `Draw()`（L404）Vulkan 分支镜像 V3 `Model::Draw` 帧结构：UpdateDrawFrame → clear/SetRenderTarget → preDraw/draw → 转 PRESENT\_SRC\_KHR → PostDraw。GL 分支不动。

### 5. V2 shader 构建

* 新建 `Live2D/V2/cmake/V2Shaders.cmake`（glslc 编译到 build 树，`V2Shaders` target），`set_target_properties(V2 PROPERTIES LIVE2D_SHADER_DIR <spv目录>)`；`V2.cmake` 尾部 include + `target_link_libraries(V2 PUBLIC Live2D::Vulkan)` + `add_dependencies(V2 V2Shaders)`。

* `tests/v2/CMakeLists.txt`：POST\_BUILD 拷贝 `$<TARGET_PROPERTY:V2,LIVE2D_SHADER_DIR>` → exe 旁 `V2Shaders/`；VkRenderer 按 CWD 相对路径 `V2Shaders/*.spv` 加载（与 FrameworkShaders 约定一致，main.cpp 已 chdir 到 exe 目录）。

### 6. Live2DTest 验证

* `tests/v2/main.cpp`：加模型版本嗅探（简化 `Wrapper/PyModel.cpp` L30–77 `CheckVersion`）：路径指向 v2 json → 实例化 `Live2D::V2::Model`，v3 → 现状；共用同一 GLFW 窗口与同一个 VulkanManager 单例；无参数默认行为不变（v3 llny 回归）。

* 测试模型：`Resources/v2/haru/haru.model.json`（资源齐全）。

## 修改/新增文件清单

| 类型 | 文件                                                                                                                                                                                                                                                                                                                           |
| -- | ---------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------- |
| 新建 | `Live2D/Vulkan/{CMakeLists.txt,include/{VulkanManager,SwapchainManager,VkHelpers}.hpp,src/{VulkanManager,SwapchainManager,VkHelpers}.cpp}`                                                                                                                                                                                   |
| 新建 | `Live2D/V2/include/V2/Core/{IRenderer,VkRenderer,ClippingManagerVulkan}.hpp`、`Live2D/V2/src/Core/{VkRenderer,ClippingManagerVulkan}.cpp`、`Live2D/V2/shaders/{normal,mask}.{vert,frag}`、`Live2D/V2/cmake/V2Shaders.cmake`                                                                                                     |
| 移动 | V3 的 VulkanManager/SwapchainManager 4 个文件 → Live2D/Vulkan                                                                                                                                                                                                                                                                    |
| 修改 | `Live2D/CMakeLists.txt`、`Live2D/V3/cmake/V3.cmake`、`Live2D/V3/src/CMakeLists.txt`、`Live2D/V2/cmake/V2.cmake`、`Live2D/V2/src/CMakeLists.txt`、`Live2D/V2/{include/V2/Core/{GLRenderer,Mesh}.hpp,src/Core/{GLRenderer,Mesh,ClippingManagerOpenGL}.cpp,include/V2/Model.hpp,src/Model.cpp}`、`tests/v2/{main.cpp,CMakeLists.txt}` |
| 不动 | `Live2D/V3/Framework/**`、Core、V3 的 Model.cpp/LAppTextureManager.cpp（仅 include 路径随迁移自动生效）                                                                                                                                                                                                                                     |

## 风险与对策

1. push constant 超 128B → 动态 UBO。
2. mask pass 屏障遗漏 → 照 V3 SubmitCommand 时序；CSM\_DEBUG 开 validation 验证。
3. 每 draw 建毁 VkBuffer 的在飞帧生命周期 → 帧级环形缓冲。
4. 搬迁后 V3 漏改链接 → 链接错误即刻暴露。

## 验证

1. VS dev shell 构建：`cmake --build build --target Live2DTest -j 16`（ninja 自动重配）。
2. v2 模型窗口渲染：`.\build\tests\v2\Live2DTest.exe Resources\v2\haru\haru.model.json` → haru 正常显示、动作/呼吸正常、ESC 退出无 validation 报错。
3. v3 回归：`.\build\tests\v2\Live2DTest.exe`（默认 llny）渲染不破坏。
4. 无头回归：`Live2DTest.exe --smoke` 全绿（v2/v3 路径均不初始化 Vulkan 时不崩）。

