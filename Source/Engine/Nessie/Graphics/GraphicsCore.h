// GraphicsCore.h
#pragma once
#include "Nessie/Core/Config.h"
#include "Nessie/Debug/Assert.h"

#define NES_BEGIN_GRAPHICS_NAMESPACE namespace nes::graphics {
#define NES_END_GRAPHICS_NAMESPACE }

namespace nes
{
    NES_DEFINE_LOG_TAG(kGraphicsLogTag, "Graphics", Info);
    
    struct  DeviceDesc;
    struct  DeviceCreationDesc;
    struct  PhysicalDeviceDesc;

    class   Swapchain;
    struct  SwapchainDesc;

    class   GFence;               /// Synchronization primitive that can be used to insert a dependency between queue operations or a queue operation and the host.
    class   DeviceQueue;               /// A logical queue, providing access to hardware queue.
    class   GMemory;              /// A Memory blob allocated on device (GPU) or host (CPU).
    class   GBuffer;              /// A Buffer object: linear array of data.
    class   RenderDevice;               /// A logical device. Interface to the GPU.
    class   GTexture;             /// A texture object: multidimensional arrays of data.
    class   Pipeline;             /// A collection of state needed for rendering: shaders and fixed.
    class   GQueryPool;           /// A collection of queries, all the same type.
    class   Descriptor;           /// A handle/pointer to a resource.
    class   GCommandBuffer;       /// Used to record commands which can be submitted to a device queue for execution (aka command list).
    class   DescriptorSet;        /// A continuous set of descriptors (set of resource handles).
    class   DescriptorPool;       /// Maintains a pool of descriptors, and where descriptor sets are allocated from (aka descriptor heap).
    class   PipelineLayout;       /// Determines the interface between shaders stages and shader resources.
    class   GCommandAllocator;    /// An object that command buffer memory is allocated from.

    using SampleType    = uint8;
    using DimType       = uint16; /// "Dimension Type".
    using GMemoryType   = uint32;

    enum class EGraphicsAPI : uint8
    {
        Vulkan,
    };

    //----------------------------------------------------------------------------------------------------
    /// @brief : Result type returned from many critical functions in the graphics api. 
    //----------------------------------------------------------------------------------------------------
    enum class EGraphicsResult : int8
    {
        /// Values less than Success (0) may result in a crash, but also might be able to be handled.
        DeviceLost          = -2,   /// May be returned by QueueSubmit, WaitIdle, AcquireNextTexture, QueuePresent, WaitForPresent
        SwapchainOutOfDate  = -1,
    
        Success             = 0,    /// All good.
    
        /// The following most likely results in a crash, or at least a validation error will occur.
        Failure             = 1,
        InvalidArgument     = 2,    
        OutOfMemory         = 3,
        Unsupported         = 4,    /// Operation or type is unsupported by the Render Device.
    };

    //----------------------------------------------------------------------------------------------------
    /// @brief : Report an error message using RenderDevice::ReportMessage() function.  
    ///	@param renderDevice : Reference to the render device to report the message.
    ///	@param message : Message to send.
    //----------------------------------------------------------------------------------------------------
    #define NES_GRAPHICS_REPORT_ERROR(renderDevice, message) \
        (renderDevice).ReportMessage(nes::ELogLevel::Error, __FILE__, __LINE__, message, nes::kGraphicsLogTag);

    //----------------------------------------------------------------------------------------------------
    /// @brief : Returns the EGraphicsResult 'returnResult' if the expression evaluates to false. It will
    ///     also report the error message using the render device.
    ///	@param renderDevice : Reference to the render device to report the message.
    ///	@param expression : Boolean expression to be evaluated.
    ///	@param returnResult : EGraphicsResult value to return.
    ///	@param message : Message to report.
    //----------------------------------------------------------------------------------------------------
    #define NES_GRAPHICS_RETURN_ON_BAD_RESULT(renderDevice, expression, returnResult, message) \
    if (!expression) \
    { \
        (renderDevice).ReportMessage(nes::ELogLevel::Error, __FILE__, __LINE__, message, nes::kGraphicsLogTag); \
        return returnResult; \
    }
    
}