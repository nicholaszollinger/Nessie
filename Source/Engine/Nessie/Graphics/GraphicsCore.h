// GraphicsCore.h
#pragma once
#include <array>
#include "Nessie/Core/Config.h"
#include "Nessie/Debug/Assert.h"
#include "Nessie/Math/Math.h"

#include "Nessie/Graphics/Vulkan/VulkanObject.h"

//----------------------------------------------------------------------------------------------------
/// @brief : Same as VK_DEFINE_HANDLE.
///     Ex: "NES_GRAPHICS_DEFINE_HANDLE(Object);" defines the following: 
///        struct Object_T;
///        using Object = Object_T*;
///	@param handle : Name of the object handle you want to define.
//----------------------------------------------------------------------------------------------------
#define NES_GRAPHICS_DEFINE_HANDLE(handle) \
    struct handle##_T;                     \
    using handle = handle##_T*;

//----------------------------------------------------------------------------------------------------
// Forward declare VMA types.
//----------------------------------------------------------------------------------------------------
NES_GRAPHICS_DEFINE_HANDLE(VmaAllocator);
NES_GRAPHICS_DEFINE_HANDLE(VmaAllocation);
NES_GRAPHICS_DEFINE_HANDLE(VmaPool);

namespace nes
{
    NES_DEFINE_LOG_TAG(kGraphicsLogTag, "Graphics", Info);

    class   RenderDevice;
    struct  DeviceDesc;
    struct  PhysicalDeviceDesc;
    class   DeviceQueue;
    class   SemaphoreValue;
    class   CommandPool;
    class   CommandBuffer;
    class   Swapchain;
    struct  SwapchainDesc;
    class   ShaderLibrary;
    class   Shader;
    struct  ShaderDesc;
    class   DeviceImage;
    class   Texture;
    struct  ImageDesc;
    struct  AllocateImageDesc;
    class   DeviceMemory;
    class   DeviceBuffer;
    struct  AllocateBufferDesc;
    struct  BufferDesc;
    class   Descriptor;
    class   DescriptorSet;
    class   DescriptorPool;
    class   Pipeline;
    class   PipelineLayout;
    
    
    //----------------------------------------------------------------------------------------------------
    /// @brief : Result type returned from many critical functions in the graphics api. 
    //----------------------------------------------------------------------------------------------------
    enum class EGraphicsResult : int8
    {
        /// Values less than Success (0) may result in a crash, but also might be able to be handled.
        InitializationFailed    = -3,
        DeviceLost              = -2,   /// May be returned by QueueSubmit, WaitIdle, AcquireNextTexture, QueuePresent, WaitForPresent
        SwapchainOutOfDate      = -1,
    
        Success                 = 0,    /// All good.
    
        /// The following most likely results in a crash, or at least a validation error will occur.
        Failure                 = 1,
        InvalidArgument         = 2,    
        OutOfMemory             = 3,
        Unsupported             = 4,    /// Operation or type is unsupported by the Render Device.
    };

    //----------------------------------------------------------------------------------------------------
    /// @brief : Convert a Graphics Result to a string.
    //----------------------------------------------------------------------------------------------------
    constexpr const char* GraphicsResultToString(const EGraphicsResult result)
    {
        constexpr std::array<const char*, 8> kStringValues = 
        {
            "Initialization Failed",
            "Device Lost",
            "Swapchain Out-Of-Date",
            "Success",
            "Failure",
            "Invalid Argument",
            "Out Of Memory",
            "Unsupported",
        };
        
        const size_t index = static_cast<int8>(result) + (static_cast<int8>(EGraphicsResult::Success) - static_cast<int8>(EGraphicsResult::InitializationFailed));
        return kStringValues[index];
    }

    //----------------------------------------------------------------------------------------------------
    /// @brief : Report an error message using the RenderDevice's debug messenger callback.
    ///	@param renderDevice : Reference to the render device to report the message. 
    ///	@param pFormat : Format for the message.
    /// @param ... : Format arguments.
    //----------------------------------------------------------------------------------------------------
    #define NES_GRAPHICS_ERROR(renderDevice, pFormat, ...) \
        (renderDevice).ReportMessage(nes::ELogLevel::Error, __FILE__, __LINE__, fmt::format(pFormat, __VA_ARGS__).c_str())

    //----------------------------------------------------------------------------------------------------
    /// @brief : Report a warning message using the RenderDevice's debug messenger callback.
    ///	@param renderDevice : Reference to the render device to report the message. 
    ///	@param pFormat : Format for the message.
    /// @param ... : Format arguments.
    //----------------------------------------------------------------------------------------------------
    #define NES_GRAPHICS_WARN(renderDevice, pFormat, ...) \
        (renderDevice).ReportMessage(nes::ELogLevel::Warn, __FILE__, __LINE__, fmt::format(pFormat, __VA_ARGS__).c_str())

    //----------------------------------------------------------------------------------------------------
    /// @brief : Report an info message using the RenderDevice's debug messenger callback.
    ///	@param renderDevice : Reference to the render device to report the message. 
    ///	@param pFormat : Format for the message.
    /// @param ... : Format arguments.
    //----------------------------------------------------------------------------------------------------
    #define NES_GRAPHICS_INFO(renderDevice, pFormat, ...) \
        (renderDevice).ReportMessage(nes::ELogLevel::Info, __FILE__, __LINE__, fmt::format(pFormat, __VA_ARGS__).c_str())

    //----------------------------------------------------------------------------------------------------
    /// @brief : If the expression fails, this will exit the program. 
    //----------------------------------------------------------------------------------------------------
    #define NES_GRAPHICS_MUST_PASS(renderDevice, expression) \
        do                                                                                  \
        {                                                                                   \
            if (!(renderDevice).CheckResult(expression, #expression, __FILE__, __LINE__))   \
            {                                                                               \
                NES_BREAKPOINT;                                                             \
            }                                                                               \
        } while (false)

    //----------------------------------------------------------------------------------------------------
    /// @brief : If the expression evaluates to false, report the error message and return the returnVal
    ///	@param renderDevice : Reference to the render device to report the message.
    /// @param expression : Boolean expression to evaluate.
    /// @param returnVal : Result to return on the expression evaluating to false.
    ///	@param pFormat : Format for the message.
    /// @param ... : Format arguments.
    //----------------------------------------------------------------------------------------------------
    #define NES_GRAPHICS_RETURN_FAIL(renderDevice, expression, returnVal, pFormat, ...) \
        do                                                                                  \
        {                                                                                   \
            if (!(expression))                                                              \
            {                                                                               \
                (renderDevice).ReportMessage(nes::ELogLevel::Error, __FILE__, __LINE__, fmt::format(pFormat, __VA_ARGS__).c_str()); \
                return returnVal;                                                           \
            }                                                                               \
        } while (false)
}