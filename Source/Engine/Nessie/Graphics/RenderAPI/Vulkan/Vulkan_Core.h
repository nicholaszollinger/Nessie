// Vulkan_Core.h
#pragma once
#include "Nessie/Core/Config.h"
#include "Nessie/Debug/Assert.h"

NES_SUPPRESS_WARNINGS_BEGIN
// 28251: Disabling the inconsistent naming warning in vulkan.hpp
// 4996:  Disable strncpy warning in vulkan_structs.hpp
NES_MSVC_SUPPRESS_WARNING(28251 4996)
#include <vulkan/vulkan.hpp>
NES_SUPPRESS_WARNINGS_END
#include <vulkan/vk_enum_string_helper.h>
#include "VkBootstrap.h"

#if defined(_DEBUG)
    #define NES_VULKAN_DEBUG 1
#else
    #define NES_VULKAN_DEBUG 0
#endif

namespace nes::vulkan
{
    NES_DEFINE_LOG_TAG(kLogTag, "Vulkan", Warn);
}

 //----------------------------------------------------------------------------------------------------
 /// @brief : Wrapper for a CRITICAL vulkan function call that returns VkResult. If the call fails, this will
 ///    log the error message and abort.
 //----------------------------------------------------------------------------------------------------
 #define NES_VULKAN_MUST_PASS(expression)                                                        \
 do                                                                                              \
 {                                                                                               \
     vk::Result error = expression;                                                                   \
     if (error != vk::Result::eSuccess)                                                                                     \
     {                                                                                              \
        NES_FATAL(nes::vulkan::kLogTag, "{} failed! Vulkan Error: {}", #expression, vk::to_string(error));     \
     }                                                                                              \
 } while (0)

namespace nes::vulkan
{
    //----------------------------------------------------------------------------------------------------
    /// @brief : Set a debug name for a vulkan resource object. 
    //----------------------------------------------------------------------------------------------------
    inline void SetDebugObjectName(const vk::Device device, const vk::ObjectType type, const std::string& name, const void* pHandle)
    {
        vk::DebugUtilsObjectNameInfoEXT nameInfo;
        nameInfo.objectType = type;
        nameInfo.pObjectName = name.c_str();
        nameInfo.objectHandle = reinterpret_cast<uint64>(pHandle);
        nameInfo.pNext = nullptr;

        NES_VULKAN_MUST_PASS(device.setDebugUtilsObjectNameEXT(&nameInfo));
    }
}