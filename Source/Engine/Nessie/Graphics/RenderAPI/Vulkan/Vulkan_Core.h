// Vulkan_Core.h
#pragma once
#include "Nessie/Core/Config.h"
#include "Nessie/Debug/Assert.h"

#include <vulkan/vk_enum_string_helper.h>
#pragma warning(push)
// 28251: Disabling the inconsistent naming warning in vulkan.hpp
// 4996:  Disable strncpy warning in vulkan_structs.hpp
#pragma warning(disable:28251 4996) 
#include <vulkan/vulkan.hpp>
#pragma warning(pop)
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
 //		NOTES:
 //		
 ///		@brief : Wrapper for a CRITICAL vulkan function call that returns VkResult. If the call fails, this will
 ///         log the error message and abort.
 //----------------------------------------------------------------------------------------------------
 #define NES_VULKAN_MUST_PASS(expression)                                                        \
 do                                                                                              \
 {                                                                                               \
     VkResult error = expression;                                                                   \
     if (error)                                                                                     \
     {                                                                                              \
        NES_FATAL(nes::vulkan::kLogTag, "{} failed! Vulkan Error: {}", #expression, string_VkResult(error));     \
     }                                                                                              \
 } while (0)