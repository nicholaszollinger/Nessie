// // Vulkan_GLFW.h
// #pragma once
// #define GLFW_INCLUDE_VULKAN
// #include <GLFW/glfw3.h>
// #include "VulkanCore.h"
//
// namespace nes::vulkan::glfw
// {
//     inline vk::SurfaceKHR CreateSurface(vk::Instance instance, GLFWwindow* pWindow)
//     {
//         NES_ASSERT(pWindow);
//         NES_ASSERT(instance != nullptr);
//
//         VkSurfaceKHR surface;
//         const VkResult result = glfwCreateWindowSurface(instance, pWindow, nullptr, &surface);
//         if (result != VK_SUCCESS)
//             NES_FATAL(nes::vulkan::kLogTag, "Failed to create GLFW Surface!");
//         
//         return surface;
//     }
//
//     inline std::vector<const char*> GetRequiredExtensions()
//     {
//         uint32 glfwExtensionCount = 0;
//         const char** glfwExtensions = glfwGetRequiredInstanceExtensions(&glfwExtensionCount);
//         std::vector<const char*> extensions(glfwExtensions, glfwExtensions + glfwExtensionCount);
//         return extensions;
//     }
//
//     inline void GetFrameBufferSize(GLFWwindow* pWindow, int& width, int& height)
//     {
//         glfwGetFramebufferSize(pWindow, &width, &height);
//     }
// }