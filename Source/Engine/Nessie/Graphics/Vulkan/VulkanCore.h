// VulkanCore.h
#pragma once
#include <vector>
#include "Nessie/Core/Config.h"
#include "Nessie/Debug/Assert.h"

NES_SUPPRESS_WARNINGS_BEGIN
// 28251: Disabling the inconsistent naming warning in vulkan.hpp
// 4996:  Disable strncpy warning in vulkan_structs.hpp
NES_MSVC_SUPPRESS_WARNING(28251 4996)
#include <vulkan/vulkan_core.h>
// Forward declare VmaAllocation so we don't need to pull in vk_mem_alloc.h
VK_DEFINE_HANDLE(VmaAllocation);

NES_SUPPRESS_WARNINGS_END

#define NES_VULKAN_NAMESPACE_NAME nes

namespace nes
{
    /// Log Tag for Vulkan Messages
    NES_DEFINE_LOG_TAG(kVulkanLogTag, "Vulkan", Warn);
}

//----------------------------------------------------------------------------------------------------
/// @brief : If the vkExpression doesn't evaluate to VK_SUCCESS, it will exit the program.
///	@param renderDevice : Reference to the render device to report any errors. 
///	@param vkExpression : Expression that should evaluate to a VkResult.
//----------------------------------------------------------------------------------------------------
#define NES_VK_MUST_PASS(renderDevice, vkExpression)                                \
do                                                                              \
{                                                                               \
    const VkResult exprResult = vkExpression;                                   \
    (renderDevice).CheckResult(exprResult, #vkExpression, __FILE__, __LINE__);  \
    if (exprResult < 0)                                                         \
    {                                                                           \
       NES_BREAKPOINT;                                                          \
    }                                                                           \
} while (0)

//----------------------------------------------------------------------------------------------------
/// @brief : If the vkExpression doesn't evaluate to VK_SUCCESS, it will report an error message,
///     and then return the equivalent EGraphicsResult value.
///	@param renderDevice : Reference to the render device to report any errors. 
///	@param vkExpression : Expression that should evaluate to a VkResult.
//----------------------------------------------------------------------------------------------------
#define NES_VK_FAIL_RETURN(renderDevice, vkExpression)  \
do                                                      \
{                                                       \
    const VkResult exprResult = vkExpression;           \
    if (exprResult < 0)                                 \
    {                                                   \
        return (renderDevice).ReportOnError(exprResult, #vkExpression, __FILE__, __LINE__); \
    }                                                   \
} while (0)

//----------------------------------------------------------------------------------------------------
/// @brief : If the vkExpression doesn't evaluate to VK_SUCCESS, it will report an error message,
///     and then return the equivalent EGraphicsResult value.
///	@param renderDevice : Reference to the render device to report any errors. 
///	@param vkExpression : Expression that should evaluate to a VkResult.
//----------------------------------------------------------------------------------------------------
#define NES_VK_FAIL_RETURN_VOID(renderDevice, vkExpression)  \
do                                                      \
{                                                       \
    const VkResult exprResult = vkExpression;           \
    if (exprResult < 0)                                 \
    {                                                   \
        (renderDevice).ReportOnError(exprResult, #vkExpression, __FILE__, __LINE__); \
        return;                                         \
    }                                                   \
} while (0)

//----------------------------------------------------------------------------------------------------
/// @brief : If the vkExpression doesn't evaluate to VK_SUCCESS, it will report an error message. You can
///     use this to set an EGraphicsResult variable. Ex: EGraphicsResult result = NES_VK_FAIL_REPORT(...).
///	@param renderDevice : Reference to the render device to report any errors. 
///	@param vkExpression : Expression that should evaluate to a VkResult.
//----------------------------------------------------------------------------------------------------
#define NES_VK_FAIL_REPORT(renderDevice, vkExpression) (renderDevice).ReportOnError(vkExpression, #vkExpression, __FILE__, __LINE__)