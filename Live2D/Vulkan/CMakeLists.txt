cmake_minimum_required(VERSION 3.16)

project(live2d-vulkan CXX)

find_package(Vulkan REQUIRED)

set(VULKAN_LIB_TARGET LVulkan)
add_library(${VULKAN_LIB_TARGET} STATIC
    src/VulkanManager.cpp
    src/SwapchainManager.cpp
    src/VkUtil.cpp
)
add_library(Live2D::Vulkan ALIAS ${VULKAN_LIB_TARGET})

target_include_directories(${VULKAN_LIB_TARGET} PUBLIC ${CMAKE_CURRENT_LIST_DIR}/include)
target_link_libraries(${VULKAN_LIB_TARGET} PUBLIC Common Vulkan::Vulkan)

if(MSVC)
    target_compile_options(${VULKAN_LIB_TARGET} PRIVATE /utf-8 /W4)
endif()
