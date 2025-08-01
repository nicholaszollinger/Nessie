// GraphicsCore.h
#pragma once
#include "Nessie/Core/Config.h"
#include "Nessie/Debug/Assert.h"
#include "Nessie/Math/Math.h"

#define NES_BEGIN_GRAPHICS_NAMESPACE namespace nes::graphics {
#define NES_END_GRAPHICS_NAMESPACE }

namespace nes
{
    NES_DEFINE_LOG_TAG(kGraphicsLogTag, "Graphics", Info);

    class   RenderDevice;
    struct  DeviceDesc;
    struct  PhysicalDeviceDesc;
    class   DeviceQueue;
    class   SemaphoreState;
    class   CommandPool;
    class   CommandBuffer;

    class   Swapchain;
    struct  SwapchainDesc;
    
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

#if NES_DEBUG
    //----------------------------------------------------------------------------------------------------
    /// @brief : Assert that an expression is true. This uses the RenderDevice's debug messenger callback.
    ///	@param renderDevice : Reference to the render device to report the message. 
    ///	@param expression : Boolean expression to test. 
    //----------------------------------------------------------------------------------------------------
    #define NES_GRAPHICS_ASSERT(renderDevice, expression)                                   \
        do                                                                                  \
        {                                                                                   \
            (renderDevice).CheckResult(expression, #expression, __FILE__, __LINE__);        \
            if (!(expression))                                                              \
            {                                                                               \
                NES_BREAKPOINT;                                                             \
            }                                                                               \
        } while (false)

#else
    #define NES_GRAPHICS_ASSERT(renderDevice, expression)
#endif
    
}