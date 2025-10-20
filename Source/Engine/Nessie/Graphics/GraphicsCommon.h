// Common.h
#pragma once
#include "GraphicsCore.h"
#include "Formats.h"
#include "Nessie/Core/Color.h"
#include "Nessie/Core/Config.h"
#include "Nessie/Core/Version.h"

//-------------------------------------------------------------------------------------------------
// Under development. This file contains the main descriptions for many graphics types. I am
// bouncing off the NRI library. However, I only want to support Vulkan, so I am going to be
// changing these as I go.
//-------------------------------------------------------------------------------------------------

namespace nes
{
    
//============================================================================================================================================================================================
#pragma region [ Common ]
//============================================================================================================================================================================================

    namespace graphics
    {
        /// Special value that marks that the remaining amount of some span (number of mips, number of image layers, etc.) should be used. 
        static constexpr uint32 kUseRemaining = std::numeric_limits<uint32>::max();

        /// Special value to use if you want to use the entire device buffer's range.
        static constexpr uint64 kWholeSize = std::numeric_limits<uint64_t>::max();
    }
    
    //----------------------------------------------------------------------------------------------------
    /// @brief : Describes a region of the framebuffer that the output will be rendered to.
    ///
    /// This is commonly set with m_offset = (0, 0) and m_extent = 'extent of the framebuffer', so the entire
    /// framebuffer is drawn to.
    ///
    /// The min and max Depth values specify the range of depth values to use for the framebuffer. The values
    /// must be in the range [0, 1]. If you aren't doing anything special, they should remain as min = 0.f and max = 1.f.
    //----------------------------------------------------------------------------------------------------
    struct Viewport
    {
        /// Constructors.
        Viewport() = default;
        Viewport(const uint32 width, const uint32 height, const uint32 xOffset = 0, const uint32 yOffset = 0);

        /// Implicit conversion to the Vulkan Type.
        operator    vk::Viewport() const { return vk::Viewport(m_offset.x, m_offset.y, m_extent.x, m_extent.y, m_minDepth, m_maxDepth); }
        
        //----------------------------------------------------------------------------------------------------
        /// @brief : This offset is in pixel coordinates of the framebuffer.
        //----------------------------------------------------------------------------------------------------
        Viewport&   SetOffset(const uint32 x, const uint32 y);
        
        //----------------------------------------------------------------------------------------------------
        /// @brief : Set the Extent of the viewport, in pixels.
        //----------------------------------------------------------------------------------------------------
        Viewport&   SetExtent(const uint32 width, const uint32 height);

        Vec2        m_offset = Vec2::Zero();    // Pixel offset for the target framebuffer.
        Vec2        m_extent = Vec2::Zero();    // Pixel dimensions of the render area.
        float       m_minDepth = 0.f;           
        float       m_maxDepth = 1.f;
    };

    //----------------------------------------------------------------------------------------------------
    /// @brief : While viewports define the transformation from the image to the framebuffer, scissor
    ///     rectangles define in which region pixels will actually be stored. The rasterizer will discard
    ///     any pixels outside the scissored rectangles. They function like a filter rather than a transformation.
    ///
    ///     So if we wanted to draw to the entire framebuffer, we would specify a scissor rectangle that
    ///     covers it entirely: m_offset = (0, 0) and m_extent = 'extent of the framebuffer'.
    //----------------------------------------------------------------------------------------------------
    struct Scissor
    {
        /// Constructors
        Scissor() = default;
        Scissor(const uint32 width, const uint32 height, const int xOffset = 0, const int yOffset = 0);
        Scissor(const Viewport& viewport);
        
        /// Implicit conversion to the Vulkan Type.
        inline      operator vk::Rect2D() const { return vk::Rect2D({m_offset.x, m_offset.y}, {m_extent.x, m_extent.y}); }
        
        //----------------------------------------------------------------------------------------------------
        /// @brief : Set the scissor so that it contains the entire viewport.  
        //----------------------------------------------------------------------------------------------------
        Scissor&    FillViewport(const Viewport& viewport);

        IVec2       m_offset = IVec2::Zero();
        UVec2       m_extent = UVec2::Zero();
    };

    //----------------------------------------------------------------------------------------------------
    /// @brief : Values to use to clear depth and stencil channels in a depth attachment.
    //----------------------------------------------------------------------------------------------------
    struct ClearDepthStencilValue
    {
        ClearDepthStencilValue() = default;
        ClearDepthStencilValue(const float depth, const uint32 stencil = 0) : m_depth(depth), m_stencil(stencil) {}
        
        float           m_depth = 1.f;
        uint32          m_stencil = 0;
    };

    //----------------------------------------------------------------------------------------------------
    /// @brief : Values to use to clear color channels in a color attachment.
    //----------------------------------------------------------------------------------------------------
    union ClearColorValue
    {
        ClearColorValue() = default;
        ClearColorValue(const float r, const float g, const float b, float a = 1.0f) : m_float32(r, g, b, a) {}
        
        Vec4            m_float32{};
        UVec4           m_uint32;
        IVec4           m_int32;
    };

    //----------------------------------------------------------------------------------------------------
    /// @brief : Value used to clear either a color or depth attachment. 
    //----------------------------------------------------------------------------------------------------
    union ClearValue
    {
        ClearValue() : m_color(0.0f, 0.0f, 0.0f, 1.0f) {}
        ClearValue(const ClearColorValue& color) : m_color(color) {}
        ClearValue(const ClearDepthStencilValue depthStencil) : m_depthStencil(depthStencil) {}
        
        ClearColorValue         m_color;
        ClearDepthStencilValue  m_depthStencil;
    };
    
#pragma endregion

//============================================================================================================================================================================================
#pragma region [ Pipline Stages and Barriers ]
//============================================================================================================================================================================================

    //----------------------------------------------------------------------------------------------------
    /// @brief : Bit flags for the different stages in a pipeline. 
    //----------------------------------------------------------------------------------------------------
    enum class EPipelineStageBits : uint32
    {
        All                     = vk::PipelineStageFlagBits2::eAllCommands,
        None                    = vk::PipelineStageFlagBits2::eNone,

        // Graphics
        IndexInput              = NES_BIT(0),   // Index buffer consumption
        VertexShader            = NES_BIT(1),   // Vertex Shader
        TessControlShader       = NES_BIT(2),   // Tesselation control (hull) shader
        TessEvaluationShader    = NES_BIT(3),   // Tesselation evaluation (domain) shader
        GeometryShader          = NES_BIT(4),   // Geometry Shader
        MeshControlShader       = NES_BIT(5),   // Mesh control (task) shader
        MeshEvaluationShader    = NES_BIT(6),   // Mesh evaluation (amplification) shader
        FragmentShader          = NES_BIT(7),   // Fragment (pixel) shader
        DepthStencilAttachment  = NES_BIT(8),   // Depth-stencil read/write operations
        ColorAttachment         = NES_BIT(9),   // Color read/write operations

        // Compute
        ComputeShader           = NES_BIT(10),  // Compute Shader

        // Ray Tracing
        RayGenShader            = NES_BIT(11),  // Ray generation shader
        MissShader              = NES_BIT(12),  // Miss shader
        IntersectionShader      = NES_BIT(13),  // Intersection shader
        ClosestHitShader        = NES_BIT(14),  // Closest hit shader.
        AnyHitShader            = NES_BIT(15),  // Any hit shader
        CallableShader          = NES_BIT(16),  // Callable shader

        AccelerationStructure   = NES_BIT(17),  // 
        MicroMap                = NES_BIT(18),  // 

        // Other
        Copy                    = NES_BIT(19),  // 
        Resolve                 = NES_BIT(20),  // 
        ClearStorage            = NES_BIT(21),  // 

        // Modifiers
        Indirect                = NES_BIT(22), // Invoked by "Indirect" commands (used in addition to other bits)

        // Top and Bottom
        TopOfPipe               = NES_BIT(23),
        BottomOfPipe            = NES_BIT(24),

        // Grouped Stages
        TessellationShaders     = TessControlShader | TessEvaluationShader,
        MeshShaders             = MeshControlShader | MeshEvaluationShader,
        GraphicsShaders         = VertexShader | TessellationShaders | GeometryShader | MeshShaders | FragmentShader,
        Draw                    = IndexInput | GraphicsShaders | DepthStencilAttachment | ColorAttachment,
        RayTracingShaders       = RayGenShader | MissShader | IntersectionShader | ClosestHitShader | AnyHitShader | CallableShader, 
    };
    NES_DEFINE_BIT_OPERATIONS_FOR_ENUM(EPipelineStageBits)

    //----------------------------------------------------------------------------------------------------
    /// @brief : Describes which planes of an image are included in a view.
    /// @see : https://registry.khronos.org/vulkan/specs/latest/man/html/VkImageAspectFlagBits.html
    //----------------------------------------------------------------------------------------------------
    enum class EImagePlaneBits : uint8
    {
        All = 0,                // All planes.
        Color = NES_BIT(0),     // Color plane.
        Depth = NES_BIT(1),     // Depth Plane.
        Stencil = NES_BIT(2),   // Stencil plane.
    };
    NES_DEFINE_BIT_OPERATIONS_FOR_ENUM(EImagePlaneBits)

    //----------------------------------------------------------------------------------------------------
    /// @brief : Determines how/when a resource can be accessed in the pipeline.
    // https://registry.khronos.org/vulkan/specs/latest/man/html/VkAccessFlagBits2.html
    // https://microsoft.github.io/DirectX-Specs/d3d/D3D12EnhancedBarriers.html#d3d12_barrier_access
    //----------------------------------------------------------------------------------------------------
    enum class EAccessBits : uint32
    {
        //                                          ACCESS         Compatible EStageBits (including All)
        None                        = 0,
    
        // Buffer
        IndexBuffer                 = NES_BIT(0),   // Read         IndexInput
        VertexBuffer                = NES_BIT(1),   // Read         VertexShader
        UniformBuffer               = NES_BIT(2),   // Read         GraphicsShaders, ComputeShader, RayTracingShaders
        ArgumentBuffer              = NES_BIT(3),   // Read         Indirect
        ScratchBuffer               = NES_BIT(4),   // Read/Write   AccelerationStructure, Micromap
    
        // Attachment
        ColorAttachment             = NES_BIT(5),   // Read/Write   ColorAttachment
        ShadingRateAttachment       = NES_BIT(6),   // Read         FragmentShader
        DepthStencilAttachmentRead  = NES_BIT(7),   // Read         DepthStencilAttachment
        DepthStencilAttachmentWrite = NES_BIT(8),   // Write        DepthStencilAttachment
    
        // Acceleration Structure
        AccelerationStructureRead   = NES_BIT(9),   // Read         ComputeShader, RayTracingShaders, AccelerationStructure
        AccelerationStructureWrite  = NES_BIT(10),  // Write        AccelerationStructure
    
        // Micromap
        MicromapRead                = NES_BIT(11),  // Read         Micromap, AccelerationStructure
        MicromapWrite               = NES_BIT(12),  // Write        Micromap
    
        // Shader Resource
        ShaderResourceRead          = NES_BIT(13),  // Read         GraphicsShaders, ComputeShader, RayTracingShaders
        ShaderResourceStorage       = NES_BIT(14),  // Read/Write   GraphicsShaders, ComputeShader, RayTracingShaders, ClearStorage
        ShaderBindingTable          = NES_BIT(15),  // Read         RayTracingShaders
    
        // Copy
        CopySource                  = NES_BIT(16),  // Read         Copy
        CopyDestination             = NES_BIT(17),  // Write        Copy
    
        // Resolve
        ResolveSource               = NES_BIT(18),  // Read         Resolve
        ResolveDestination          = NES_BIT(19),  // Write        Resolve
    };
    NES_DEFINE_BIT_OPERATIONS_FOR_ENUM(EAccessBits);

    //----------------------------------------------------------------------------------------------------
    /// @brief : Image Layout types.
    // https://registry.khronos.org/vulkan/specs/latest/man/html/VkImageLayout.html
    //----------------------------------------------------------------------------------------------------
    enum class EImageLayout : uint8
    {
        // Special
        Undefined,
        General,                // ~All access, but not optimal.
        Present,                // No Access
    
        // Access Specific
        ColorAttachment,        // ColorAttachment
        ShadingRateAttachment,  // ShadingRateAttachment
        DepthStencilAttachment, // DepthStencilAttachmentWrite
        DepthStencilReadOnly,   // DepthStencilAttachmentRead, ShaderResource
        ShaderResource,         // ShaderResource
        ShaderResourceStorage,  // ShaderResourceStorage
        CopySource,             // CopySource
        CopyDestination,        // CopyDestination
        ResolveSource,          // ResolveSource
        ResolveDestination,     // ResolveDestination
        MaxNum,
    };

    namespace graphics
    {
        //----------------------------------------------------------------------------------------------------
        /// @brief : Special value to infer pipeline access for a barrier.
        //----------------------------------------------------------------------------------------------------
        static constexpr EPipelineStageBits kInferPipelineStage = static_cast<EPipelineStageBits>(std::numeric_limits<std::underlying_type_t<EPipelineStageBits>>::max());

        //----------------------------------------------------------------------------------------------------
        /// @brief : Special value to infer resource access for a barrier.
        //----------------------------------------------------------------------------------------------------
        static constexpr EAccessBits kInferAccess = static_cast<EAccessBits>(std::numeric_limits<std::underlying_type_t<EAccessBits>>::max());
    }

    //----------------------------------------------------------------------------------------------------
    /// @brief : An Access level and Pipeline Stage. 
    //----------------------------------------------------------------------------------------------------
    struct AccessStage
    {
        EAccessBits         m_access = EAccessBits::None;
        EPipelineStageBits  m_stages = EPipelineStageBits::None;
    };

    //----------------------------------------------------------------------------------------------------
    /// @brief : Describes the Access, Layout and PipelineStage for an image resource.
    //----------------------------------------------------------------------------------------------------
    struct AccessLayoutStage
    {
        EAccessBits         m_access = graphics::kInferAccess;
        EImageLayout        m_layout = EImageLayout::Undefined;
        EPipelineStageBits  m_stages = graphics::kInferPipelineStage;

        //----------------------------------------------------------------------------------------------------
        /// @brief : State for an image that is a copy destination.
        //----------------------------------------------------------------------------------------------------
        static constexpr AccessLayoutStage CopyDestinationState()   { return { EAccessBits::CopyDestination, EImageLayout::CopyDestination, EPipelineStageBits::Copy}; }

        //----------------------------------------------------------------------------------------------------
        /// @brief : State for an image in an unknown state. Common to use as a "before" state.
        //----------------------------------------------------------------------------------------------------
        static constexpr AccessLayoutStage UnknownState()           { return { EAccessBits::None, EImageLayout::Undefined, EPipelineStageBits::None }; }
    };

    //----------------------------------------------------------------------------------------------------
    /// @brief : Parameters to transition a DeviceBuffer's access and stage from one state to another.
    //----------------------------------------------------------------------------------------------------
    struct BufferBarrierDesc
    {
        DeviceBuffer*       m_pBuffer = nullptr;            // The Device Buffer that we are changing access for.
        AccessStage         m_before{};                     // The Access and Layout that the buffer is in before the transition.
        AccessStage         m_after{};                      // The Access and Layout that the buffer will be after the transition.
    };

    //----------------------------------------------------------------------------------------------------
    /// @brief : The type of queue operation that needs to be performed for this barrier. When a resource
    /// changes Queue ownership, it must be 'released' by the first queue and then 'acquired' by the receiving
    /// queue.
    ///     - Release: Giving ownership of a resource to another DeviceQueue.
    ///     - Acquire: Taking ownership of a resource from another DeviceQueue.
    //----------------------------------------------------------------------------------------------------
    enum class EBarrierQueueOp : uint8
    {
        None = 0,
        Release,    // Giving ownership of a resource to another DeviceQueue.
        Acquire,    // Taking ownership of a resource from another DeviceQueue.
    };

    //----------------------------------------------------------------------------------------------------
    /// @brief : Parameters to transition the Device Image's access and layout from one state to another.
    ///     Can also transfer queue ownership.
    //----------------------------------------------------------------------------------------------------
    struct ImageBarrierDesc
    {
        //----------------------------------------------------------------------------------------------------
        /// @brief : Set the image that will be transitioned.
        //----------------------------------------------------------------------------------------------------
        ImageBarrierDesc&   SetImage(DeviceImage* pImage, const EImagePlaneBits planes = EImagePlaneBits::Color);

        //----------------------------------------------------------------------------------------------------
        /// @brief : Set the layout that the image should be in before the barrier, and what layout the barrier should
        ///     set it as after.
        //----------------------------------------------------------------------------------------------------
        ImageBarrierDesc&   SetLayout(const EImageLayout before, const EImageLayout after);

        //----------------------------------------------------------------------------------------------------
        /// @brief : Set the pipeline stage that the image will be in its "before" state, and the stage that
        ///     the barrier needs to set the image's access and layout to "after" state.
        /// @note : By default, the pipeline stages will be inferred from the image layouts.
        //----------------------------------------------------------------------------------------------------
        ImageBarrierDesc&   SetBarrierStage(const EPipelineStageBits before, const EPipelineStageBits after);

        //----------------------------------------------------------------------------------------------------
        /// @brief : Set the access level that the image should have before the barrier, and the access the barrier
        ///     should set it to after.
        /// @note : By default, the image access will be inferred from the pipeline stages.
        //----------------------------------------------------------------------------------------------------
        ImageBarrierDesc&   SetAccess(const EAccessBits before, const EAccessBits after);

        //----------------------------------------------------------------------------------------------------
        /// @brief : Set the region of the Image planes, mips and layers that will be transitioned.
        /// @note : Defaults to the Color plane, the first mip level and first layer. 
        //----------------------------------------------------------------------------------------------------
        ImageBarrierDesc&   SetRegion(const EImagePlaneBits planes, const uint32 baseMip, const uint32 numMips, const uint32 baseLayer, const uint32 numLayers);

        //----------------------------------------------------------------------------------------------------
        /// @brief : Set the Queue that can currently owns the image, and the queue that will take ownership.
        ///     This is only needed for exclusive access images.
        // @see : https://registry.khronos.org/vulkan/specs/latest/html/vkspec.html#synchronization-queue-transfers
        //----------------------------------------------------------------------------------------------------
        ImageBarrierDesc&   SetQueueAccess(DeviceQueue* pSrcQueue, DeviceQueue* pDstQueue, const EBarrierQueueOp op);
        
        DeviceImage*        m_pImage = nullptr;                 // Device Image that we are transitioning.
        AccessLayoutStage   m_before = {};                      // The Access, Layout and PipelineStages that the image is in before the transition.
        AccessLayoutStage   m_after = {};                       // The Access, Layout and PipelineStages that the image will be after the transition.
        uint32              m_baseMip = 0;                      // The first mip level that we are transitioning.  
        uint32              m_mipCount = 1;                     // The number of mip levels that we are transitioning.
        uint32              m_baseLayer = 0;                    // The first image layer that we are transitioning.
        uint32              m_layerCount = 1;                   // The number of image layers that we are transitioning.
        DeviceQueue*        m_pSrcQueue = nullptr;              // If not null, the queue that currently owns the Image.
        DeviceQueue*        m_pDstQueue = nullptr;              // If not null, the queue that will own the Image.
        EImagePlaneBits     m_planes = EImagePlaneBits::Color;  // The planes of the image are we addressing.
        EBarrierQueueOp     m_queueOp = EBarrierQueueOp::None;  // Must be either set to Release or Acquire if queue ownership is changing. See EBarrierQueueOp for details

        // [TODO]: I don't want to have this here, but I am trying to solve an issue with the transfer of
        // resources between threads and queues.
        vk::Semaphore       m_transferSemaphore = nullptr;
    };

    //----------------------------------------------------------------------------------------------------
    // [TODO]: Buffer barriers, memory barriers.
    //
    /// @brief : Group of image and buffer barriers that can be set in a command buffer. Barriers are used
    ///     to transition data access of resources.
    //----------------------------------------------------------------------------------------------------
    struct BarrierGroupDesc
    {
        //----------------------------------------------------------------------------------------------------
        /// @brief : Set the image barriers for the group.
        //----------------------------------------------------------------------------------------------------
        BarrierGroupDesc& SetImageBarriers(const vk::ArrayProxy<ImageBarrierDesc>& barriers); 
        
        std::vector<ImageBarrierDesc> m_imageBarriers{};
    };

#pragma endregion

//============================================================================================================================================================================================
#pragma region [ Resources: creation ]
//============================================================================================================================================================================================

    enum class EImageType : uint8
    {
        Image1D,    // Single buffer of pixel data.
        Image2D,    // 2D buffer of pixel data, used for Textures.
        Image3D,    // 3D buffer of pixel data.
        MaxNum
    };

    //----------------------------------------------------------------------------------------------------------------
    /// Generally, the system avoids using "queue ownership transfers" (see "TextureBarrierDesc").
    /// In most cases, "ESharingMode" can be ignored. Where is it necessary?
    /// - VK: Use "Exclusive" for attachments participating into multi-queue activities to preserve DCC (Delta Color Compression) on some HW
    /// - D3D12: Use "Simultaneous" to concurrently use a texture as a "ShaderResource" (or "ShaderResourceStorage") and a "CopyDestination" for non overlapping texture regions
    // https://registry.khronos.org/vulkan/specs/latest/man/html/VkSharingMode.html
    //----------------------------------------------------------------------------------------------------------------
    enum class ESharingMode : uint8
    {
        Concurrent,
        Exclusive,
        Simultaneous,
    };

    //----------------------------------------------------------------------------------------------------
    /// @brief : Bit flags for how an image will be used. 
    //----------------------------------------------------------------------------------------------------
    enum class EImageUsageBits : uint8
    {
                                                        // Min Compatible Access:               Usage:
        None                            = 0,
        ShaderResource                  = NES_BIT(0),   // ShaderResource                       Read-only shader resource (SRV)
        ShaderResourceStorage           = NES_BIT(1),   // ShaderResourceStorage                Read/write shader resource (UAV)
        ColorAttachment                 = NES_BIT(2),   // ColorAttachment                      Color attachment (render target)
        DepthStencilAttachment          = NES_BIT(3),   // DepthStencilAttachmentRead/Write     Depth-stencil attachment (depth-stencil target)
        ShadingRateAttachment           = NES_BIT(4),   // ShadingRateAttachment                Shading rate attachment (source)
    };
    NES_DEFINE_BIT_OPERATIONS_FOR_ENUM(EImageUsageBits);

    //----------------------------------------------------------------------------------------------------
    /// @brief : Bit flags for how a buffer will be used. 
    //----------------------------------------------------------------------------------------------------
    enum class EBufferUsageBits : uint16
    {
                                                        // Min Compatible Access:               Usage:
        None                            = 0,
        ShaderResource                  = NES_BIT(0),   // ShaderResource                       Read-only shader resource (SRV)
        ShaderResourceStorage           = NES_BIT(1),   // ShaderResourceStorage                Read/write shader resource (UAV)
        VertexBuffer                    = NES_BIT(2),   // VertexBuffer                         Vertex Buffer
        IndexBuffer                     = NES_BIT(3),   // IndexBuffer                          Index Buffer
        UniformBuffer                   = NES_BIT(4),   // UniformBuffer                        Uniform buffer.
        ArgumentBuffer                  = NES_BIT(5),   // ArgumentBuffer                       Argument buffer in "Indirect" commands
        ScratchBuffer                   = NES_BIT(6),   // ScratchBuffer                        Scratch buffer in "CmdBuild*" commands
        ShaderBindingTable              = NES_BIT(7),   // ShaderBindingTable                   Shader binding table (SBT) in "CmdDispatchRays*" commands
        AccelerationStructureBuildInput = NES_BIT(8),   // ShaderResource                       Read-only input in "CmdBuildAccelerationStructures" command      
        AccelerationStructureStorage    = NES_BIT(9),   // AccelerationStructureRead/Write      (INTERNAL) acceleration structure storage
        MicromapBuildInput              = NES_BIT(10),  // ShaderResource                       Read-only input in "CmdBuildMicromaps" command
        MicromapStorage                 = NES_BIT(11),  // MicromapRead/Write                   (INTERNAL) micromap storage
    };
    NES_DEFINE_BIT_OPERATIONS_FOR_ENUM(EBufferUsageBits);

    //----------------------------------------------------------------------------------------------------
    /// @brief : Properites of a DeviceImage. 
    //----------------------------------------------------------------------------------------------------
    struct ImageDesc
    {
        EImageType          m_type = EImageType::Image2D;               // Type of image that this resource is.
        EImageUsageBits     m_usage = EImageUsageBits::ShaderResource;  // How the image will be used.
        EFormat             m_format = EFormat::Unknown;                // The pixel format of the image.
        uint32              m_width = 1;                                // Width of the image, in pixels.
        uint32              m_height = 1;                               // Height of the image, in pixels.
        uint32              m_depth = 1;                                // Depth of the image, in pixels.
        uint32              m_mipCount = 1;                             // Number of mip levels. Mip Level 0 is the original image, and subsequent levels are copies of the image with dimensions cut in half. They can be thought of as Level of Detail.
        uint32              m_layerCount = 1;                           // Number of layers for the image.
        uint32              m_sampleCount = 1;                          // Used for multisampling.
        ESharingMode        m_sharingMode = ESharingMode::Exclusive;    // Defines if this image can be used across multiple device queues.
        ClearValue          m_clearValue{};                             // Values to use when clearing this image.

        //----------------------------------------------------------------------------------------------------
        /// @brief : Ensures that ranges are valid. 
        //----------------------------------------------------------------------------------------------------
        void                Validate();
    };

    //----------------------------------------------------------------------------------------------------
    /// @brief : Properties of a Device Buffer.
    //----------------------------------------------------------------------------------------------------
    struct BufferDesc
    {
        uint64              m_size = 0;                                 // Byte size of the buffer.
        uint32              m_structuredStride = 0;                     // If the Buffer is being used as a storage buffer, this is the size of an element.
        EBufferUsageBits    m_usage = EBufferUsageBits::VertexBuffer;   // How the buffer will be used.
    };

#pragma endregion

//============================================================================================================================================================================================
#pragma region [ Resources: binding to memory ]
//============================================================================================================================================================================================

    //----------------------------------------------------------------------------------------------------
    /// @brief : Where the memory will be allocated. See the individual values for details.
    //----------------------------------------------------------------------------------------------------
    enum class EMemoryLocation : uint8
    {
        // Video Memory. Fast access from the GPU.
        // - No direct access for the CPU; mapping is not possible.
        // - Good for any resources that you frequently write and read on GPU, e.g. images used as color attachments
        //   (aka "render targets"), depth-stencil attachments, images/buffers used as storage image/buffer
        //   (aka "Unordered Access View (UAV)").
        // - For vertex buffers, index buffers, and images, use this location, and have the Staging uploader handle the
        //   transfer.
        Device,
        
        // Special pool of Video Memory. Exposed on AMD only. 256MiB. Soft fall back to HostUpload.
        // - CPU accessible, mapping possible. The memory is uncached, do not read.
        // - The memory is directly accessed by both the CPU and GPU - you don't need to do an explicit transfer.
        // - Good for resources updated frequently by the CPU (dynamic) and read by the GPU.
        DeviceUpload,

        // System Memory. Accessible to the CPU - mapping possible. Uncached.
        // - Good for CPU-side (staging) copy of your resources - used as a source of transfer.
        // - Data written by the CPU, read once by the GPU (constant/uniform buffers)).
        HostUpload,
        
        // System Memory. CPU reads and writes are cached.
        // - Good for resources written by the GPU, read by the CPU - results of computations.
        // - The memory is directly accessed by both the CPU and GPU - you don't need to do an explicit transfer.
        // - Use for any resources read or randomly accessed on the CPU.
        HostReadback,
        
        MaxNum
    };

    //----------------------------------------------------------------------------------------------------
    /// @brief : Allowable types for an index buffer. 
    //----------------------------------------------------------------------------------------------------
    enum class EIndexType : uint8
    {
        U16,
        U32,
    };

    using DeviceMemoryTypeIndex = uint16;
    using DeviceMemoryType = uint32; 

    //----------------------------------------------------------------------------------------------------
    /// @brief : Information about a Memory Type on the Device.
    //----------------------------------------------------------------------------------------------------
    struct DeviceMemoryTypeInfo
    {
        DeviceMemoryTypeIndex   m_index = 0;
        EMemoryLocation         m_location = EMemoryLocation::Device;
        bool                    m_mustBeDedicated = false; // Memory must be put into a dedicated memory object, containing only 1 object with offset = 0.

        //----------------------------------------------------------------------------------------------------
        /// @brief : Pack DeviceMemoryTypeInfo into the DeviceMemoryType. (It's a Reinterpret Cast).
        //----------------------------------------------------------------------------------------------------
        inline static DeviceMemoryType Pack(const DeviceMemoryTypeInfo& memoryTypeInfo)
        {
            return *(reinterpret_cast<const DeviceMemoryType*>(&memoryTypeInfo));
        }

        //----------------------------------------------------------------------------------------------------
        /// @brief : Unpack DeviceMemoryType into the DeviceMemoryTypeInfo. (It's a Reinterpret Cast).
        //----------------------------------------------------------------------------------------------------
        inline static DeviceMemoryTypeInfo Unpack(const DeviceMemoryType& memoryType)
        {
            return *(reinterpret_cast<const DeviceMemoryTypeInfo*>(&memoryType));
        }
    };
    static_assert(sizeof(DeviceMemoryType) == sizeof(DeviceMemoryTypeInfo));

    struct DeviceMemoryDesc
    {
        uint64              m_size;
        uint32              m_alignment;
        DeviceMemoryType    m_type;
        bool                m_mustBeDedicated; // Must be put into a dedicated memory object, containing only 1 object with offset = 0
    };

    //----------------------------------------------------------------------------------------------------
    /// @brief : Parameters for allocating a DeviceBuffer object. Also contains some static helper functions for
    ///     common use cases.
    //----------------------------------------------------------------------------------------------------
    struct AllocateBufferDesc
    {
        // Size of the buffer, in bytes.
        uint64                      m_size = 0;

        // If the Buffer is being used as a storage buffer (not a StorageTexelBuffer), this is the size of an element in the buffer.
        // - For StorageTexelBuffers or Uniform Buffers, leave this at 0.
        uint32                      m_structureStride = 0;

        // Defines how the buffer will be used.
        EBufferUsageBits            m_usage = EBufferUsageBits::ShaderResource;

        // Defines where the buffer will be allocated, which changes its access for the CPU.
        // See EMemoryLocation for more info.
        EMemoryLocation             m_location = EMemoryLocation::Device;

        // If empty, the buffer resource will have exclusive access. Otherwise, specified queue will 
        std::span<uint32>           m_queueFamilyIndices = {};
        
        // Set this to true if the allocation should have its own memory block.
        // Use it for special, big resources, like fullscreen images used as attachments.
        bool                        m_isDedicated   = false;
    };

    //----------------------------------------------------------------------------------------------------
    /// @brief : Parameters for allocating a DeviceImage object.
    //----------------------------------------------------------------------------------------------------
    struct AllocateImageDesc
    {
        // Image Description, including usage and dimensions.
        ImageDesc           m_imageDesc{};
        
        // Defines where the image will be allocated, which changes its access for the CPU.
        // See EMemoryLocation for more info.
        EMemoryLocation     m_memoryLocation = EMemoryLocation::Device;
        
        // If empty, the buffer resource will have exclusive access. Otherwise, specified queue will 
        std::span<uint32>   m_queueFamilyIndices = {};

        // Set this to true if the allocation should have its own memory block.
        // Use it for special, big resources, like fullscreen images used as attachments.
        bool                m_isDedicated = false;
    };
    
#pragma endregion

//============================================================================================================================================================================================
#pragma region [ Descriptor ]
//============================================================================================================================================================================================
    enum class EImage1DViewType : uint8
    {
        ShaderResource1D,
        ShaderResource1DArray,
        ShaderResourceStorage1D,
        ShaderResourceStorage1DArray,
        ColorAttachment,
        DepthStencilAttachment,
        DepthReadOnlyStencilAttachment,
        DepthAttachmentReadonly,
        DepthStencilReadonly,
    
        MaxNum
    };

    enum class EImage2DViewType : uint8
    {
        ShaderResource2D,
        ShaderResource2DArray,
        ShaderResourceCube,
        ShaderResourceCubeArray,
        ShaderResourceStorage2D,
        ShaderResourceStorage2DArray,
        ColorAttachment,
        DepthStencilAttachment,
        DepthReadOnlyStencilAttachment,
        DepthAttachmentReadonly,
        DepthStencilReadonly,
        ShadingRateAttachment,

        MaxNum
    };

    enum class EImage3DViewType : uint8
    {
        ShaderResource3D,
        ShaderResourceStorage3D,
        ColorAttachment,

        MaxNum
    };

    enum class EBufferViewType : uint8
    {
        ShaderResource,
        ShaderResourceStorage,
        Uniform
    };

    //----------------------------------------------------------------------------------------------------
    // https://registry.khronos.org/vulkan/specs/latest/man/html/VkFilter.html
    // https://registry.khronos.org/vulkan/specs/latest/man/html/VkSamplerMipmapMode.html
    //----------------------------------------------------------------------------------------------------
    enum class EFilterType : uint8
    {
        Nearest,
        Linear,
        MaxNum
    };

    //----------------------------------------------------------------------------------------------------
    // https://registry.khronos.org/vulkan/specs/latest/man/html/VkSamplerReductionMode.html
    //----------------------------------------------------------------------------------------------------
    enum class EReductionMode : uint8
    {
        Average,    // A weighted average of values in the footprint
        Min,        // A component-wise minimum of values in the footprint with non-zero weights
        Max,        // A component-wise maximum of values in the footprint with non-zero weights
        MaxNum
    };

    //----------------------------------------------------------------------------------------------------
    // https://registry.khronos.org/vulkan/specs/latest/man/html/VkSamplerAddressMode.html
    //----------------------------------------------------------------------------------------------------
    enum class EAddressMode : uint8
    {
        Repeat,
        MirroredRepeat,
        ClampToEdge,
        ClampToBorder,
        MirroredClampToEdge,
        MaxNum
    };

    //----------------------------------------------------------------------------------------------------
    /// @brief : Determines how to test fragment depth, stencil reference, or "SampleCmp" reference to the
    ///     current value in the depth or stencil buffer.
    //      R = Incoming fragment depth, stencil reference, or "SampleCmp" reference.
    //      D = Current value in the depth or stencil buffer.
    //----------------------------------------------------------------------------------------------------
    enum class ECompareOp : uint8
    {
        None,           // Test is disabled
        Always,         // True
        Never,          // False
        Equal,          // R == D
        NotEqual,       // R != D
        Less,           // R < D
        LessEqual,      // R <= D
        Greater,        // R > D
        GreaterEqual,   // R >= D
        MaxNum
    };

    //----------------------------------------------------------------------------------------------------
    /// @brief : Describes a region of a texture for copy commands. 
    //----------------------------------------------------------------------------------------------------
    struct ImageRegionDesc
    {
        //----------------------------------------------------------------------------------------------------
        /// @brief : Set the pixel offset of the region.
        //----------------------------------------------------------------------------------------------------
        ImageRegionDesc&    SetOffset(const uint32 x, const uint32 y, const uint32 z = 0);

        //----------------------------------------------------------------------------------------------------
        /// @brief : Set the pixel extent of the region. By default, each dimension is set to 'graphics::kUseRemaining',
        ///     which means that the remaining pixels from the offset will be used.
        //----------------------------------------------------------------------------------------------------
        ImageRegionDesc&    SetSize(const uint32 width, const uint32 height, const uint32 depth = 1);

        //----------------------------------------------------------------------------------------------------
        /// @brief : Set the mip level of the image that will be used. Mip Level 0 is the default. 
        //----------------------------------------------------------------------------------------------------
        ImageRegionDesc&    SetMipLevel(const uint32 mipLevel);

        //----------------------------------------------------------------------------------------------------
        /// @brief : Set the region's image layer. By default, the first layer (0) is used. 
        //----------------------------------------------------------------------------------------------------
        ImageRegionDesc&    SetLayer(const uint32 layer);

        //----------------------------------------------------------------------------------------------------
        /// @brief : Set the image planes that will be accessed for this region. All planes are set by default.
        //----------------------------------------------------------------------------------------------------
        ImageRegionDesc&    SetImagePlanes(const EImagePlaneBits planes);
        
        UInt3               m_offset = UInt3::Zero();               // Pixel offset of the region. Zero by default.
        uint32              m_width = graphics::kUseRemaining;      // Width of the region, or the number of pixels in the x-axis.
        uint32              m_height = graphics::kUseRemaining;     // Height of the region, or the number of pixels in the y-axis.
        uint32              m_depth = graphics::kUseRemaining;      // Depth of the region, or the number of pixels in the z-axis.
        uint32              m_mipLevel = 0;                         // Mip Level of the image.     
        uint32              m_layer = 0;                            // Which image layer.  
        EImagePlaneBits     m_planes = EImagePlaneBits::All;        // Which of the image planes will be accessed. Default is All. 
    };

    //----------------------------------------------------------------------------------------------------
    /// @brief : Describes access to a 1D Image resource. Used to create a Descriptor.
    //----------------------------------------------------------------------------------------------------
    struct Image1DViewDesc
    {
        const DeviceImage*  m_pImage;
        EImage1DViewType    m_viewType;
        EFormat             m_format = EFormat::Unknown;
        uint32              m_baseMipLevel = 0;
        uint32              m_mipCount = graphics::kUseRemaining;
        uint32              m_baseLayer = 0;
        uint32              m_layerCount = graphics::kUseRemaining;
    };

    //----------------------------------------------------------------------------------------------------
    /// @brief : Describes access to a 2D Texture resource. Used to create a Descriptor.
    //----------------------------------------------------------------------------------------------------
    struct Image2DViewDesc
    {
        const DeviceImage*  m_pImage = nullptr;
        EImage2DViewType    m_viewType;
        EFormat             m_format = EFormat::Unknown;
        uint32              m_baseMipLevel = 0;
        uint32              m_mipCount = graphics::kUseRemaining;
        uint32              m_baseLayer = 0;
        uint32              m_layerCount = graphics::kUseRemaining;
    };

    //----------------------------------------------------------------------------------------------------
    /// @brief : Describes access to a 3D Image resource. Used to create a Descriptor.
    //----------------------------------------------------------------------------------------------------
    struct Image3DViewDesc
    {
        const DeviceImage*  m_pImage = nullptr;
        EImage3DViewType    m_viewType;
        EFormat             m_format = EFormat::Unknown;
        uint32              m_baseMipLevel = 0;
        uint32              m_mipCount = graphics::kUseRemaining;
        uint32              m_baseSlice = 0;
        uint32              m_sliceCount = graphics::kUseRemaining;
    };

    //----------------------------------------------------------------------------------------------------
    /// @brief : Describes access to a Buffer resource. Used to create a Descriptor.
    //----------------------------------------------------------------------------------------------------
    struct BufferViewDesc
    {
        const DeviceBuffer* m_pBuffer;
        EBufferViewType     m_viewType;
        EFormat             m_format;               // Format of an element in the buffer.
        uint64              m_offset = 0;           // Offset from the beginning 
        uint64              m_size = vk::WholeSize;
    };

    struct AddressModes
    {
        EAddressMode u;
        EAddressMode v;
        EAddressMode w;
    };

    struct Filters
    {
        EFilterType m_min;          // This filter is used when the object is further from the camera.
        EFilterType m_mag;          // This filter is used when the object is close to the camera.
        EFilterType m_mip;
        EReductionMode m_reduction; // Requires features.m_textureFilterMinMax.
    };

    //----------------------------------------------------------------------------------------------------
    // https://registry.khronos.org/vulkan/specs/latest/man/html/VkSamplerCreateInfo.html
    /// @brief : Describes a sampler resource. Used to create a Descriptor.
    //----------------------------------------------------------------------------------------------------
    struct SamplerDesc
    {
        Filters             m_filters;
        uint8               m_anisotropy;
        float               m_mipBias;      // This bias allows you to use a lower level of detail that it would normally use.
        float               m_mipMin;       // Minimum level of detail (mip level) to use.
        float               m_mipMax;       // Maximum level of detail (mip level) to use.
        AddressModes        m_addressModes;
        ECompareOp          m_compareOp;
        ClearColorValue     m_borderColor;
        bool                m_isInteger;
    };

#pragma endregion

//============================================================================================================================================================================================
#pragma region [ Pipeline Layout and Descriptors Management ]
//============================================================================================================================================================================================

    //----------------------------------------------------------------------------------------------------
    // Pipeline Layout Overview:
    //  - A Pipeline Layout defines the resources that can be bound across the different shaders in a pipeline.
    //    This comes in the form of Descriptor Sets and Push Constants.
    //  - A Descriptor Set specifies the actual buffer or image resources that will be bound to the Shader at a given set index.
    //  - A Descriptor Binding is one or more resources at a specific binding index in the Shader.
    //  - A Push Constant is a small, single block of data that can have values set to it without the need of descriptors.
    //
    //  Example:
    //
    //  Descriptor Set (0)                  // "SetIndex = 0". A Descriptor Set index in the Pipeline Layout, provided as an argument or bound to the pipeline.
    //      * DescriptorBinding (0)         // "BindingIndex = 0". GLSL: "layout(set = 0, binding = 0)". This is a fixed array of Descriptors.   
    //          - Descriptor (0)            // - Descriptor value at index 0 in the array.
    //          - Descriptor (1)            // - Descriptor value at index 1 in the array.
    //      * DescriptorBinding (1)         // "BindingIndex = 1". GLSL: "layout(set = 0, binding = 1)"
    //          - Descriptor (0)            // - Descriptor value.
    //
    //  Descriptor Set (1)
    //      * DescriptorBinding (0)         // GLSL: "layout(set = 1, binding = 0)"
    //          - Descriptor (0)
    //
    //  Push Constant Block
    //      * Offset (0), Size (16)         // 16 bytes of the block can be used to push data to.
    //
    //  Resources:
    //  - Mapping Data to Shaders : https://docs.vulkan.org/guide/latest/mapping_data_to_shaders.html
    //  - Descriptor Arrays : https://docs.vulkan.org/guide/latest/descriptor_arrays.html
    //  - Push Constants : https://docs.vulkan.org/guide/latest/push_constants.html
    //----------------------------------------------------------------------------------------------------

    //----------------------------------------------------------------------------------------------------
    /// @brief : Contains an optional flag that allows allocated descriptor sets to be updated after being bound. 
    //----------------------------------------------------------------------------------------------------
    enum class EDescriptorPoolBits : uint8
    {
        None                                = 0,
        AllowUpdateAfterBound               = NES_BIT(0)  // Allows DescriptorSets allocated with this pool to be updated after being bound.
    };
    NES_DEFINE_BIT_OPERATIONS_FOR_ENUM(EDescriptorPoolBits);

    //----------------------------------------------------------------------------------------------------
    /// @brief : Contains an optional flag that allows a descriptor set to be updated after being bound. 
    //----------------------------------------------------------------------------------------------------
    enum class EDescriptorSetBits : uint8
    {
        None                                = 0,
        AllowUpdateAfterBound               = NES_BIT(0)  
    };
    NES_DEFINE_BIT_OPERATIONS_FOR_ENUM(EDescriptorSetBits);

    //----------------------------------------------------------------------------------------------------
    /// @brief : Flags that describe a bound descriptor, including whether descriptors at this binding
    ///     are organized into an array or not.
    /// @see : https://registry.khronos.org/vulkan/specs/latest/man/html/VkDescriptorBindingFlagBits.html
    //----------------------------------------------------------------------------------------------------
    enum class EDescriptorBindingBits : uint8
    {
        None                                = 0,
        PartiallyBound                      = NES_BIT(0),   // Descriptors at this binding may not contain valid descriptors at the time the descriptors are consumed (but referenced descriptors must be valid).
        Array                               = NES_BIT(1),   // Descriptors at this binding are organized into an array.
        VariableSizedArray                  = NES_BIT(2),   // Descriptors at this binding are organized into a variable-sized array; The size is specified via "numVariableDescriptors" argument of "DescriptorPool::AllocateDescriptorSets()" function.

        // https://docs.vulkan.org/samples/latest/samples/extensions/descriptor_indexing/README.html#_update_after_bind_streaming_descriptors_concurrently
        AllowUpdateAfterSet                 = NES_BIT(3),   // Descriptors at this binding can be updated after being bound, but before its submitted to the queue.
    };
    NES_DEFINE_BIT_OPERATIONS_FOR_ENUM(EDescriptorBindingBits);

    //----------------------------------------------------------------------------------------------------
    /// @brief : The type of resource that a Descriptor points to.
    /// @see : https://registry.khronos.org/vulkan/specs/latest/man/html/VkDescriptorType.html
    //----------------------------------------------------------------------------------------------------
    enum class EDescriptorType
    {
        None = -1,
        Sampler                 = vk::DescriptorType::eSampler,            
        UniformBuffer           = vk::DescriptorType::eUniformBuffer, // Also known as a "Constant" Buffer.
        Image                   = vk::DescriptorType::eSampledImage,
        StorageImage            = vk::DescriptorType::eStorageImage,     
        Buffer                  = vk::DescriptorType::eUniformTexelBuffer,
        StorageTexelBuffer      = vk::DescriptorType::eStorageTexelBuffer,          // A Storage buffer formatted for pixel-like data.
        StorageBuffer           = vk::DescriptorType::eStorageBuffer,               // A Storage buffer for custom structures. AKA a "structured buffer". 
        AccelerationStructure   = vk::DescriptorType::eAccelerationStructureKHR,
    };

    //----------------------------------------------------------------------------------------------------
    /// @brief : Describes one or more resources of a single type that are bound to a binding index in the shader.
    //----------------------------------------------------------------------------------------------------
    struct DescriptorBindingDesc
    {
        //----------------------------------------------------------------------------------------------------
        /// @brief : Set the binding index within the DescriptorSet.
        //----------------------------------------------------------------------------------------------------
        DescriptorBindingDesc&      SetBindingIndex(const uint32 index);

        //----------------------------------------------------------------------------------------------------
        /// @brief : Set the type of descriptor and the number of descriptor values of that type that will be
        /// bound. By default, the count is one.
        ///
        /// If the count is greater than 1, either the EDescriptorBindingBits::Array or EDescriptorBindingBits::VariableSizedArray
        /// flags must be set. Otherwise, the count will be ignored and it will act as a single element.
        ///
        /// If the EDescriptorBindingBits::VariableSizedArray flag is set, then the count will be the maximum
        /// number of elements in that array.
        //----------------------------------------------------------------------------------------------------
        DescriptorBindingDesc&      SetDescriptorType(const EDescriptorType type, const uint32 count = 1);

        //----------------------------------------------------------------------------------------------------
        /// @brief : Set the shader stages that will reference this descriptor.
        //----------------------------------------------------------------------------------------------------
        DescriptorBindingDesc&      SetShaderStages(const EPipelineStageBits stages);

        //----------------------------------------------------------------------------------------------------
        /// @brief : Set any flags for this binding. See EDescriptorBindingBits for more details.
        //----------------------------------------------------------------------------------------------------
        DescriptorBindingDesc&      SetFlags(const EDescriptorBindingBits flags);

        uint32                      m_bindingIndex = 0;
        uint32                      m_descriptorCount = 1;
        EDescriptorType             m_descriptorType = EDescriptorType::None;
        EPipelineStageBits          m_shaderStages = EPipelineStageBits::GraphicsShaders;
        EDescriptorBindingBits      m_flags = EDescriptorBindingBits::None;
    };
    
    //----------------------------------------------------------------------------------------------------
    // [TODO]: Dynamic Constant Buffers, which allow dynamic offsets in the buffer to be set.
    //      - https://docs.vulkan.org/guide/latest/descriptor_dynamic_offset.html
    //
    /// @brief : Describes a set of shader variables used in a pipeline.
    //----------------------------------------------------------------------------------------------------
    struct DescriptorSetDesc
    {
        DescriptorSetDesc& SetBindings(const DescriptorBindingDesc* pBindings, const uint32 count);
        
        const DescriptorBindingDesc*    m_pBindings = nullptr;   // Pointer to the array of bindings.
        uint32                          m_numBindings = 0;      // Number of bindings in the set.
        EDescriptorSetBits              m_flags = EDescriptorSetBits::None;
    };

    //----------------------------------------------------------------------------------------------------
    /// @brief : A Push Constants are a small bank of values writable and accessible in shaders.
    ///     Push constants allow the application to set values used in shaders without creating buffers
    ///     or modifying and binding descriptor sets for each update.
    //----------------------------------------------------------------------------------------------------
    struct PushConstantDesc
    {
        uint32                      m_offset = 0;   // Byte offset in the push constant block.
        uint32                      m_size = 0;     // Size of the type of Push Constant.
        EPipelineStageBits          m_shaderStages = EPipelineStageBits::GraphicsShaders;
    };

    //----------------------------------------------------------------------------------------------------
    /// @brief : Defines the resource bindings across all shaders in the pipeline.
    /// @see : https://registry.khronos.org/vulkan/specs/latest/man/html/VkPipelineLayoutCreateInfo.html
    //----------------------------------------------------------------------------------------------------
    struct PipelineLayoutDesc
    {
        //----------------------------------------------------------------------------------------------------
        /// @brief : Set the descriptor sets that will be referenced for the pipeline with this layout. 
        //----------------------------------------------------------------------------------------------------
        PipelineLayoutDesc&         SetDescriptorSets(const vk::ArrayProxy<DescriptorSetDesc>& sets);

        //----------------------------------------------------------------------------------------------------
        /// @brief : Set the push constant descriptions for the global push constant block. 
        //----------------------------------------------------------------------------------------------------
        PipelineLayoutDesc&         SetPushConstants(const vk::ArrayProxy<PushConstantDesc>& pushConstants);

        //----------------------------------------------------------------------------------------------------
        /// @brief : Set which shader stages the Pipeline Layout will be used.
        //----------------------------------------------------------------------------------------------------
        PipelineLayoutDesc&         SetShaderStages(const EPipelineStageBits stages);
        
        vk::ArrayProxy<DescriptorSetDesc> m_descriptorSets{};
        vk::ArrayProxy<PushConstantDesc>  m_pushConstants{};
        EPipelineStageBits                m_shaderStages = EPipelineStageBits::GraphicsShaders; // Which shader stages will this pipeline layout be used.
    };

    //----------------------------------------------------------------------------------------------------
    /// @brief : Used to create a Descriptor Pool. The values determine how many DescriptorSets can be allocated,
    ///     and how many of each type of shader variable can be allocated.
    /// @see: https://registry.khronos.org/vulkan/specs/latest/man/html/VkDescriptorPoolCreateInfo.html
    //----------------------------------------------------------------------------------------------------
    struct DescriptorPoolDesc
    {
        uint32                      m_descriptorSetMaxNum = 0;
        uint32                      m_samplerMaxNum = 0;      
        uint32                      m_uniformBufferMaxNum = 0;      
        uint32                      m_dynamicUniformBufferMaxNum = 0;      
        uint32                      m_imageMaxNum = 0;      
        uint32                      m_storageImageMaxNum = 0;
        uint32                      m_bufferMaxNum = 0;
        uint32                      m_storageBufferMaxNum = 0;          // Number of Storage Buffer (AKA Structured Buffers) that can be allocated.
        uint32                      m_storageTexelBufferMaxNum = 0;     // Number of Storage Texel Buffers (storage buffers with pixel-like elements) that can be allocated.
        uint32                      m_accelerationStructureMaxNum = 0;
        EDescriptorPoolBits         m_flags = EDescriptorPoolBits::None;
    };

    //----------------------------------------------------------------------------------------------------
    /// @brief : Used to update a single binding's Descriptor values in a Descriptor Set. 
    //----------------------------------------------------------------------------------------------------
    struct DescriptorBindingUpdateDesc
    {
        const Descriptor* const*    m_pDescriptors = nullptr;   // Array of descriptors that will be written to the descriptor set.
        uint32                      m_descriptorCount = 0;      // Number of descriptors values that will be set.
        uint32                      m_baseDescriptorIndex = 0;  // If the binding is an array, this is the first element to update in that array.
    };

#pragma endregion

//============================================================================================================================================================================================
#pragma region [ Graphics Pipeline: Input Assembly ]
//============================================================================================================================================================================================
    
    enum class EPrimitiveRestart : uint8
    {
        Disabled,   
        IndicesU16, // Index "0xFFFF" enforces primitive restart.
        IndicesU32  // Index "0xFFFFFFFF" enforces primitive restart.
    };

    //----------------------------------------------------------------------------------------------------
    /// https://registry.khronos.org/vulkan/specs/latest/man/html/VkVertexInputRate.html
    //----------------------------------------------------------------------------------------------------
    enum class EVertexStreamStepRate : uint8
    {
        PerVertex,      // Move to the next data entry after each vertex.
        PerInstance,    // Move to the next data entry after each instance.
    };

    //----------------------------------------------------------------------------------------------------
    /// @brief : Describes what kind of geometry will be drawn from the vertices.
    /// @note : https://registry.khronos.org/vulkan/specs/latest/man/html/VkPrimitiveTopology.html 
    //----------------------------------------------------------------------------------------------------
    enum class ETopology : uint8
    {
        PointList,                  // Points from vertices.
        LineList,                   // Line from every two vertices without reuse.
        LineStrip,                  // The end vertex of every line is used as start vertex for the next line.
        TriangleList,               // Triangle from every three vertices without reuse.
        TriangleStrip,              // The second and third vertex of every triangle is used as first two vertices of the next triangle
        TriangleFan,                // Specifies a series of connected triangle primitives with all triangles sharing a common vertex.
        LineListWithAdjacency,      // Specifies a series of separate line primitives with adjacency
        LineStripWithAdjacency,     // Specifies a series of connected line primitives with adjacency, with consecutive primitives sharing three vertices.
        TriangleListWithAdjacency,  // Specifies a series of separate triangle primitives with adjacency.
        TriangleStripWithAdjacency, // Specifies connected triangle primitives with adjacency, with consecutive triangles sharing an edge. 
        PatchList,                  // Specifies separate patch primitives. https://registry.khronos.org/vulkan/specs/latest/html/vkspec.html#drawing-patch-lists 
    };

    //----------------------------------------------------------------------------------------------------
    /// @brief : The Input Assembly defines two things: what kind of geometry will be drawn from the
    /// vertices and if 'primitive restart' should be enabled.
    ///
    /// Primitive Restart controls whether a special vertex index value is treated as restarting
    /// the assembly of primitives. This only applies to indexed draws. More info in the link attached.
    /// @note : https://registry.khronos.org/vulkan/specs/latest/man/html/VkPipelineInputAssemblyStateCreateInfo.html
    //----------------------------------------------------------------------------------------------------
    struct InputAssemblyDesc
    {
        ETopology               m_topology              = ETopology::TriangleList;      // Type of geometry that will be drawn.
        uint8                   m_tessControlPointCount = 0;                            // Only used with ETopology::PatchList. Number of control points per patch.
        EPrimitiveRestart       m_primitiveRestart      = EPrimitiveRestart::Disabled;  // Whether Primitive resart should be enabled.
    };

    //----------------------------------------------------------------------------------------------------
    /// @brief : Describes a single attribute (or variable) in a vertex stream.
    ///
    /// Example: <code>
    ///  struct Vertex
    ///  {
    ///      nes::Vec2 m_position;
    ///      nes::Vec3 m_color;
    ///  }
    ///
    ///  // For Position:
    ///  VertexAttributeDesc positionDesc{};
    ///  positionDesc.m_location = 0;                   // This is first element in the shaders input.
    ///  positionDesc.m_offset = offsetof(Vertex, m_position); // Byte offset of this data.
    ///  positionDesc.m_format = EFormat::RG32_SFloat;  // Position is 2, 32-bit floats
    ///  positionDesc.m_streamIndex = 0;                // Which vertex stream is this a part of.
    /// </code>
    //----------------------------------------------------------------------------------------------------
    struct VertexAttributeDesc
    {
        uint32                  m_location = 0;     // References the location directive of the input in the vertex shader.
        uint32                  m_offset = 0;       // Byte offset of the attribute in the greater Vertex object.
        EFormat                 m_format;           // Format of the data type.
        uint32                  m_streamIndex = 0;  // Which stream index the per-vertex data comes through to the shader.
    };

    //----------------------------------------------------------------------------------------------------
    /// @brief : A Vertex Stream is a buffer of a single data type sent to the shader. This describes
    ///     the stream's binding position in the shader, as well as when you need to change the vertex data;
    ///     either per vertex or per instance.
    ///
    ///     You can have a vertex stream that contains individual vertex information (position, normal, etc.),
    ///     and another stream that contains an object's transform (world position, rotation, scale).
    ///
    ///     Having different vertex streams can allow you to bind less data per invocation of the vertex shader.
    ///     For vertex specific data, that will have m_stepRate EVertexStreamStepRate::PerVertex, as it is
    ///     different for each vertex. But the object's transform only changes per object, so that step rate
    ///     can be EVertexStreamStepRate::PerInstance.
    //----------------------------------------------------------------------------------------------------
    struct VertexStreamDesc
    {
        //----------------------------------------------------------------------------------------------------
        /// @brief : Set the number of bytes to move between consecutive elements within the buffer. This is
        ///     commonly the size of the struct that you use to represent a vertex element.
        /// @note : Be sure to take alignment into account!
        //----------------------------------------------------------------------------------------------------
        VertexStreamDesc&           SetStride(const uint32 stride);

        //----------------------------------------------------------------------------------------------------
        /// @brief : Set which index this stream will be bound to in the shader.
        //----------------------------------------------------------------------------------------------------
        VertexStreamDesc&           SetBinding(const uint32 index);

        //----------------------------------------------------------------------------------------------------
        /// @brief : Set how often we move to the next data entry, either for each vertex, or for each instance.
        //----------------------------------------------------------------------------------------------------
        VertexStreamDesc&           SetStepRate(const EVertexStreamStepRate stepRate);
        
        uint32                      m_stride = 0;                                   // The number of bytes to move between consecutive elements in the buffer.
        uint32                      m_bindingIndex = 0;                             // Specifies the index in the array of vertex bindings.
        EVertexStreamStepRate       m_stepRate = EVertexStreamStepRate::PerVertex;  // When should we move to the next data entry? After each vertex? Or after each instance?
    };

    //----------------------------------------------------------------------------------------------------
    /// @brief : Defines what data is sent to the vertex shader.
    ///     - The Attributes define the individual data variables of a single element of a Vertex Stream.
    ///     - The Streams define where the buffer will be bound in the shader, and how often it is updated.
    /// @see : VertexAttributeDesc, VertexStreamDesc.
    //----------------------------------------------------------------------------------------------------
    struct VertexInputDesc
    {
        //----------------------------------------------------------------------------------------------------
        /// @brief : Set the different attributes of all vertex streams. They describe the data layout and
        ///     which stream they are a part of.
        //----------------------------------------------------------------------------------------------------
        VertexInputDesc&        SetAttributes(const vk::ArrayProxy<VertexAttributeDesc>& attributes);

        //----------------------------------------------------------------------------------------------------
        /// @brief : Set the number of different vertex streams that will be sent to the shader.
        //----------------------------------------------------------------------------------------------------
        VertexInputDesc&        SetStreams(const vk::ArrayProxy<VertexStreamDesc>& streams);
        
        vk::ArrayProxy<VertexAttributeDesc> m_attributes{};
        vk::ArrayProxy<VertexStreamDesc>    m_streams{};
    };

    //----------------------------------------------------------------------------------------------------
    /// @brief : Describes a contiguous section of a DeviceBuffer resource.
    //----------------------------------------------------------------------------------------------------
    class DeviceBufferRange
    {
    public:
        DeviceBufferRange() = default;
        
        //----------------------------------------------------------------------------------------------------
        /// @brief : Construct the range based on the offset and size.
        ///	@param pBuffer : Buffer resource that this range is for. Must not be null.
        ///	@param offset : Byte offset from the beginning of the DeviceBuffer. Default is 0; the start of the buffer.
        ///	@param size : Size, in bytes of the entire range. By default, it is set to graphics::kWholeSize which
        ///     will get the size from the offset to the end of the buffer. If offset + size is greater than
        ///     the buffer's size, the range will be clamped to the buffer's size.
        //----------------------------------------------------------------------------------------------------
        DeviceBufferRange(DeviceBuffer* pBuffer, const uint64 offset = 0, const uint64 size = graphics::kWholeSize);

        //----------------------------------------------------------------------------------------------------
        /// @brief : Get the Device Buffer that this range is in.
        //----------------------------------------------------------------------------------------------------
        DeviceBuffer*           GetBuffer() const           { return m_pBuffer; }

        //----------------------------------------------------------------------------------------------------
        /// @brief : Get the size of the range, in bytes.
        //----------------------------------------------------------------------------------------------------
        uint64                  GetSize() const             { return m_size; }

        //----------------------------------------------------------------------------------------------------
        /// @brief : Get the byte offset of the buffer.
        //----------------------------------------------------------------------------------------------------
        uint64                  GetOffset() const           { return m_offset; }

        //----------------------------------------------------------------------------------------------------
        /// @brief : Get the CPU-addressable memory pointer to the beginning of the range. Only valid if the
        ///     DeviceBuffer is mappable.
        //----------------------------------------------------------------------------------------------------
        uint8*                  GetMappedMemory() const;

        //----------------------------------------------------------------------------------------------------
        /// @brief : Get the address of the range on the device.
        //----------------------------------------------------------------------------------------------------
        uint64                  GetDeviceAddress() const;

        //----------------------------------------------------------------------------------------------------
        /// @brief : Check that the range is associated with a valid buffer and that its size is non-zero.
        //----------------------------------------------------------------------------------------------------
        bool                    IsValid() const             { return m_pBuffer != nullptr && m_size > 0; }

    private:
        DeviceBuffer*           m_pBuffer = nullptr;
        uint64                  m_offset = 0;
        uint64                  m_size = 0;
    };

    //----------------------------------------------------------------------------------------------------
    /// @brief : Describes a range of vertices within a DeviceBuffer. This can be used for draw calls. 
    //----------------------------------------------------------------------------------------------------
    class VertexBufferRange final : public DeviceBufferRange
    {
    public:
        VertexBufferRange() = default;
        
        //----------------------------------------------------------------------------------------------------
        /// @brief : Construct the description of vertices within the Device Buffer.
        ///	@param pBuffer : Buffer resource that contains the vertices. Must not be null. 
        ///	@param stride : The number of bytes needed to move to get to the next vertex. Commonly, this is
        ///     the size of the Vertex object.
        ///	@param vertexCount : Number of vertices in the range.
        ///	@param bufferOffset : Byte offset from the beginning of the DeviceBuffer to get to the first vertex.
        ///     Default is 0; the start of the buffer.
        //----------------------------------------------------------------------------------------------------
        VertexBufferRange(DeviceBuffer* pBuffer, const uint64 stride, const uint64 vertexCount, const uint64 bufferOffset = 0);

        //----------------------------------------------------------------------------------------------------
        /// @brief : Returns the number of bytes needed to move to get to the next vertex.
        ///     Commonly, this is the size of the Vertex object.
        //----------------------------------------------------------------------------------------------------
        uint32              GetStride() const       { return m_stride; }

        //----------------------------------------------------------------------------------------------------
        /// @brief : Get the number of vertices in the range.
        //----------------------------------------------------------------------------------------------------
        uint32              GetNumVertices() const  { return m_vertexCount; }

    private:
        uint32              m_stride = 0;
        uint32              m_firstVertex = 0;
        uint32              m_vertexCount = 0;
    };

    //----------------------------------------------------------------------------------------------------
    /// @brief : Describes a range of indices within a DeviceBuffer. This can be used for indexed draw calls. 
    //----------------------------------------------------------------------------------------------------
    class IndexBufferRange final : public DeviceBufferRange
    {
    public:
        IndexBufferRange() = default;

        //----------------------------------------------------------------------------------------------------
        /// @brief : Construct the description of indices within the Device Buffer.
        ///	@param pBuffer : Buffer resource that contains the indices. Must not be null. 
        ///	@param indexCount : Number of indices in the range.
        ///	@param firstIndex : First index in the range.
        ///	@param type : Type of index that is stored. Default is uint32.
        ///	@param bufferOffset : Byte offset from the beginning of the DeviceBuffer to get to first index.
        ///     Default is 0; the start of the buffer.
        //----------------------------------------------------------------------------------------------------
        IndexBufferRange(DeviceBuffer* pBuffer, const uint64 indexCount, const uint32 firstIndex = 0, const EIndexType type = EIndexType::U32, const uint64 bufferOffset = 0);
        
        //----------------------------------------------------------------------------------------------------
        /// @brief : Get the type of indices that this index buffer contains.
        //----------------------------------------------------------------------------------------------------
        EIndexType          GetIndexType() const       { return m_indexType; }

        //----------------------------------------------------------------------------------------------------
        /// @brief : Get the number of indices in the range.
        //----------------------------------------------------------------------------------------------------
        uint32              GetNumIndices() const    { return m_indexCount; }

        //----------------------------------------------------------------------------------------------------
        /// @brief : Get the index of the first index value in the range.
        //----------------------------------------------------------------------------------------------------
        uint32              GetFirstIndex() const    { return m_firstIndex; }
    
    private:
        uint32              m_firstIndex = 0;
        uint32              m_indexCount = 0;
        EIndexType          m_indexType  = EIndexType::U16;
    };

#pragma endregion

//============================================================================================================================================================================================
#pragma region [ Graphics Pipeline: Rasterization ]
//============================================================================================================================================================================================

    //----------------------------------------------------------------------------------------------------
    /// @brief : Defines where fragments are generated to be colored in by the fragment shader.
    /// @note : https://registry.khronos.org/vulkan/specs/latest/man/html/VkPolygonMode.html 
    //----------------------------------------------------------------------------------------------------
    enum class EFillMode : uint8
    {
        Solid,      // Fill the area of the polygon with fragments.
        Wireframe,  // Polygon edges are drawn as lines.
        Point,     // Polygon vertices are drawn as points.
    };

    //----------------------------------------------------------------------------------------------------
    /// @brief : Determines which faces will be culled from rendering. 
    /// @note : https://registry.khronos.org/vulkan/specs/latest/man/html/VkCullModeFlagBits.html
    //----------------------------------------------------------------------------------------------------
    enum class ECullMode : uint8
    {
        None,       // No faces will be culled.
        Front,      // Front faces will be culled.
        Back,       // Back faces will be culled.
        Both,       // Both front and back faces will be culled.
    };

    //----------------------------------------------------------------------------------------------------
    /// @brief : Vertex order can be used to determine which "direction" a polygon is facing.
    //----------------------------------------------------------------------------------------------------
    enum class EFrontFaceWinding : uint8
    {
        Clockwise,
        CounterClockwise,
    };

    //----------------------------------------------------------------------------------------------------
    // https://microsoft.github.io/DirectX-Specs/d3d/VariableRateShading.html
    //----------------------------------------------------------------------------------------------------
    enum class EShadingRate : uint8
    {
        FragmentSize_1x1,
        FragmentSize_1x2,
        FragmentSize_2x1,
        FragmentSize_2x2,

        // Requires "features.m_additionalShadingRates".
        FragmentSize_2x4,
        FragmentSize_4x2,
        FragmentSize_4x4,
    };

    //----------------------------------------------------------------------------------------------------
    // https://registry.khronos.org/vulkan/specs/latest/man/html/VkFragmentShadingRateCombinerOpKHR.html
    //    "primitiveCombiner"      "attachmentCombiner"
    // A   Pipeline shading rate    Result of Op1
    // B   Primitive shading rate   Attachment shading rate
    //----------------------------------------------------------------------------------------------------
    enum class EShadingRateCombiner : uint8
    {
        Keep,       // A
        Replace,    // B
        Min,        // min(A, B)
        Max,        // max(A, B)
        Sum,        // (A + B) or (A * B)
    };

    //----------------------------------------------------------------------------------------------------
    // https://registry.khronos.org/vulkan/specs/latest/html/vkspec.html#primsrast-depthbias-computation
    // 
    // R - minimum resolvable difference
    // S - maximum slope
    // 
    // bias = constant * R + slopeFactor * S
    // if (clamp > 0)
    //     bias = min(bias, clamp)
    // else if (clamp < 0)
    //     bias = max(bias, clamp)
    //----------------------------------------------------------------------------------------------------
    struct DepthBiasDesc
    {
        float           m_constant  = 0.f;      // Always applied.
        float           m_clamp     = 0.f;      // If non-zero, clamps the bias value to either a minimum or maximum.
        float           m_slope     = 0.f;      // Applied based on the polygon's slope.
        bool            m_enabled   = false;    // If enabled, the Depth Bias values are expected to be set dynamically in the frame (CommandBuffer::SetDepthBias()).
    };

    //----------------------------------------------------------------------------------------------------
    /// @brief : The rasterizer takes the geometry shaped by the vertices from the vertex shader and turns it
    ///     into fragments to be colored by the fragment shader. It also performs depth testing, face culling,
    ///     and the scissor test, and it can be configured to output fragments that fill entire polygons or
    ///     just the edges (wireframe rendering).
    //----------------------------------------------------------------------------------------------------
    struct RasterizationDesc
    {
        DepthBiasDesc       m_depthBias{};
        EFillMode           m_fillMode                  = EFillMode::Solid;

        // Determines the type of face culling to use. You can disable culling, cull the front faces,
        // cull the back faces or both. The m_frontFace variable specifies the vertex order for the
        // faces to be considered front-facing and can be clockwise or counterclockwise.
        ECullMode           m_cullMode                  = ECullMode::Back;
        EFrontFaceWinding   m_frontFace                 = EFrontFaceWinding::Clockwise;

        // Describes the thickness of lines in terms of number of fragments.
        // The maximum line width that is supported depends on the hardware and any line thicker
        // than 1.0f requires you to enable the 'wideLines' GPU feature.
        float               m_lineWidth                 = 1.f;
        
        // If true, then fragments that are beyond the near and far planes are clamped to them rather
        // than discarding them. This is useful in some special cases like shadow maps.
        // Using this requires enabling a GPU feature.
        bool                m_enableDepthClamp          = false;
    };

    //----------------------------------------------------------------------------------------------------
    /// @brief : Multisampling is one of the ways to perform antialiasing.
    ///
    ///     It works by combining the fragment shader results of multiple polygons
    ///     that rasterize to the same pixel. This mainly occurs along edges, which is also where
    ///     the most noticeable aliasing artifacts occur. Because it does not need to run the fragment
    ///     shader multiple times if only one polygon maps to a pixel, it is significantly less expensive
    ///     than simply rendering to a higher resolution and then downscaling.
    ///     Enabling it requires enabling a GPU feature.
    ///
    /// @note : https://en.wikipedia.org/wiki/Multisample_anti-aliasing
    //----------------------------------------------------------------------------------------------------
    struct MultisampleDesc
    {
        uint32          m_sampleMask{};
        uint32          m_sampleCount = 0;
        bool            m_alphaToCoverage = false;
        bool            m_sampleLocations = false;
    };

    struct ShadingRateDesc
    {
        EShadingRate            m_shadingRate;
        EShadingRateCombiner    m_primitiveCombiner;      
        EShadingRateCombiner    m_attachmentCombiner;
    };

#pragma endregion

//============================================================================================================================================================================================
#pragma region [ Graphics Pipeline: Output Merger ]
//============================================================================================================================================================================================

    enum class EMultiview : uint8
    {
        // Destination "viewport" and/or "layer" must be set in shaders explicitly, "viewMask" for rendering can be < than the one used for pipeline creation (D3D12 style)
        //     Requires "features.m_flexibleMultiview"
        Flexible,

        // View instances go to statically assigned corresponding attachment layers, "viewMask" for rendering must match the one used for pipeline creation (VK style)
        //     Requires "features.layerBasedMultiview"
        LayerBased,

        // View instances go to statically assigned corresponding viewports, "viewMask" for pipeline creation is unused (D3D11 style)
        //     Requires "features.viewportBasedMultiview"
        ViewportBased,
    };

    //----------------------------------------------------------------------------------------------------
    /// @brief : Logical operation used when comparing two colors..
    //  https://registry.khronos.org/vulkan/specs/latest/man/html/VkLogicOp.html
    //  https://learn.microsoft.com/en-us/windows/win32/api/d3d12/ne-d3d12-d3d12_logic_op
    //  S - source color 0
    //  D - destination color
    //----------------------------------------------------------------------------------------------------
    enum class ELogicOp : uint8
    {
        None,
        Clear,              // 0
        And,                // S & D
        AndReverse,         // S & ~D
        Copy,               // S
        AndInverted,        // ~S & D
        Xor,                // S ^ D
        Or,                 // S | D
        Nor,                // ~(S | D)
        Equivalent,         // ~(S ^ D)
        Invert,             // ~D
        OrReverse,          // S | ~D
        CopyInverted,       // ~S
        OrInverted,         // ~S | D
        Nand,               // ~(S | D)
        Set                 // 1
    };

    //----------------------------------------------------------------------------------------------------
    /// @brief : Operation used when comparing the current stencil value with an incoming value.
    // https://registry.khronos.org/vulkan/specs/latest/man/html/VkStencilOp.html
    // https://learn.microsoft.com/en-us/windows/win32/api/d3d12/ne-d3d12-d3d12_stencil_op
    // R - reference, set by "CmdSetStencilReference"
    // D - stencil buffer 
    //----------------------------------------------------------------------------------------------------
    enum class EStencilOp : uint8
    {
        Keep,               // D == D
        Zero,               // D = 0
        Replace,            // D = R
        IncrementAndClamp,  // D = min(D++, 255)
        DecrementAndClamp,  // D = max(D--, 0)
        Invert,             // D = ~D
        IncrementAndWrap,   // D++
        DecrementAndWrap,   // D--
        MaxNum
    };

    //----------------------------------------------------------------------------------------------------
    /// @brief : What kind of input to use when blending Src and Dst color.
    // https://registry.khronos.org/vulkan/specs/latest/man/html/VkBlendFactor.html
    // https://learn.microsoft.com/en-us/windows/win32/api/d3d12/ne-d3d12-d3d12_blend
    // S0 - source color 0
    // S1 - source color 1
    // D - destination color
    // C - blend constants
    //----------------------------------------------------------------------------------------------------
    enum class EBlendFactor : uint8
    {                           // RGB                               ALPHA
        Zero,                   // 0                                 0
        One,                    // 1                                 1
        SrcColor,               // S0.r, S0.g, S0.b                  S0.a
        OneMinusSrcColor,       // 1 - S0.r, 1 - S0.g, 1 - S0.b      1 - S0.a
        DstColor,               // D.r, D.g, D.b                     D.a
        OneMinusDstColor,       // 1 - D.r, 1 - D.g, 1 - D.b         1 - D.a
        SrcAlpha,               // S0.a                              S0.a
        OneMinusSrcAlpha,       // 1 - S0.a                          1 - S0.a
        DstAlpha,               // D.a                               D.a
        OneMinusDstAlpha,       // 1 - D.a                           1 - D.a
        ConstantColor,          // C.r, C.g, C.b                     C.a
        OneMinusConstantColor,  // 1 - C.r, 1 - C.g, 1 - C.b         1 - C.a
        ConstantAlpha,          // C.a                               C.a
        OneMinusConstantAlpha,  // 1 - C.a                           1 - C.a
        SrcAlphaSaturate,       // min(S0.a, 1 - D.a)                1
        Src1Color,              // S1.r, S1.g, S1.b                  S1.a
        OneMinusSrc1Color,      // 1 - S1.r, 1 - S1.g, 1 - S1.b      1 - S1.a
        Src1Alpha,              // S1.a                              S1.a
        OneMinusSrc1Alpha,      // 1 - S1.a                          1 - S1.a
        MaxNum
    };

    //----------------------------------------------------------------------------------------------------
    /// @brief : What operation to use to blend between two EBlendFactors.
    // https://registry.khronos.org/vulkan/specs/latest/man/html/VkBlendOp.html
    // https://learn.microsoft.com/en-us/windows/win32/api/d3d12/ne-d3d12-d3d12_blend_op
    // S - source color
    // D - destination color
    // Sf - source factor, produced by "EBlendFactor"
    // Df - destination factor, produced by "EBlendFactor"
    //----------------------------------------------------------------------------------------------------
    enum class EBlendOp : uint8
    {
        Add,                    // S * Sf + D * Df
        Subtract,               // S * Sf - D * Df
        ReverseSubtract,        // D * Df - S * Sf
        Min,                    // min(S, D)
        Max,                    // max(S, D)
    };

    //----------------------------------------------------------------------------------------------------
    /// @brief : Which color components to use in a blend operation.
    // https://registry.khronos.org/vulkan/specs/latest/man/html/VkColorComponentFlagBits.html
    //----------------------------------------------------------------------------------------------------
    enum class EColorComponentBits : uint8
    {
        None    = 0,
        R       = NES_BIT(0),
        G       = NES_BIT(1),
        B       = NES_BIT(2),
        A       = NES_BIT(3),

        RGB     = R | G | B,
        RGBA    = RGB | A,
    };
    NES_DEFINE_BIT_OPERATIONS_FOR_ENUM(EColorComponentBits);

    //----------------------------------------------------------------------------------------------------
    // https://registry.khronos.org/vulkan/specs/latest/man/html/VkStencilOpState.html
    // https://learn.microsoft.com/en-us/windows/win32/api/d3d12/ns-d3d12-d3d12_depth_stencil_desc 
    //----------------------------------------------------------------------------------------------------
    struct StencilDesc
    {
        ECompareOp                  m_compareOp = ECompareOp::None;     // How to compare depth values.
        EStencilOp                  m_failOp;                           // Value to use on fail.
        EStencilOp                  m_passOp;                           // Value to use on pass.
        EStencilOp                  m_depthFailOp;                      // Value to use on depth fail.
        uint8                       m_writeMask;                        // Which channels to write to.
        uint8                       m_compareMask;                      // Which channels to compare.
    };

    //----------------------------------------------------------------------------------------------------
    /// @brief : Describes how incoming fragment depths should be compared to the current depth buffer. 
    //----------------------------------------------------------------------------------------------------
    struct DepthAttachmentDesc
    {
        ECompareOp                  m_compareOp = ECompareOp::None; // How depth values should be compared. Setting to None disables depth testing.  
        bool                        m_enableWrite = true;           // If true, depth values that pass will be written to the depth buffer.

        // TODO: Requires a feature and dynamic state call.
        //bool                        m_enableBoundsTest = false;
    };

    struct StencilAttachmentDesc
    {
        StencilDesc                 m_front{};
        StencilDesc                 m_back{};             
    };

    //----------------------------------------------------------------------------------------------------
    // https://registry.khronos.org/vulkan/specs/latest/man/html/VkPipelineColorBlendAttachmentState.html
    //
    /// @brief : Describes how blending is calculated.
    ///  Equation: <code> Result = (srcBlendFactor * newColor) <Op> (dstBlendFactor * oldColor); </code>
    ///
    /// Example for Alpha Blending:
    ///
    /// Color Blend Desc:
    ///     - m_srcFactor = EBlendFactor::SrcAlpha;
    ///     - m_dstFactor = EBlendFactor::OneMinusSrcAlpha;
    ///     - m_op        = EBlendOp::Add;
    ///
    /// Alpha Blend Desc:
    ///     - m_srcFactor = EBlendFactor::One;
    ///     - m_dstFactor = EBlendFactor::Zero;
    ///     - m_op        = EBlendOp::Add;
    ///
    /// Code Result: <code>
    ///     finalColor.rgb = (oldColor.a * newColor.rgb) + ((1 - oldColor.a) * oldColor.rgb);
    ///     finalColor.a = (1 * newColor.a) + (0.f * oldColor.a);
    /// </code>
    //----------------------------------------------------------------------------------------------------
    struct BlendDesc
    {
        EBlendFactor                m_srcFactor = EBlendFactor::SrcAlpha;
        EBlendFactor                m_dstFactor = EBlendFactor::OneMinusSrcAlpha;
        EBlendOp                    m_op        = EBlendOp::Add;

        //----------------------------------------------------------------------------------------------------
        /// @brief : Default Blend Description for blending color channels based on alpha. 
        //----------------------------------------------------------------------------------------------------
        static constexpr BlendDesc  ColorAlphaBlend() { return {}; }
        static constexpr BlendDesc  AlphaBlend() { return { EBlendFactor::One, EBlendFactor::Zero, EBlendOp::Add }; }
    };

    //----------------------------------------------------------------------------------------------------
    /// @brief : Defines how a color result from the fragment shader will be blended together with the current
    ///     value in the framebuffer.
    //----------------------------------------------------------------------------------------------------
    struct ColorAttachmentDesc
    {
        EFormat                     m_format            = EFormat::RGBA8_SRGB;          // The format of the color channels.
        BlendDesc                   m_colorBlend        = BlendDesc::ColorAlphaBlend(); // Defines how the r,g,b components will be blended.
        BlendDesc                   m_alphaBlend        = BlendDesc::AlphaBlend();      // Defines how the alpha channel will be blended.
        EColorComponentBits         m_colorWriteMask    = EColorComponentBits::RGBA;    // Defines which color channels will actually be blended.
        bool                        m_enableBlend       = true;                         // Should this blending attachment be enabled.
    };

    //----------------------------------------------------------------------------------------------------
    /// @brief : Defines how fragments are written to the framebuffer. This includes
    ///  how colors are blended together, and whether to perform depth testing to discard fragments that are
    ///  occluded by others in 3D space.
    //----------------------------------------------------------------------------------------------------
    struct OutputMergerDesc
    {
        const ColorAttachmentDesc*  m_pColors = nullptr;
        uint32                      m_colorCount = 0;
        DepthAttachmentDesc         m_depth{};
        StencilAttachmentDesc       m_stencil{};
        EFormat                     m_depthStencilFormat = EFormat::Unknown;

        //  The application can enable a logical operation between the fragment’s color values
        //  and the existing value in the framebuffer attachment. This logical operation is applied
        //  prior to updating the framebuffer attachment. Logical operations are applied only for
        //  signed and unsigned integer and normalized integer framebuffers. Logical operations are
        //  not applied to floating-point or sRGB format color attachments.
        //
        //  The "source" (s) color is the fragment output for the color attachment, and the "dest" (d) color is
        //  the color attachment's RGBA component value. 
        ELogicOp                    m_logicOp = ELogicOp::None;              
    };

    //----------------------------------------------------------------------------------------------------
    /// @brief : Defines the images that will be rendered to for a series of draw calls. Set these attachments
    ///     using CommandBuffer::BeginRendering().
    ///
    ///     Contains 1 or more color attachments (which is commonly the swapchain's color attachment) and
    ///     an optional depth attachment.
    //----------------------------------------------------------------------------------------------------
    struct RenderTargetsDesc
    {
        //----------------------------------------------------------------------------------------------------
        /// @brief : Set the one or more color targets for a series of render commands.
        //----------------------------------------------------------------------------------------------------
        RenderTargetsDesc&          SetColorTargets(const vk::ArrayProxy<const Descriptor*>& colors);
    
        //----------------------------------------------------------------------------------------------------
        /// @brief : Set an optional depth image target. Depth information will be written to this image.
        //----------------------------------------------------------------------------------------------------
        RenderTargetsDesc&          SetDepthStencilTarget(const Descriptor* depthStencil);
    
        //----------------------------------------------------------------------------------------------------
        /// @brief : Returns whether any attachments have been set.
        //----------------------------------------------------------------------------------------------------
        bool                        HasTargets() const { return (!m_colors.empty()) || m_pDepthStencil != nullptr; }
    
        vk::ArrayProxy<const Descriptor*> m_colors{};
        const Descriptor*           m_pDepthStencil = nullptr;  // Optional depth image.
    };

    //----------------------------------------------------------------------------------------------------
    /// @brief : Defines a color or depth stencil values to use to clear a single render target.
    ///     To be used for CommandBuffer::ClearAttachments().
    /// @note : A single Clear Desc can be used for either color or depth, not both!
    //----------------------------------------------------------------------------------------------------
    struct ClearDesc
    {
        //----------------------------------------------------------------------------------------------------
        /// @brief : Set the color that will be used to clear an image's pixels.
        /// @note : By setting the color value, this cannot be used for depth clear values as well!
        //----------------------------------------------------------------------------------------------------
        static ClearDesc        Color(const LinearColor color, const uint32 attachmentIndex = 0);

        //----------------------------------------------------------------------------------------------------
        /// @brief : Set the color that will be used to clear an image's pixels.
        /// @note : By setting the color value, this cannot be used for depth clear values as well!
        //----------------------------------------------------------------------------------------------------
        static ClearDesc        Color(const ClearValue color, const uint32 attachmentIndex = 0);

        //----------------------------------------------------------------------------------------------------
        /// @brief : Creates a clear value for just depth plane.
        //----------------------------------------------------------------------------------------------------
        static ClearDesc        Depth(const float depth);

        //----------------------------------------------------------------------------------------------------
        /// @brief : Creates a clear value for just stencil plane.
        //----------------------------------------------------------------------------------------------------
        static ClearDesc        Stencil(const uint32 stencil);

        //----------------------------------------------------------------------------------------------------
        /// @brief : Set both the depth and stencil clear values.
        //----------------------------------------------------------------------------------------------------
        static ClearDesc        DepthStencil(const float depth, const uint32 stencil);

        //----------------------------------------------------------------------------------------------------
        /// @brief : Set the depth/stencil clear values.
        //----------------------------------------------------------------------------------------------------
        static ClearDesc        DepthStencil(const ClearValue depthStencilValue);

        ClearValue              m_clearValue{};
        EImagePlaneBits         m_planes = EImagePlaneBits::All; 
        uint32                  m_colorAttachmentIndex = 0;
    };

#pragma endregion

//============================================================================================================================================================================================
#pragma region [ Pipelines ]
//============================================================================================================================================================================================

    enum class ERobustness : uint8
    {
        Default,    // Don't care, follow device settings.
        Off,        // No overhead, no robust access (out-of-bounds access is not allowed).
    };

    struct ShaderModuleDesc
    {
        EPipelineStageBits      m_stage = EPipelineStageBits::None;
        std::vector<char>       m_binary{};
        std::string             m_entryPointName{};

        //----------------------------------------------------------------------------------------------------
        /// @brief : Check that this points to a loaded shader. 
        //----------------------------------------------------------------------------------------------------
        bool                    IsValid() const { return m_stage != EPipelineStageBits::None && !m_binary.empty() && !m_entryPointName.empty(); }
    };

    //----------------------------------------------------------------------------------------------------
    /// @brief : Set of shader modules that can be part of a graphics pipeline.
    //----------------------------------------------------------------------------------------------------
    struct GraphicsPipelineShaders
    {
        static constexpr uint32 kGraphicsShaderStagesCount = 7; 
        
        const ShaderModule* m_vertex = nullptr;         // Vertex Shader.
        const ShaderModule* m_fragment = nullptr;       // Fragment Shader.
        const ShaderModule* m_tessControl = nullptr;    // Tesselation Control Shader.
        const ShaderModule* m_tessEval = nullptr;       // Tesselation Evaluation Shader.
        const ShaderModule* m_geometry = nullptr;       // Geometry Shader.
        const ShaderModule* m_meshControl = nullptr;    // Mesh Control (Task) Shader.
        const ShaderModule* m_meshEval = nullptr;       // Mesh Evaluation Shader.
    };

    //----------------------------------------------------------------------------------------------------
    /// @brief : Parameters to create a Pipeline capable of rendering to the screen.
    //----------------------------------------------------------------------------------------------------
    struct GraphicsPipelineDesc
    {
        //----------------------------------------------------------------------------------------------------
        /// @brief : Set the shaders that will run at different stages in the pipeline's execution. 
        //----------------------------------------------------------------------------------------------------
        GraphicsPipelineDesc&           SetShaderStages(const GraphicsPipelineShaders& shaders);

        //----------------------------------------------------------------------------------------------------
        /// @brief : Set the format of the data sent to the vertex shader. 
        //----------------------------------------------------------------------------------------------------
        GraphicsPipelineDesc&           SetVertexInput(const VertexInputDesc& vertexInput);
        
        //----------------------------------------------------------------------------------------------------
        /// @brief : Set how vertices are grouped into geometry. 
        //----------------------------------------------------------------------------------------------------
        GraphicsPipelineDesc&           SetInputAssemblyDesc(const InputAssemblyDesc& desc);

        //----------------------------------------------------------------------------------------------------
        /// @brief : Set how fragments are generated to be colored in by the fragment shader.
        //----------------------------------------------------------------------------------------------------
        GraphicsPipelineDesc&           SetRasterizationDesc(const RasterizationDesc& desc);

        //----------------------------------------------------------------------------------------------------
        /// @brief : Set the Multisample behavior for the pipeline. Since you are setting the behavior,
        ///     this will enable multisampling.
        //----------------------------------------------------------------------------------------------------
        GraphicsPipelineDesc&           SetMultisampleDesc(const MultisampleDesc& desc);

        //----------------------------------------------------------------------------------------------------
        /// @brief : Set whether multisampling should be enabled. It is disabled by default.
        //----------------------------------------------------------------------------------------------------
        GraphicsPipelineDesc&           SetMultisampleEnabled(const bool enabled = true);

        //----------------------------------------------------------------------------------------------------
        /// @brief : Set the behavior for writing resulting fragments to the framebuffer. This includes color
        ///     blending and depth testing.
        //----------------------------------------------------------------------------------------------------
        GraphicsPipelineDesc&           SetOutputMergerDesc(const OutputMergerDesc& desc);

        GraphicsPipelineShaders         m_shaderStages{};
        VertexInputDesc                 m_vertexInput{};
        InputAssemblyDesc               m_inputAssembly{};
        RasterizationDesc               m_rasterization{};
        MultisampleDesc                 m_multisample{};
        OutputMergerDesc                m_outputMerger{};
        bool                            m_enableMultisample = false;
    };

    // [TODO]: 
    struct ComputePipelineDesc
    {
        const PipelineLayout*           m_pPipelineLayout;
        ShaderModuleDesc                      m_shader;
        ERobustness                     m_robustness;       // Optional.
    };

#pragma endregion

//============================================================================================================================================================================================
#pragma region [ Queries ]
//============================================================================================================================================================================================

    //----------------------------------------------------------------------------------------------------
    // https://microsoft.github.io/DirectX-Specs/d3d/CountersAndQueries.html
    // https://registry.khronos.org/vulkan/specs/latest/man/html/VkQueryType.html 
    //----------------------------------------------------------------------------------------------------
    enum class EQueryType : uint8
    {
        Timestamp,                          // uint64
        TimestampCopyQueue,                 // uint64 (requires "features.m_copyQueueTimestamp"), same as "Timestamp" but for a "Copy" queue.
        Occlusion,                          // uint64
        PipelineStatistics,                 // see "PipelineStatisticsDesc" (requires "features.m_pipelineStatistics")
        AccelerationStructureSize,          // uint64, requires "features.m_rayTracing"
        AccelerationStructureCompactedSize, // uint64, requires "features.m_rayTracing"
        MicromapCompactedSize,              // uint64, requires "features.m_micromap"
        MaxNum,
    };

    struct QueryPoolDesc
    {
        EQueryType  m_queryType;
        uint32      m_capacity;
    };

    //----------------------------------------------------------------------------------------------------
    /// @brief : Data layout for EQueryType::PipelineStatistics. 
    //----------------------------------------------------------------------------------------------------
    struct PipelineStatisticsDesc
    {
        // Common Part
        uint64      m_inputVertexCount = 0;                 // The number of input vertices processed in the pipeline. 
        uint64      m_inputPrimitiveCount = 0;              
        uint64      m_vertexShaderCallCount = 0;            // The number of times the Vertex Shader was invoked.
        uint64      m_geometryShaderCallCount = 0;          // The number of times the Geometry Shader was invoked.
        uint64      m_geometryShaderPrimitiveNum = 0;
        uint64      m_rasterizerInPrimitiveNum;
        uint64      m_rasterizerOutPrimitiveNum;
        uint64      m_fragmentShaderCallCount = 0;          // The number of times the Fragment Shader was invoked.
        uint64      m_tessControlShaderCallCount = 0;       // The number of times the Tesselation Control Shader was invoked.
        uint64      m_tessEvaluationShaderCallCount = 0;    // The number of times the Tesselation Evaluation Shader was invoked.
        uint64      m_computeShaderCallCount = 0;           // The number of times the Compute Shader was invoked.

        // If "features.m_meshShaderPipelineStats"
        uint64      m_meshControlShaderCallCount = 0;       // The number of times the Mesh Control Shader was invoked.
        uint64      m_meshEvaluationShaderCallCount = 0;    // The number of times the Mesh Evaluation Shader was invoked.
        uint64      m_meshEvaluationShaderPrimitiveCount = 0;
    };

#pragma endregion

//============================================================================================================================================================================================
#pragma region [ Command Signatures ]
//============================================================================================================================================================================================

    //----------------------------------------------------------------------------------------------------
    /// @brief : Defines the set of vertices and instances that will be drawn.
    //----------------------------------------------------------------------------------------------------
    struct DrawDesc
    {
        DrawDesc() = default;
        DrawDesc(const uint32 numVertices, const uint32 firstVertex = 0, const uint32 numInstances = 1, const uint32 firstInstance = 0);
        
        uint32  m_vertexCount = 0;      // Number of vertices to submit from the attached vertex buffer.
        uint32  m_firstVertex = 0;      // Used as a byte offset into the vertex buffer. Must be calculated as: (sizeof(vertex) * index).                      
        uint32  m_instanceCount = 1;    // Used for instance rendering. Use 1 if you aren't using that.
        uint32  m_firstInstance = 0;    // Used as an offset for instanced rendering.
    };

    //----------------------------------------------------------------------------------------------------
    /// @brief : Parameters to submit an indexed draw call. By binding an index buffer alongside a vertex buffer,
    ///     vertices can be reused, saving on memory.
    //----------------------------------------------------------------------------------------------------
    struct DrawIndexedDesc
    {
        DrawIndexedDesc() = default;
        DrawIndexedDesc(const uint32 numIndices, const uint32 firstIndex = 0, const uint32 firstVertex = 0, const uint32 numInstances = 1, const uint32 firstInstance = 0);
        
        uint64  m_vertexOffset = 0;     // Byte offset to the first vertex in the vertex buffer range. With a value == 0, this will start at the first vertex in the range.                   
        uint32  m_indexCount = 0;       // Number of indices to submit.   
        uint32  m_firstIndex = 0;       // First index in the index buffer.                       
        uint32  m_instanceCount = 1;    // Used for instance rendering. Use 1 if you aren't using that.
        uint32  m_firstInstance = 0;    // Used as an offset for instanced rendering.
    };

    //----------------------------------------------------------------------------------------------------
    /// @brief : Parameters to use when invoking a compute shader.
    //----------------------------------------------------------------------------------------------------
    struct DispatchDesc
    {
        uint32  x = 0;
        uint32  y = 0;
        uint32  z = 0;
    };

    //----------------------------------------------------------------------------------------------------
    /// @brief : Parameters to copy a buffer's data into another. 
    //----------------------------------------------------------------------------------------------------
    struct CopyBufferDesc
    {
        DeviceBuffer*       m_dstBuffer = nullptr;
        uint64              m_dstOffset = 0;
        const DeviceBuffer* m_srcBuffer = nullptr;
        uint64              m_srcOffset = 0;
        uint64              m_size = graphics::kWholeSize;
    };

    //----------------------------------------------------------------------------------------------------
    // [TODO]: Right now, I am forcing the Color Aspect, Buffer Row Length & Buffer image Height == 0
    //
    /// @brief : Parameters to copy a buffer's data to a Device Image.
    //----------------------------------------------------------------------------------------------------
    struct CopyBufferToImageDesc
    {
        DeviceImage*        m_dstImage = nullptr;                           // 
        Int3                m_imageOffset = {0, 0, 0};                // Extent offset in the image to begin copying.
        uint32              m_layerCount = 1;
        EImageLayout        m_dstImageLayout = EImageLayout::CopyDestination;    // The layout that the image will be in at the time of the copy.
        EImagePlaneBits     m_planes = EImagePlaneBits::Color;
        const DeviceBuffer* m_srcBuffer = nullptr;
        uint64              m_srcOffset = 0;
        uint64              m_size = graphics::kWholeSize;
    };

#pragma endregion
    
//============================================================================================================================================================================================
#pragma region [ Device Description ]
//============================================================================================================================================================================================

    enum class EVendor : uint8
    {
        Unknown,
        NVIDIA,
        AMD,
        INTEL,
    };

    enum class EPhysicalDeviceType : uint8
    {
        Unknown,
        CPU,
        VirtualGPU,
        Integrated,
        DiscreteGPU, 
    };

    enum class EQueueType : uint8
    {
        Graphics,
        Compute,
        Transfer,
        MaxNum
    };

    //----------------------------------------------------------------------------------------------------
    // [TODO]: Add a link to the available extensions.
    /// @brief : Contains information about an Extension and its corresponding feature.
    //----------------------------------------------------------------------------------------------------
    struct ExtensionDesc
    {
        const char* m_extensionName = nullptr;      // Name of the extension.
        void*       m_pFeature = nullptr;           // [optional] Pointer to the feature structure for the extension.
        bool        m_isRequired = true;            // [optional] If the extension is required.
        uint32      m_version = 0;                  // [optional] Spec version of the extension, this version or higher.
        bool        m_requireExactVersion = false;  // [optional] If true, the spec version must match exactly.
    };

    //----------------------------------------------------------------------------------------------------
    /// @brief : Describes the Physical Device's properties.
    //----------------------------------------------------------------------------------------------------
    struct PhysicalDeviceDesc
    {
        char                m_name[256]{};                                          /// Name of the Device.
        uint32              m_deviceID{};                                           /// Unique identifier for the Device.
        uint32              m_driverVersion{};
        uint32              m_apiSupport{};
        uint64              m_videoMemorySize{};
        uint64              m_sharedSystemMemorySize{};
        uint32              m_queueCountByType[static_cast<uint32>(EQueueType::MaxNum)];
        uint32              m_queueFamilyIndices[static_cast<uint32>(EQueueType::MaxNum)]; 
        EVendor             m_vendor = EVendor::Unknown;                            /// Vendor that made the device.
        EPhysicalDeviceType m_architecture = EPhysicalDeviceType::Unknown;          /// What type of device it is.
    };

    //----------------------------------------------------------------------------------------------------
    /// @brief : Information about the hardware device.
    //----------------------------------------------------------------------------------------------------
    struct DeviceDesc
    {
        PhysicalDeviceDesc          m_physicalDeviceDesc{};
        Version                     m_apiVersion{};
        std::vector<ExtensionDesc>  m_deviceExtensions{};

        // Capabilities

        // Buffer/Image Dimensions
        struct
        {
            uint32                  m_maxTypedBufferDimension;
            uint16                  m_maxDimensionAttachment;
            uint16                  m_maxAttachmentLayerCount;
            uint16                  m_maxImageDimension1D;
            uint16                  m_maxImageDimension2D;
            uint16                  m_maxImageDimension3D;
            uint16                  m_maxImageLayerCount;
        } m_dimensions;

        // Tiered Feature Support: (0 = unsupported)
        struct
        {
            // 1 - 1/2 pixel uncertainty region and does not support post-snap degenerates
            // 2 - reduces the maximum uncertainty region to 1/256 and requires post-snap degenerates not be culled
            // 3 - maintains a maximum 1/256 uncertainty region and adds support for inner input coverage, aka "SV_InnerCoverage"
            uint8                   m_conservativeRaster = 0;

            // 1 - A single sample pattern can be specified to repeat for every pixel.
            //     1x and 16x sample counts do not support programmable locations.
            // 2 - Four separate sample patterns can be specified for each pixel in a 2x2 grid.
            //     All sample counts support programmable positions.
            uint8                   m_sampleLocations = 0;

            // 1 - DXR 1.0: full raytracing functionality, except features below.
            // 2 - DXR 1.1: adds - ray query, "CmdDispatchRaysIndirect", "GeometryIndex()" intrinsic, additional ray flags and vertex formats.
            // 3 - DXR 1.2: adds - micromap, shader execution reordering.
            uint8                   m_rayTracing = 0;

            // 1 - shading rate can be specified only per draw
            // 2 - adds: per primitive shading rate, per "shadingRateAttachmentTileSize" shading rate, combiners, "SV_ShadingRate" support
            uint8                   m_shadingRate = 0;

            // 1 - unbound arrays with dynamic indexing
            // (2 was a D3D12 setting)...
            uint8                   m_bindless = 0;

            // 0 - ALL descriptors in range must be valid by the time the command list executes
            // 1 - Only "ConstantBuffer" and "Storage" descriptors in range must be valid.
            // 2 - Only referenced descriptors must be valid.
            uint8                   m_resourceBinding = 0;

            // 1 - A "DeviceMemory" object can support resources from all 3 categories: buffers, attachments, all other textures.
            uint8                   m_memory;
        } m_tieredFeatures;

        // Features (Boolean).
        struct
        {
            uint32                  m_getMemoryDesc2                                    : 1 = 0;
            //uint32                  m_enhancedBarriers                                  : 1 = 0;
            uint32                  m_swapChain                                         : 1 = 0;
            uint32                  m_rayTracing                                        : 1 = 0;
            uint32                  m_meshShader                                        : 1 = 0;
            uint32                  m_lowLatency                                        : 1 = 0;
            uint32                  m_micromap                                          : 1 = 0;
            
            uint32                  m_independentFrontAndBackStencilReferenceAndMasks   : 1 = 0;
            uint32                  m_textureFilterMinMax                               : 1 = 0;
            uint32                  m_logicOp                                           : 1 = 0;
            uint32                  m_depthBoundsTest                                   : 1 = 0;
            uint32                  m_drawIndirectCount                                 : 1 = 0;
            uint32                  m_lineSmoothing                                     : 1 = 0;
            uint32                  m_copyQueueTimestamp                                : 1 = 0;
            uint32                  m_meshShaderPipelineStats                           : 1 = 0;
            uint32                  m_dynamicDepthBias                                  : 1 = 0;
            uint32                  m_additionalShadingRates                            : 1 = 0;
            uint32                  m_viewportOriginBottomLeft                          : 1 = 0;
            uint32                  m_regionResolve                                     : 1 = 0;
            uint32                  m_flexibleMultiview                                 : 1 = 0;
            uint32                  m_layerBasedMultiview                               : 1 = 0;
            uint32                  m_viewportBasedMultiview                            : 1 = 0;
            uint32                  m_presentFromCompute                                : 1 = 0;
            uint32                  m_waitableSwapChain                                 : 1 = 0;
            uint32                  m_pipelineStatistics                                : 1 = 0;
            
            uint32                  m_descriptorIndexing                                : 1 = 0;
            uint32                  m_deviceAddress                                     : 1 = 0;
            uint32                  m_swapchainMutableFormat                            : 1 = 0;
            uint32                  m_presentId                                         : 1 = 0;
            uint32                  m_memoryPriority                                    : 1 = 0;
            uint32                  m_memoryBudget                                      : 1 = 0;
            uint32                  m_maintenance4                                      : 1 = 0;
            uint32                  m_maintenance5                                      : 1 = 0;
            uint32                  m_maintenance6                                      : 1 = 0;
            uint32                  m_imageSlicedView                                   : 1 = 0;
            uint32                  m_customBorderColor                                 : 1 = 0;
            uint32                  m_robustness                                        : 1 = 0;
            uint32                  m_robustness2                                       : 1 = 0;
            uint32                  m_pipelineRobustness                                : 1 = 0;
            uint32                  m_swapchainMaintenance1                             : 1 = 0;
            uint32                  m_fifoLatestReady                                   : 1 = 0;
        } m_features;

        // Memory Properties
        struct
        {
            uint64                  m_deviceUploadHeapSize          = 0; // ReBAR
            uint32                  m_allocationMaxNum              = 0;
            uint32                  m_samplerAllocationMaxNum       = 0;
            uint32                  m_constantBufferMaxRange        = 0;
            uint32                  m_storageBufferMaxRange         = 0;
            uint32                  m_bufferImageGranularity      = 0;
            uint64                  m_bufferMaxSize                 = 0;
        } m_memory;

        // Memory Alignment Properties
        struct
        {
            uint32                  m_uploadBufferImageRow        = 0;
            uint32                  m_uploadBufferImageSlice      = 0;
            uint32                  m_shaderBindingTable            = 0;
            uint32                  m_bufferShaderResourceOffset    = 0;
            uint32                  m_constantBufferOffset          = 0;
            uint32                  m_scratchBufferOffset           = 0;
            uint32                  m_accelerationStructureOffset   = 0;
            uint32                  m_micromapOffset                = 0;
        } m_memoryAlignment;
        
        
        // [TODO]: Shader Features

        struct
        {
            uint8                   m_maxColorSamples = 0;          // Maximum samples allowable for the color attachment.
            uint8                   m_maxDepthSamples = 0;          // Maximum samples allowable for the depth attachment.
            float                   m_maxSamplerAnisotropy = 0.f;
        } m_other;
        
    };
#pragma endregion
}

#include "GraphicsHelpers.h"