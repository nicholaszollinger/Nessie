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

NES_SUPPRESS_WARNINGS_END

#define NES_VULKAN_NAMESPACE_NAME nes

namespace nes
{
    /// Log Tag for Vulkan Messages
    NES_DEFINE_LOG_TAG(kVulkanLogTag, "Vulkan", Warn);
}

//----------------------------------------------------------------------------------------------------
/// @brief : If the vkExpression doesn't evaluate to vk::Result::eSuccess, it will exit the program.
///	@param renderDevice : Reference to the render device to report any errors. 
///	@param vkExpression : Expression that should evaluate to a vk::Result.
//----------------------------------------------------------------------------------------------------
#define NES_VK_MUST_PASS(renderDevice, vkExpression)                            \
do                                                                              \
{                                                                               \
    const vk::Result exprResult = static_cast<vk::Result>(vkExpression);;       \
    (renderDevice).CheckResult(exprResult, #vkExpression, __FILE__, __LINE__);  \
    if (exprResult != vk::Result::eSuccess)                                     \
    {                                                                           \
       NES_BREAKPOINT;                                                          \
    }                                                                           \
} while (0)

//----------------------------------------------------------------------------------------------------
/// @brief : If the vkExpression doesn't evaluate to vk::Result::eSuccess, it will report an error message,
///     and then return the equivalent EGraphicsResult value.
///	@param renderDevice : Reference to the render device to report any errors. 
///	@param vkExpression : Expression that should evaluate to a VkResult.
//----------------------------------------------------------------------------------------------------
#define NES_VK_FAIL_RETURN(renderDevice, vkExpression)                                      \
do                                                                                          \
{                                                                                           \
    const vk::Result exprResult = static_cast<vk::Result>(vkExpression);                    \
    if (exprResult != vk::Result::eSuccess)                                                 \
    {                                                                                       \
        return (renderDevice).ReportOnError(exprResult, #vkExpression, __FILE__, __LINE__); \
    }                                                                                       \
} while (0)

//----------------------------------------------------------------------------------------------------
/// @brief : If the vkExpression doesn't evaluate to vk::Result::eSuccess, it will report an error message,
///     and then return the equivalent EGraphicsResult value.
///	@param renderDevice : Reference to the render device to report any errors. 
///	@param vkExpression : Expression that should evaluate to a VkResult.
//----------------------------------------------------------------------------------------------------
#define NES_VK_FAIL_RETURN_VOID(renderDevice, vkExpression)  \
do                                                      \
{                                                       \
    const vk::Result exprResult = static_cast<vk::Result>(vkExpression);           \
    if (exprResult != vk::Result::eSuccess)                                 \
    {                                                   \
        (renderDevice).ReportOnError(exprResult, #vkExpression, __FILE__, __LINE__); \
        return;                                         \
    }                                                   \
} while (0)

//----------------------------------------------------------------------------------------------------
/// @brief : If the vkExpression doesn't evaluate to vk::Result::eSuccess, it will report an error message. You can
///     use this to set an EGraphicsResult variable. Ex: EGraphicsResult result = NES_VK_FAIL_REPORT(...).
///	@param renderDevice : Reference to the render device to report any errors. 
///	@param vkExpression : Expression that should evaluate to a VkResult.
//----------------------------------------------------------------------------------------------------
#define NES_VK_FAIL_REPORT(renderDevice, vkExpression) (renderDevice).ReportOnError(vkExpression, #vkExpression, __FILE__, __LINE__)