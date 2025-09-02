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

// Forward Declare VMA types.
struct VmaAllocator_T;
struct VmaAllocation_T;

namespace nes
{
    
//============================================================================================================================================================================================
#pragma region [ Common ]
//============================================================================================================================================================================================

    namespace graphics
    {
        /// Special value that marks that the remaining amount of some span (number of mips, number of image layers, etc.) should be used. 
        static constexpr uint16 kUseRemaining = std::numeric_limits<uint16>::max();

        /// Special value to use if you want to use the entire device buffer's range.
        static constexpr uint64 kWholeSize = std::numeric_limits<uint64>::max();
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

    struct DepthStencil
    {
        float           m_depth = 0.f;
        uint8           m_stencil = 0;
    };

    union ClearColor
    {
        Vec4            m_float32{};
        UVec4           m_uint32;
        IVec4           m_int32;
    };

    union ClearValue
    {
        ClearColor      m_color;
        DepthStencil    m_depthStencil;
    };

    struct SampleLocation
    {
        int8 x, y = 0;
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
        All                     = 0,
        None                    = 0x7FFFFFFF,

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

        // Grouped Stages
        TessellationShaders     = TessControlShader | TessEvaluationShader,
        MeshShaders             = MeshControlShader | MeshEvaluationShader,
        GraphicsShaders         = VertexShader | TessellationShaders | GeometryShader | MeshShaders | FragmentShader,
        Draw                    = IndexInput | GraphicsShaders | DepthStencilAttachment | ColorAttachment,
        RayTracingShaders       = RayGenShader | MissShader | IntersectionShader | ClosestHitShader | AnyHitShader | CallableShader, 
    };
    NES_DEFINE_BIT_OPERATIONS_FOR_ENUM(EPipelineStageBits)

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
        ConstantBuffer              = NES_BIT(2),   // Read         GraphicsShaders, ComputeShader, RayTracingShaders
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
    enum class ELayout : uint8
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

#pragma endregion

//============================================================================================================================================================================================
#pragma region [ Resources: creation ]
//============================================================================================================================================================================================

    enum class EImageType : uint8
    {
        Image1D,
        Image2D,
        Image3D,
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

    struct ImageDesc
    {
        EImageType          m_type = EImageType::Image2D;
        EImageUsageBits     m_usage = EImageUsageBits::ShaderResource;
        EFormat             m_format = EFormat::Unknown;
        uint32              m_width = 1;
        uint32              m_height = 1;
        uint32              m_depth = 1;
        uint32              m_mipCount = 1;
        uint32              m_layerCount = 1;
        uint32              m_sampleCount = 1;
        ESharingMode        m_sharingMode = ESharingMode::Exclusive;
        ClearValue          m_clearValue{};

        //----------------------------------------------------------------------------------------------------
        /// @brief : Ensures that ranges are valid. 
        //----------------------------------------------------------------------------------------------------
        void                Validate();
    };

    //----------------------------------------------------------------------------------------------------
    /// @brief : Used to create a buffer resource.
    //  Available "m_structuredStride" Values:
    //      0   = allows "typed" views.
    //      4   = allows "types", "byte address" (raw) and "structured" views.
    //      >4  = allows "structured" and potentially "typed" views.
    //----------------------------------------------------------------------------------------------------
    struct BufferDesc
    {
        uint64              m_size = 0;                                 // Byte size of the buffer.
        uint32              m_structuredStride = 0;                     // 
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

        //----------------------------------------------------------------------------------------------------
        /// @brief : Default description for a Vertex Buffer. The buffer will be allocated on Device, and is expected
        ///     to be used with the StagingUploader to set its data.
        ///	@param vertexCount : Number of vertices for the buffer. 
        ///	@param vertexSize : Size a single vertex, in bytes; "sizeof(VertexType". 
        //----------------------------------------------------------------------------------------------------
        static AllocateBufferDesc   VertexBuffer(const uint64 vertexCount, const uint32 vertexSize);

        //----------------------------------------------------------------------------------------------------
        /// @brief : Default description for an Index Buffer. The buffer will be allocated on Device, and is expected
        ///     to be used with the StagingUploader to set its data.
        ///	@param indexCount : Number of indices in the buffer. 
        ///	@param type : Type of index to use. Default is uint32.
        //----------------------------------------------------------------------------------------------------
        static AllocateBufferDesc   IndexBuffer(const uint64 indexCount, const EIndexType type = EIndexType::U32);
    };

    //----------------------------------------------------------------------------------------------------
    /// @brief : Parameters for allocating a DeviceImage object.
    //----------------------------------------------------------------------------------------------------
    struct AllocateImageDesc
    {
        // Image Description, including usage and dimensions.
        ImageDesc           m_desc{};
        
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

    //----------------------------------------------------------------------------------------------------
    // https://registry.khronos.org/vulkan/specs/latest/man/html/VkImageAspectFlagBits.html 
    //----------------------------------------------------------------------------------------------------
    enum class EPlaneBits : uint8
    {
        All     = 0,
        Color   = NES_BIT(0),   // indicates "color" plane (same as "ALL" for color formats)
        Depth   = NES_BIT(1),   // indicates "depth" plane (same as "ALL" for depth-only formats)
        Stencil = NES_BIT(2),   // indicates "stencil" plane in depth-stencil formats
    };
    NES_DEFINE_BIT_OPERATIONS_FOR_ENUM(EPlaneBits)
    
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
        Constant
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
        uint32      x;
        uint32      y;
        uint32      z;
        uint32      m_width;
        uint32      m_height;
        uint32      m_depth;
        uint32      m_mipOffset;
        uint32      m_layerOffset;
        EPlaneBits  m_planes;
    };

    //----------------------------------------------------------------------------------------------------
    /// @brief : Describes access to a 1D Image resource. Used to create a Descriptor.
    //----------------------------------------------------------------------------------------------------
    struct Image1DViewDesc
    {
        const DeviceImage*  m_pImage;
        EImage1DViewType    m_viewType;
        EFormat             m_format = EFormat::Unknown;
        uint16              m_baseMipLevel = 0;
        uint16              m_mipCount = graphics::kUseRemaining;
        uint16              m_baseLayer = 0;
        uint16              m_layerCount = graphics::kUseRemaining;
    };

    //----------------------------------------------------------------------------------------------------
    /// @brief : Describes access to a 2D Texture resource. Used to create a Descriptor.
    //----------------------------------------------------------------------------------------------------
    struct Image2DViewDesc
    {
        const DeviceImage*  m_pImage = nullptr;
        EImage2DViewType    m_viewType;
        EFormat             m_format = EFormat::Unknown;
        uint16              m_baseMipLevel = 0;
        uint16              m_mipCount = graphics::kUseRemaining;
        uint16              m_baseLayer = 0;
        uint16              m_layerCount = graphics::kUseRemaining;
    };

    //----------------------------------------------------------------------------------------------------
    /// @brief : Describes access to a 3D Image resource. Used to create a Descriptor.
    //----------------------------------------------------------------------------------------------------
    struct Image3DViewDesc
    {
        const DeviceImage*  m_pImage = nullptr;
        EImage3DViewType    m_viewType;
        EFormat             m_format = EFormat::Unknown;
        uint16              m_baseMipLevel = 0;
        uint16              m_mipCount = graphics::kUseRemaining;
        uint16              m_baseSlice = 0;
        uint16              m_sliceCount = graphics::kUseRemaining;
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

        // Optional
        uint32              m_structuredStride; // This will equal the structure stride from "BufferDesc" if not provided.
    };

    struct AddressModes
    {
        EAddressMode u;
        EAddressMode v;
        EAddressMode w;
    };

    struct Filters
    {
        EFilterType m_min;
        EFilterType m_mag;
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
        float               m_mipBias;
        float               m_mipMin;
        float               m_mipMax;
        AddressModes        m_addressModes;
        ECompareOp          m_compareOp;
        ClearColor          m_borderColor;
        bool                m_isInteger;
    };

#pragma endregion

//============================================================================================================================================================================================
#pragma region [ Pipeline Layout and Descriptors Management ]
//============================================================================================================================================================================================

    // [TODO]: 
    enum class EPipelineLayoutBits : uint8
    {
        None                                = 0,
        IgnoreGlobalSPIRVOffsets            = NES_BIT(0), // VK: ignore "DeviceCreationDesc::vkBindingOffsets"
        EnableD3D12DrawParametersEmulation  = NES_BIT(1)  // D3D12: enable draw parameters emulation, not needed if all vertex shaders for this layout compiled with SM 6.8 (native support)
    };
    NES_DEFINE_BIT_OPERATIONS_FOR_ENUM(EPipelineLayoutBits);

    // [TODO]: 
    enum class EDescriptorPoolBits : uint8
    {
        None                                = 0,
        AllowUpdateAfterSet                 = NES_BIT(0)  // Allows "DescriptorSetBits::AllowUpdateAfterSet"
    };
    NES_DEFINE_BIT_OPERATIONS_FOR_ENUM(EDescriptorPoolBits);

    // [TODO]: 
    enum class EDescriptorSetBits : uint8
    {
        None                                = 0,
        AllowUpdateAfterSet                 = NES_BIT(0)  // Allows "DescriptorRangeBits::AllowUpdateAfterSet".
    };
    NES_DEFINE_BIT_OPERATIONS_FOR_ENUM(EDescriptorSetBits);

    //----------------------------------------------------------------------------------------------------
    // https://registry.khronos.org/vulkan/specs/latest/man/html/VkDescriptorBindingFlagBits.html
    //----------------------------------------------------------------------------------------------------
    enum class EDescriptorRangeBits : uint8
    {
        None                                = 0,
        PartiallyBound                      = NES_BIT(0),   // Descriptors in range may not contain valid descriptors at the time the descriptors are consumed (but referenced descriptors must be valid).
        Array                               = NES_BIT(1),   // Descriptors in range are organized into an array.
        VariableSizedArray                  = NES_BIT(2),   // Descriptors in range are organized into a variable-sized array, which size is specified via "variableDescriptorNum" argument of "AllocateDescriptorSets" function.

        // https://docs.vulkan.org/samples/latest/samples/extensions/descriptor_indexing/README.html#_update_after_bind_streaming_descriptors_concurrently
        AllowUpdateAfterSet                 = NES_BIT(4),   // Descriptors in range can be updated after "CmdSetDescriptorSet" but before "QueueSubmit", also works as "DataVolatile"
    };
    NES_DEFINE_BIT_OPERATIONS_FOR_ENUM(EDescriptorRangeBits);

    //----------------------------------------------------------------------------------------------------
    // https://registry.khronos.org/vulkan/specs/latest/man/html/VkDescriptorType.html
    //----------------------------------------------------------------------------------------------------
    enum class EDescriptorType
    {
        None,
        Sampler                 = vk::DescriptorType::eSampler,            
        ConstantBuffer          = vk::DescriptorType::eUniformBuffer,
        Texture                 = vk::DescriptorType::eSampledImage,
        StorageTexture          = vk::DescriptorType::eStorageImage,     
        Buffer                  = vk::DescriptorType::eUniformTexelBuffer,
        StorageBuffer           = vk::DescriptorType::eStorageTexelBuffer,
        AccelerationStructure   = vk::DescriptorType::eAccelerationStructureKHR,
    };

    //----------------------------------------------------------------------------------------------------
    /// @brief : A Descriptor Range consists of "Descriptor" entities. 
    //----------------------------------------------------------------------------------------------------
    struct DescriptorRangeDesc
    {
        uint32                      m_baseRegisterIndex;
        uint32                      m_descriptorNum;        /// Treated as max size if "VariableSizedArray" flag is set
        EDescriptorType             m_descriptorType;
        EPipelineStageBits          m_shaderStages;
        EDescriptorRangeBits        m_flags;
    };

    struct DynamicConstantBufferDesc
    {
        uint32                      m_registerIndex;
        EPipelineStageBits          m_shaderStages;
    };

    //----------------------------------------------------------------------------------------------------
    /// @brief : "DescriptorSet" consists of "DescriptorRange" entities. 
    //----------------------------------------------------------------------------------------------------
    struct DescriptorSetDesc
    {
        uint32                      m_registerSpace; // Must be unique; avoid big gaps.
        const DescriptorRangeDesc*  m_pRanges;
        uint32                      m_rangeNum;
        const DynamicConstantBufferDesc* m_pDynamicConstantBuffers; // A dynamic constant buffer allows you to dynamically specify an offset in the buffer via "CmdSetDescriptorSet" call.
        uint32                      m_dynamicConstantBufferNum;
        EDescriptorSetBits          m_flags;
    };

    //----------------------------------------------------------------------------------------------------
    /// @brief : "PipelineLayout" consists of "DescriptorSet" descriptions and root parameters.
    ///     This is also known as a "push constants block".
    //----------------------------------------------------------------------------------------------------
    struct PushConstantDesc
    {
        uint32                      m_registerIndex;
        uint32                      m_size;
        EPipelineStageBits          m_shaderStages;
    };

    //----------------------------------------------------------------------------------------------------
    /// @brief : "PipelineLayout" consists of "DescriptorSet" descriptions and root parameters.
    ///     This is also known as a "push descriptor".
    //----------------------------------------------------------------------------------------------------
    struct PushDescriptorDesc
    {
        uint32                      m_registerIndex;
        EDescriptorType             m_descriptorType; // Can be ConstantBuffer, StructuredBuffer or StorageStructuredBuffer.
        EPipelineStageBits          m_shaderStages;
    };

    //----------------------------------------------------------------------------------------------------
    // https://registry.khronos.org/vulkan/specs/latest/man/html/VkPipelineLayoutCreateInfo.html
    //----------------------------------------------------------------------------------------------------
    struct PipelineLayoutDesc
    {
        // [TODO]: Descriptor Sets.
        // [TODO]: Push Constant Ranges.
        
        // What stages of the pipeline should be bound?
        EPipelineStageBits          m_shaderStages = EPipelineStageBits::GraphicsShaders;
    };

    //----------------------------------------------------------------------------------------------------
    // https://learn.microsoft.com/en-us/windows/win32/api/d3d12/ns-d3d12-d3d12_descriptor_heap_desc
    // https://registry.khronos.org/vulkan/specs/latest/man/html/VkDescriptorPoolCreateInfo.html
    //----------------------------------------------------------------------------------------------------
    struct DescriptorPoolDesc
    {
        uint32                      m_descriptorSetMaxNum;      
        uint32                      m_samplerMaxNum;      
        uint32                      m_constantBufferMaxNum;      
        uint32                      m_dynamicConstantBufferMaxNum;      
        uint32                      m_textureMaxNum;      
        uint32                      m_storageTextureMaxNum;
        uint32                      m_bufferMaxNum;      
        uint32                      m_storageBufferMaxNum;      
        uint32                      m_structuredBufferMaxNum;      
        uint32                      m_storageStructuredBufferMaxNum;
        uint32                      m_accelerationStructureMaxNum;
        EDescriptorPoolBits         m_flags;
    };

    //----------------------------------------------------------------------------------------------------
    /// @brief : Used to update descriptors in a descriptor set, allocated from a descriptor pool. 
    //----------------------------------------------------------------------------------------------------
    struct DescriptorRangeUpdateDesc
    {
        const Descriptor* const*    m_pDescriptors;
        uint32                      m_descriptorNum;
        uint32                      m_baseDescriptor;
    };

    struct DescriptorSetCopyDesc
    {
        //const DescriptorSet*        m_pSrcDescriptorSet;
        uint32                      m_srcBaseRange;
        uint32                      m_dstBaseRange;
        uint32                      m_rangeNum;
        uint32                      m_srcBaseDynamicConstantBuffer;
        uint32                      m_dstBaseDynamicConstantBuffer;
        uint32                      m_dynamicConstantBufferNum;
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
        uint16                  m_streamIndex = 0;  // Which stream index the per-vertex data comes through to the shader.
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
        uint16                      m_bindingIndex = 0;                             // Specifies the index in the array of vertex bindings.
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
        ///	@param type : Type of index that is stored. Default is uint32.
        ///	@param bufferOffset : Byte offset from the beginning of the DeviceBuffer to get to first index.
        ///     Default is 0; the start of the buffer.
        //----------------------------------------------------------------------------------------------------
        IndexBufferRange(DeviceBuffer* pBuffer, const uint64 indexCount, const EIndexType type = EIndexType::U32, const uint64 bufferOffset = 0);
        
        //----------------------------------------------------------------------------------------------------
        /// @brief : Get the type of indices that this index buffer contains.
        //----------------------------------------------------------------------------------------------------
        EIndexType          GetIndexType() const       { return m_indexType; }

        //----------------------------------------------------------------------------------------------------
        /// @brief : Get the number of indices in the range.
        //----------------------------------------------------------------------------------------------------
        uint32              GetNumIndices() const    { return m_indexCount; }
    
    private:
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
    // 
    // enabled if constant != 0 or slope != 0
    //----------------------------------------------------------------------------------------------------
    struct DepthBiasDesc
    {
        float           m_constant  = 0.f;
        float           m_clamp     = 0.f;
        float           m_slope     = 1.f;

        bool            IsEnabled() const { return m_constant != 0.f || m_slope != 0.f; }
    };

    //----------------------------------------------------------------------------------------------------
    /// @brief : The rasterizer takes the geometry shaped by the vertices from the vertex shader and turns it
    ///     into fragments to be colored by the fragment shader. It also performs depth testing, face culling,
    ///     and the scissor test, and it can be configured to output fragments that fill entire polygons or
    ///     just the edges (wireframe rendering).
    //----------------------------------------------------------------------------------------------------
    struct RasterizationDesc
    {
        DepthBiasDesc       m_depthBias;
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
        uint32          m_sampleMask;
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
    // C - blend constants, set by "CmdSetBlendConstants"
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
        ECompareOp                  m_compareOp;        // How to compare depth values.
        EStencilOp                  m_failOp;           // Value to use on fail.
        EStencilOp                  m_passOp;           // Value to use on pass.
        EStencilOp                  m_depthFailOp;      // Value to use on depth fail.
        uint8                       m_writeMask;        // Which color channels to write to.
        uint8                       m_compareMask;      // Which color channels to compare.
    };

    //----------------------------------------------------------------------------------------------------
    /// @brief : Describes how incoming fragment depths should be compared to the current depth buffer. 
    //----------------------------------------------------------------------------------------------------
    struct DepthAttachmentDesc
    {
        ECompareOp                  m_compareOp = ECompareOp::Less; // How depth values should be compared. Default behavior is 'lesser' values are preferred.  
        bool                        m_enableWrite = true;           // If true, depth values that pass will be written to the depth buffer.
        bool                        m_enableBoundsTest = false;     // Requires "features.m_depthBoundsTest", expects "CmdSetDepthBounds".   
    };

    struct StencilAttachmentDesc
    {
        StencilDesc                 m_front;
        StencilDesc                 m_back;             // Requires "features.m_independentFrontAndBackStencilReferenceAndMasks" for "back.m_writeMask"
    };

    //----------------------------------------------------------------------------------------------------
    // https://registry.khronos.org/vulkan/specs/latest/man/html/VkPipelineColorBlendAttachmentState.html
    // https://learn.microsoft.com/en-us/windows/win32/api/d3d12/ns-d3d12-d3d12_render_target_blend_desc
    //----------------------------------------------------------------------------------------------------
    struct BlendDesc
    {
        EBlendFactor                m_srcFactor = EBlendFactor::SrcAlpha;
        EBlendFactor                m_dstFactor = EBlendFactor::OneMinusSrc1Alpha;
        EBlendOp                    m_op        = EBlendOp::Add;

        //----------------------------------------------------------------------------------------------------
        /// @brief : Default Blend Description for blending color channels based on alpha. 
        //----------------------------------------------------------------------------------------------------
        static constexpr BlendDesc  ColorAlphaBlend() { return {}; }

        //----------------------------------------------------------------------------------------------------
        /// @brief : Default Blend Description for blending alpha channels for an 'alpha blending' color attachment. 
        //----------------------------------------------------------------------------------------------------
        static constexpr BlendDesc  AlphaBlend()      { return {.m_srcFactor = EBlendFactor::One, .m_dstFactor = EBlendFactor::Zero, .m_op = EBlendOp::Add }; }
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
        const ColorAttachmentDesc*  m_pColors;
        uint32                      m_colorNum;
        DepthAttachmentDesc         m_depth;
        StencilAttachmentDesc       m_stencil;
        EFormat                     m_depthStencilFormat;
        ELogicOp                    m_logicOp = ELogicOp::None;              
    
        // Optional
        uint32                      m_viewMask;             /// If non-0, requires "viewMaximum > 0".
        EMultiview                  m_multiView;            /// if "m_viewMask != 0", requires "features.(xxx)Muliview".
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
        RenderTargetsDesc&          SetDepthStencilTargets(const Descriptor* depthStencil);

        //----------------------------------------------------------------------------------------------------
        /// @brief : Returns whether any attachments have been set.
        //----------------------------------------------------------------------------------------------------
        bool                        HasTargets() const { return (!m_colors.empty()) || m_pDepthStencil != nullptr; }

        vk::ArrayProxy<const Descriptor*> m_colors{};
        const Descriptor*           m_pDepthStencil = nullptr;  // Optional depth image.
    };

    //----------------------------------------------------------------------------------------------------
    /// @brief : Defines any of color, depth and stencil values to use to clear a single render target.
    ///     To be used for CommandBuffer::ClearAttachments().
    //----------------------------------------------------------------------------------------------------
    struct ClearDesc
    {
        //----------------------------------------------------------------------------------------------------
        /// @brief : Set the color that will be used to clear an image's pixels.
        ///     This will add the vk::ImageAspectFlags::eColor flag to the aspect.
        //----------------------------------------------------------------------------------------------------
        ClearDesc&              SetColorValue(const vk::ClearColorValue& color, const uint32 attachmentIndex = 0);

        //----------------------------------------------------------------------------------------------------
        /// @brief : Set the clear value for depth.
        ///     This will add the vk::ImageAspectFlags::eDepth flag to the aspect.
        //----------------------------------------------------------------------------------------------------
        ClearDesc&              SetDepthValue(const float depth);

        //----------------------------------------------------------------------------------------------------
        /// @brief : Set the clear for the stencil.
        ///     This will add the vk::ImageAspectFlags::eStencil flag to the aspect.
        //----------------------------------------------------------------------------------------------------
        ClearDesc&              SetStencilValue(const uint32 stencil);

        //----------------------------------------------------------------------------------------------------
        /// @brief : Set both the depth and stencil clear values.
        ///     This will add both the vk::ImageAspectFlags::eDepth & eStencil flags to the aspect.
        //----------------------------------------------------------------------------------------------------
        ClearDesc&              SetDepthStencilValue(const float depth, const uint32 stencil);
        
        vk::ClearValue          m_clearValue            = {};
        vk::ImageAspectFlags    m_aspect                = vk::ImageAspectFlagBits::eNone;
        uint32                  m_colorAttachmentIndex  = 0;
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

    struct ShaderDesc
    {
        EPipelineStageBits      m_stage = EPipelineStageBits::GraphicsShaders;
        const void*             m_pByteCode = nullptr;
        uint64                  m_size = 0;
        const char*             m_entryPointName = "main";

        //----------------------------------------------------------------------------------------------------
        /// @brief : Check that this points to a loaded shader. 
        //----------------------------------------------------------------------------------------------------
        bool                    IsValid() const { return m_pByteCode != nullptr && m_size > 0; }
    };

    //----------------------------------------------------------------------------------------------------
    /// @brief : Parameters to create a Pipeline capable of rendering to the screen.
    //----------------------------------------------------------------------------------------------------
    struct GraphicsPipelineDesc
    {
        //----------------------------------------------------------------------------------------------------
        /// @brief : Set the shaders that will run at different stages in the pipeline's execution. 
        //----------------------------------------------------------------------------------------------------
        GraphicsPipelineDesc&           SetShaderStages(const std::vector<ShaderDesc>& shaderStages);

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
        
        std::vector<ShaderDesc>         m_shaderStages{};
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
        ShaderDesc                      m_shader;
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
        uint32  m_vertexCount = 0;      // Number of vertices to submit from the attached vertex buffer.
        uint32  m_firstVertex = 0;      // Used as a byte offset into the vertex buffer. Must be calculated as: (sizeof(vertex) * index).                      
        uint32  m_instanceCount = 1;    // Used for instance rendering. Use 1 if you aren't using that.
        uint32  m_firstInstance = 0;    // Used as an offset for instanced rendering.

        DrawDesc(const uint32 numVertices, const uint32 firstVertex = 0, const uint32 numInstances = 1, const uint32 firstInstance = 0);
    };

    //----------------------------------------------------------------------------------------------------
    /// @brief : Parameters to submit an indexed draw call. By binding an index buffer alongside a vertex buffer,
    ///     vertices can be reused, saving on memory.
    //----------------------------------------------------------------------------------------------------
    struct DrawIndexedDesc
    {
        uint32  m_firstVertex = 0;      // Used as a byte offset into the vertex buffer. Must be calculated as: (sizeof(vertex) * index).                   
        uint32  m_indexCount = 0;       // Number of indices to submit.   
        uint32  m_firstIndex = 0;       // Used as a byte offset into the index buffer. Must be calculated as: (sizeof(index) * index).                       
        uint32  m_instanceCount = 1;    // Used for instance rendering. Use 1 if you aren't using that.
        uint32  m_firstInstance = 0;    // Used as an offset for instanced rendering.
        
        DrawIndexedDesc(const uint32 numIndices, const uint32 firstIndex = 0, const uint32 firstVertex = 0, const uint32 numInstances = 1, const uint32 firstInstance = 0);
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
        
    };
#pragma endregion
}

#include "GraphicsHelpers.h"