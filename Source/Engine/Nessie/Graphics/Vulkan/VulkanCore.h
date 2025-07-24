//// VulkanCore.h
//#pragma once
//
//#include "Nessie/Core/Config.h"
//#include "Nessie/Debug/Assert.h"
//
//NES_SUPPRESS_WARNINGS_BEGIN
//// 28251: Disabling the inconsistent naming warning in vulkan.hpp
//// 4996:  Disable strncpy warning in vulkan_structs.hpp
//NES_MSVC_SUPPRESS_WARNING(28251 4996)
//#include <vulkan/vulkan.hpp>
//NES_SUPPRESS_WARNINGS_END
//
//#include <vulkan/vk_enum_string_helper.h>
//#include "VkBootstrap.h"
//
//#define NES_VULKAN_NAMESPACE_NAME nes::vulkan
//#define NES_VULKAN_NAMESPACE_BEGIN namespace NES_VULKAN_NAMESPACE_NAME {
//#define NES_VULKAN_NAMESPACE_END }
//
//NES_VULKAN_NAMESPACE_BEGIN
//    /// Log Tag for Vulkan Messages
//    NES_DEFINE_LOG_TAG(kLogTag, "Vulkan", Warn);
//NES_VULKAN_NAMESPACE_END
//
///// Vulkan Log Macros
//#define NES_VULKAN_INFO(...) NES_LOG(NES_VULKAN_NAMESPACE_NAME::kLogTag, __VA_ARGS__)
//#define NES_VULKAN_WARN(...) NES_WARN(NES_VULKAN_NAMESPACE_NAME::kLogTag, __VA_ARGS__)
//#define NES_VULKAN_ERROR(...) NES_ERROR(NES_VULKAN_NAMESPACE_NAME::kLogTag, __VA_ARGS__)
//#define NES_VULKAN_FATAL(...) NES_FATAL(NES_VULKAN_NAMESPACE_NAME::kLogTag, __VA_ARGS__)
//
//NES_VULKAN_NAMESPACE_BEGIN
//
//inline void VulkanCheckResult(const char* expression, const VkResult result)
//{
//    if (result != VK_SUCCESS)
//    {
//        if (result == VK_ERROR_DEVICE_LOST)
//        {
//            using namespace std::chrono_literals;
//            std::this_thread::sleep_for(3s);
//            // [TODO]: Dump GPU info.
//        }
//        NES_VULKAN_FATAL("{} failed! Vulkan Error: '{}'" , expression, string_VkResult(result));
//    }
//}
//
//inline void VulkanCheckResult(const char* expression, const vk::Result result)
//{
//    if (result != vk::Result::eSuccess)
//    {
//        if (result == vk::Result::eErrorDeviceLost)
//        {
//            using namespace std::chrono_literals;
//            std::this_thread::sleep_for(3s);
//            // [TODO]: Dump GPU info.
//        }
//        NES_VULKAN_FATAL("{} failed! Vulkan Error: '{}'" , expression, vk::to_string(result));
//    }
//}
//
//NES_VULKAN_NAMESPACE_END
//
////----------------------------------------------------------------------------------------------------
///// @brief : Wrapper for a CRITICAL vulkan function call that returns vk::result. If the call fails, this will
/////    log the error message and abort.
////----------------------------------------------------------------------------------------------------
//#define NES_VULKAN_MUST_PASS(expression)                                                       \
//do                                                                                             \
//{                                                                                              \
//    vk::Result exprResult = expression;                                                        \
//    NES_VULKAN_NAMESPACE_NAME::VulkanCheckResult(#expression, exprResult);                                   \
//} while (0)
//
////----------------------------------------------------------------------------------------------------
///// @brief : Wrapper for a CRITICAL vulkan function call that returns VkResult. If the call fails, this will
/////    log the error message and abort.
////----------------------------------------------------------------------------------------------------
//#define NES_VULKAN_C_MUST_PASS(expression)                                                     \
//do                                                                                             \
//{                                                                                              \
//    VkResult exprResult = expression;                                                          \
//    NES_VULKAN_NAMESPACE_NAME::VulkanCheckResult(#expression, exprResult);                                   \
//} while (0)
//
