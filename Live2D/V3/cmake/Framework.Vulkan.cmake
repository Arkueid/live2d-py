# ---config vulkan ---
set(FRAMEWORK_SOURCE Vulkan)
# Add Cubism Native Framework.
add_subdirectory(${LIVE2D_ROOT}/V3/Framework)
# Add rendering definition to framework.
target_compile_definitions(Framework PUBLIC ${CSM_TARGET})

find_package(Vulkan REQUIRED)
set(VULKAN_SDK_INCLUDE "$ENV{VULKAN_SDK}/Include")
if(EXISTS "${VULKAN_SDK_INCLUDE}")
    target_include_directories(Framework PUBLIC "${VULKAN_SDK_INCLUDE}")
else()
    message(FATAL_ERROR "cannot find vulkan include")
endif()

# If current compiler is mingw, add some options
if(CMAKE_SYSTEM_NAME MATCHES "Windows")
  # extra compile options for mingw on Windows
  if("${CMAKE_CXX_COMPILER_ID}" STREQUAL "GNU")
    target_compile_options(Framework PRIVATE -fpermissive -Wconversion-null)
  endif()  
endif()
# Link libraries to framework.

# include for HackProperties
target_include_directories(Framework PUBLIC ${LIVE2D_ROOT}/V3/include/V3)
target_link_libraries(Framework PUBLIC Live2DCubismCore)

if (NOT CMAKE_SYSTEM_NAME MATCHES "Android")
  target_link_libraries(Framework PUBLIC ${Vulkan_LIBRARIES})
endif()

# Compile Vulkan shaders (.spv) using the SDK's own Shaders/CMakeLists.txt
# (Framework 源码树不修改，仅通过 add_subdirectory 复用其编译规则).
# Exposes the output dir via $<TARGET_PROPERTY:Framework,LIVE2D_SHADER_DIR>.
add_subdirectory(${LIVE2D_ROOT}/V3/Framework/src/Rendering/Vulkan/Shaders
                 ${CMAKE_CURRENT_BINARY_DIR}/FrameworkVulkanShaders)
set_target_properties(FrameworkShaders PROPERTIES
  LIVE2D_SHADER_DIR "${CMAKE_CURRENT_BINARY_DIR}/FrameworkVulkanShaders/compiledShaders"
)
set_target_properties(Framework PROPERTIES
  LIVE2D_SHADER_DIR "${CMAKE_CURRENT_BINARY_DIR}/FrameworkVulkanShaders/compiledShaders"
)