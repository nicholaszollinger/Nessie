// Common.h
#pragma once
#include "GraphicsCore.h"
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
    namespace graphics
    {
        /// Special value that marks that the remaining amount of some span (number of mips, number of image layers, etc.) should be used. 
        static constexpr uint32 kUseRemaining = ~0UL;
    }
//============================================================================================================================================================================================
#pragma region [ Common ]
//============================================================================================================================================================================================

    struct Viewport
    {
        float m_minX;
        float m_maxX;
        float m_minY;
        float m_maxY;
        float m_minZ;
        float m_maxZ;
    
        Viewport() : m_minX(0.0f), m_maxX(0.f), m_minY(0.f), m_maxY(0.f), m_minZ(0.f), m_maxZ(1.f) {}
        Viewport(const float width, const float height) : m_minX(0.0f), m_maxX(width), m_minY(0.f), m_maxY(height), m_minZ(0.f), m_maxZ(1.f) {}
        Viewport(const float minX, const float maxX, const float minY, const float maxY, const float minZ, const float maxZ) : m_minX(minX), m_maxX(maxX), m_minY(minY), m_maxY(maxY), m_minZ(minZ), m_maxZ(maxZ) {}

        bool operator==(const Viewport& other) const
        {
            return m_minX == other.m_minX
                && m_maxX == other.m_maxX
                && m_minY == other.m_minY
                && m_maxY == other.m_maxY
                && m_minZ == other.m_minZ
                && m_maxZ == other.m_maxZ;
        }
        bool operator!=(const Viewport& other) const { return !(*this == other); }

        float Width() const     { return m_maxX - m_minX; }
        float Height() const    { return m_maxY - m_minY; }
        float Depth() const     { return m_maxZ - m_minZ; }
    };

    struct Rect
    {
        int m_minX;
        int m_maxX;
        int m_minY;
        int m_maxY;

        Rect() : m_minX(0), m_maxX(0), m_minY(0), m_maxY(0) {}
        Rect(const int width, const int height) : m_minX(0), m_maxX(width), m_minY(0), m_maxY(height) {}
        Rect(const int minX, const int maxX, const int minY, const int maxY) : m_minX(minX), m_maxX(maxX), m_minY(minY), m_maxY(maxY) {}
        explicit Rect(const Viewport& viewport)
            : m_minX(static_cast<int>(std::floorf(viewport.m_minX)))
            , m_maxX(static_cast<int>(std::ceilf(viewport.m_maxX)))
            , m_minY(static_cast<int>(std::floorf(viewport.m_minY)))
            , m_maxY(static_cast<int>(std::ceilf(viewport.m_maxY)))
        {
            //
        }

        bool operator==(const Rect& other) const { return m_minX == other.m_minX && m_maxX == other.m_maxX && m_minY == other.m_minY && m_maxY == other.m_maxY; }
        bool operator!=(const Rect& other) const { return !(*this == other); }

        int Width() const { return m_maxX - m_minX; }
        int Height() const { return m_maxY - m_minY; }
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
#pragma region [ Formats ]
//============================================================================================================================================================================================

    //----------------------------------------------------------------------------------------------------
    /// @brief : Data format types. Includes information about what data types support which formats.
    ///     Types are written in the bit order: left -> right : low -> high bits.
    // Type Suffixes:
    //     SINT    - Signed int.
    //     UINT    - Unsigned int.
    //     UNORM   - Unsigned floating point [0.f, 1.f].
    //     SFLOAT  - Signed float.
    //
    // More Info:
    // - https://registry.khronos.org/vulkan/specs/latest/man/html/VkFormat.html
    // - https://learn.microsoft.com/en-us/windows/win32/api/dxgiformat/ne-dxgiformat-dxgi_format
    //
    // Support Key:
    // Expected (but not guaranteed) "FormatSupportBits" are provided, but "GetFormatSupport" should be used for querying real hardware support
    // To demote sRGB use the previous format, i.e. "format - 1"
    //                                               STORAGE_BUFFER_ATOMICS
    //                                                     VERTEX_BUFFER  |
    //                                                 STORAGE_BUFFER  |  |
    //                                                      BUFFER  |  |  |
    //                                  STORAGE_TEXTURE_ATOMICS  |  |  |  |
    //                                                 BLEND  |  |  |  |  |
    //                           DEPTH_STENCIL_ATTACHMENT  |  |  |  |  |  |
    //                                COLOR_ATTACHMENT  |  |  |  |  |  |  |
    //                              STORAGE_TEXTURE  |  |  |  |  |  |  |  |
    //                                   TEXTURE  |  |  |  |  |  |  |  |  |
    //                                         |  |  |  |  |  |  |  |  |  |
    //                                         |    FormatSupportBits     | 
    //----------------------------------------------------------------------------------------------------
    enum class EFormat
    {
        Unknown,                            // -  -  -  -  -  -  -  -  -  -

        // Plain: 8 bits per channel
        R8_UNORM,                           // +  +  +  -  +  -  +  +  +  -
        R8_SNORM,                           // +  +  +  -  +  -  +  +  +  -
        R8_UINT,                            // +  +  +  -  -  -  +  +  +  - // SHADING_RATE compatible, see NRI_SHADING_RATE macro
        R8_SINT,                            // +  +  +  -  -  -  +  +  +  -

        RG8_UNORM,                          // +  +  +  -  +  -  +  +  +  - // "AccelerationStructure" compatible (requires "tiers.rayTracing >= 2")
        RG8_SNORM,                          // +  +  +  -  +  -  +  +  +  - // "AccelerationStructure" compatible (requires "tiers.rayTracing >= 2")
        RG8_UINT,                           // +  +  +  -  -  -  +  +  +  -
        RG8_SINT,                           // +  +  +  -  -  -  +  +  +  -

        BGRA8_UNORM,                        // +  +  +  -  +  -  +  +  +  -
        BGRA8_SRGB,                         // +  -  +  -  +  -  -  -  -  -

        RGBA8_UNORM,                        // +  +  +  -  +  -  +  +  +  - // "AccelerationStructure" compatible (requires "tiers.rayTracing >= 2")
        RGBA8_SRGB,                         // +  -  +  -  +  -  -  -  -  -
        RGBA8_SNORM,                        // +  +  +  -  +  -  +  +  +  - // "AccelerationStructure" compatible (requires "tiers.rayTracing >= 2")
        RGBA8_UINT,                         // +  +  +  -  -  -  +  +  +  -
        RGBA8_SINT,                         // +  +  +  -  -  -  +  +  +  -

        // Plain: 16 bits per channel
        R16_UNORM,                          // +  +  +  -  +  -  +  +  +  -
        R16_SNORM,                          // +  +  +  -  +  -  +  +  +  -
        R16_UINT,                           // +  +  +  -  -  -  +  +  +  -
        R16_SINT,                           // +  +  +  -  -  -  +  +  +  -
        R16_SFLOAT,                         // +  +  +  -  +  -  +  +  +  -

        RG16_UNORM,                         // +  +  +  -  +  -  +  +  +  - // "AccelerationStructure" compatible (requires "tiers.rayTracing >= 2")
        RG16_SNORM,                         // +  +  +  -  +  -  +  +  +  - // "AccelerationStructure" compatible
        RG16_UINT,                          // +  +  +  -  -  -  +  +  +  -
        RG16_SINT,                          // +  +  +  -  -  -  +  +  +  -
        RG16_SFLOAT,                        // +  +  +  -  +  -  +  +  +  - // "AccelerationStructure" compatible

        RGBA16_UNORM,                       // +  +  +  -  +  -  +  +  +  - // "AccelerationStructure" compatible (requires "tiers.rayTracing >= 2")
        RGBA16_SNORM,                       // +  +  +  -  +  -  +  +  +  - // "AccelerationStructure" compatible
        RGBA16_UINT,                        // +  +  +  -  -  -  +  +  +  -
        RGBA16_SINT,                        // +  +  +  -  -  -  +  +  +  -
        RGBA16_SFLOAT,                      // +  +  +  -  +  -  +  +  +  - // "AccelerationStructure" compatible

        // Plain: 32 bits per channel
        R32_UINT,                           // +  +  +  -  -  +  +  +  +  +
        R32_SINT,                           // +  +  +  -  -  +  +  +  +  +
        R32_SFLOAT,                         // +  +  +  -  +  +  +  +  +  +

        RG32_UINT,                          // +  +  +  -  -  -  +  +  +  -
        RG32_SINT,                          // +  +  +  -  -  -  +  +  +  -
        RG32_SFLOAT,                        // +  +  +  -  +  -  +  +  +  - // "AccelerationStructure" compatible

        RGB32_UINT,                         // +  -  -  -  -  -  +  -  +  -
        RGB32_SINT,                         // +  -  -  -  -  -  +  -  +  -
        RGB32_SFLOAT,                       // +  -  -  -  -  -  +  -  +  - // "AccelerationStructure" compatible

        RGBA32_UINT,                        // +  +  +  -  -  -  +  +  +  -
        RGBA32_SINT,                        // +  +  +  -  -  -  +  +  +  -
        RGBA32_SFLOAT,                      // +  +  +  -  +  -  +  +  +  -

        // Packed: 16 bits per pixel
        B5_G6_R5_UNORM,                     // +  -  +  -  +  -  -  -  -  -
        B5_G5_R5_A1_UNORM,                  // +  -  +  -  +  -  -  -  -  -
        B4_G4_R4_A4_UNORM,                  // +  -  +  -  +  -  -  -  -  -

        // Packed: 32 bits per pixel
        R10_G10_B10_A2_UNORM,               // +  +  +  -  +  -  +  +  +  - // "AccelerationStructure" compatible (requires "tiers.rayTracing >= 2")
        R10_G10_B10_A2_UINT,                // +  +  +  -  -  -  +  +  +  -
        R11_G11_B10_UFLOAT,                 // +  +  +  -  +  -  +  +  +  -
        R9_G9_B9_E5_UFLOAT,                 // +  -  -  -  -  -  -  -  -  -

        // Block-compressed
        BC1_RGBA_UNORM,                     // +  -  -  -  -  -  -  -  -  -
        BC1_RGBA_SRGB,                      // +  -  -  -  -  -  -  -  -  -
        BC2_RGBA_UNORM,                     // +  -  -  -  -  -  -  -  -  -
        BC2_RGBA_SRGB,                      // +  -  -  -  -  -  -  -  -  -
        BC3_RGBA_UNORM,                     // +  -  -  -  -  -  -  -  -  -
        BC3_RGBA_SRGB,                      // +  -  -  -  -  -  -  -  -  -
        BC4_R_UNORM,                        // +  -  -  -  -  -  -  -  -  -
        BC4_R_SNORM,                        // +  -  -  -  -  -  -  -  -  -
        BC5_RG_UNORM,                       // +  -  -  -  -  -  -  -  -  -
        BC5_RG_SNORM,                       // +  -  -  -  -  -  -  -  -  -
        BC6H_RGB_UFLOAT,                    // +  -  -  -  -  -  -  -  -  -
        BC6H_RGB_SFLOAT,                    // +  -  -  -  -  -  -  -  -  -
        BC7_RGBA_UNORM,                     // +  -  -  -  -  -  -  -  -  -
        BC7_RGBA_SRGB,                      // +  -  -  -  -  -  -  -  -  -

        // Depth-stencil
        D16_UNORM,                          // -  -  -  +  -  -  -  -  -  -
        D24_UNORM_S8_UINT,                  // -  -  -  +  -  -  -  -  -  -
        D32_SFLOAT,                         // -  -  -  +  -  -  -  -  -  -
        D32_SFLOAT_S8_UINT_X24,             // -  -  -  +  -  -  -  -  -  -

        // Depth-stencil (SHADER_RESOURCE)
        R24_UNORM_X8,       // .x - depth   // +  -  -  -  -  -  -  -  -  -
        X24_G8_UINT,        // .y - stencil // +  -  -  -  -  -  -  -  -  -
        R32_SFLOAT_X8_X24,  // .x - depth   // +  -  -  -  -  -  -  -  -  -
        X32_G8_UINT_X24,    // .y - stencil // +  -  -  -  -  -  -  -  -  -
    
        MaxNum
    };

    //----------------------------------------------------------------------------------------------------
    // https://learn.microsoft.com/en-us/windows/win32/direct3d12/subresources#plane-slice
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

    enum class EFormatType : uint8
    {
        Integer,
        Normalized,
        Float,
        DepthStencil,
    };

    enum class EFormatSupportBits : uint16
    {
        Unsupported                 = 0,

        // Texture
        Texture                     = NES_BIT(0),
        StorageTexture              = NES_BIT(1),
        StorageTextureAtomics       = NES_BIT(2),
        ColorAttachment             = NES_BIT(3),
        DepthStencilAttachment      = NES_BIT(4),
        Blend                       = NES_BIT(5),
        Multisample2x               = NES_BIT(6),
        Multisample4x               = NES_BIT(7),
        Multisample8x               = NES_BIT(8),

        // Buffer
        Buffer                      = NES_BIT(9),
        StorageBuffer               = NES_BIT(10),
        StorageBufferAtomics        = NES_BIT(11),
        VertexBuffer                = NES_BIT(12),
        StorageLoadWithoutFormat    = NES_BIT(13),
    };
    NES_DEFINE_BIT_OPERATIONS_FOR_ENUM(EFormatSupportBits);

    //----------------------------------------------------------------------------------------------------
    /// @brief : Information about a format type. 
    //----------------------------------------------------------------------------------------------------
    struct FormatProps
    {
        const char* m_name;                 // Name of the format.
        EFormat     m_format;               // The format value.
        uint8       m_redBits;              // R (or depth) bits
        uint8       m_greenBits;            // G (or stencil) bits (0 if channels are < 2)
        uint8       m_blueBits;             // B bits (0 if channels < 3)
        uint8       m_alphaBits;            // A (or shared exponent) bits (0 if channels are < 4)
        uint32      m_stride        : 6;    // Block size in bytes
        uint32      m_blockWidth    : 4;    // 1 for plain formats, >1 for compressed
        uint32_t    m_blockHeight   : 4;    // 1 for plain formats, >1 for compressed
        uint32      m_isBgr         : 1;    // Reversed channels (RGBA => BGRA)
        uint32      m_isCompressed  : 1;    // Block-compressed format
        uint32      m_isDepth       : 1;    // Has depth component.
        uint32      m_isExpShared   : 1;    // Shared exponent in alpha channel
        uint32      m_isFloat       : 1;    // Floating point
        uint32      m_isPacked      : 1;    // 16- or 32- bit packed
        uint32      m_isInteger     : 1;    // integer
        uint32      m_isNorm        : 1;    // [0; 1] normalized
        uint32      m_isSigned      : 1;    // signed.
        uint32      m_isSrgb        : 1;    // sRGB
        uint32      m_isStencil     : 1;    // has stencil component
        uint32      m_unused        : 7;
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

        // Graphics, invoked by "CmdDraw*" commands.
        IndexInput              = NES_BIT(0),   /// Index buffer consumption
        VertexShader            = NES_BIT(1),   /// Vertex Shader
        TessControlShader       = NES_BIT(2),   /// Tesselation control (hull) shader
        TessEvaluationShader    = NES_BIT(3),   /// Tesselation evaluation (domain) shader
        GeometryShader          = NES_BIT(4),   /// Geometry Shader
        MeshControlShader       = NES_BIT(5),   /// Mesh control (task) shader
        MeshEvaluationShader    = NES_BIT(6),   /// Mesh evaluation (amplification) shader
        FragmentShader          = NES_BIT(7),   /// Fragment (pixel) shader
        DepthStencilAttachment  = NES_BIT(8),   /// Depth-stencil read/write operations
        ColorAttachment         = NES_BIT(9),   /// Color read/write operations

        // Compute, invoked by "CmdDispatch*" commands. (Not rays).
        ComputeShader           = NES_BIT(10),  /// Compute Shader

        // Ray Tracing, invoked by "CmdDispatchRays*" commands.
        RayGenShader            = NES_BIT(11),  /// Ray generation shader
        MissShader              = NES_BIT(12),  /// Miss shader
        IntersectionShader      = NES_BIT(13),  /// Intersection shader
        ClosestHitShader        = NES_BIT(14),  /// Closest hit shader.
        AnyHitShader            = NES_BIT(15),  /// Any hit shader
        CallableShader          = NES_BIT(16),  /// Callable shader

        AccelerationStructure   = NES_BIT(17),  /// Invoked by "Cmd*AccelerationStructure*" commands.
        MicroMap                = NES_BIT(18),  /// Invoked by "Cmd*Micromap*" commands

        // Other
        Copy                    = NES_BIT(19),  /// Invoked by "CmdCopy*", "CmdUpload*" and "CmdReadback*"
        Resolve                 = NES_BIT(20),  /// Invoked by "CmdResolveTexture"
        ClearStorage            = NES_BIT(21),  /// Invoked by "CmdClearStorage"

        // Modifiers
        Indirect                = NES_BIT(22), /// Invoked by "Indirect" commands (used in addition to other bits)

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
        None                        = 0,

        //                                          ACCESS         Compatible EStageBits (including All)
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
    // https://microsoft.github.io/DirectX-Specs/d3d/D3D12EnhancedBarriers.html#d3d12_barrier_layout
    //----------------------------------------------------------------------------------------------------
    enum class ELayout : uint8
    {
        // Special
        Undefined,
        General,                // ~All access, but not optimal.
        Present,                // None

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
    
    //----------------------------------------------------------------------------------------------------
    // BARRIERS:
    // Barriers consist of two phases:
    // - before (source scope, 1st synchronization scope):
    //   - "AccessBits" corresponding with any relevant resource usage since the preceding barrier or the start of "QueueSubmit" scope
    //   - "PipelineStageBits" of all preceding GPU work that must be completed before executing the barrier (stages to wait before the barrier)
    //   - "Layout" for textures
    // - after (destination scope, 2nd synchronization scope):
    //   - "AccessBits" corresponding with any relevant resource usage after the barrier completes.
    //   - "PipelineStageBits" of all subsequent GPU work that must wait until the barrier execution is finished (stages to halt until the barrier is executed).
    //   - "Layout" for textures
    //----------------------------------------------------------------------------------------------------

    //----------------------------------------------------------------------------------------------------
    /// @brief : Defines an access level that the resource should be in, as well as what
    ///     stage in the pipeline that it should have that access.
    //----------------------------------------------------------------------------------------------------
    struct AccessStage
    {
        EAccessBits         m_access;
        EPipelineStageBits  m_stages;
    };

    //----------------------------------------------------------------------------------------------------
    /// @brief : Defines an access level and image layout that the resource should be in, as well as what
    ///     stage in the pipeline that it should have these values.
    //----------------------------------------------------------------------------------------------------
    struct AccessLayoutStage
    {
        EAccessBits         m_access;
        ELayout             m_layout;
        EPipelineStageBits  m_stages;
    };

    //----------------------------------------------------------------------------------------------------
    /// @brief : Barrier information for a resource.
    // Barriers consist of two phases:
    // - before (source scope, 1st synchronization scope):
    //   - "AccessBits" corresponding with any relevant resource usage since the preceding barrier or the start of "QueueSubmit" scope
    //   - "PipelineStageBits" of all preceding GPU work that must be completed before executing the barrier (stages to wait before the barrier)
    // - after (destination scope, 2nd synchronization scope):
    //   - "AccessBits" corresponding with any relevant resource usage after the barrier completes.
    //   - "PipelineStageBits" of all subsequent GPU work that must wait until the barrier execution is finished (stages to halt until the barrier is executed).
    //----------------------------------------------------------------------------------------------------
    struct GlobalBarrierDesc
    {
        AccessStage         m_before;
        AccessStage         m_after;
    };

    //----------------------------------------------------------------------------------------------------
    /// @brief : Barrier description for a buffer.
    // Barriers consist of two phases:
    // - before (source scope, 1st synchronization scope):
    //   - "AccessBits" corresponding with any relevant resource usage since the preceding barrier or the start of "QueueSubmit" scope
    //   - "PipelineStageBits" of all preceding GPU work that must be completed before executing the barrier (stages to wait before the barrier)
    // - after (destination scope, 2nd synchronization scope):
    //   - "AccessBits" corresponding with any relevant resource usage after the barrier completes.
    //   - "PipelineStageBits" of all subsequent GPU work that must wait until the barrier execution is finished (stages to halt until the barrier is executed).
    //----------------------------------------------------------------------------------------------------
    struct BufferBarrierDesc
    {
        //GBuffer*            m_pBuffer;
        AccessStage         m_before;
        AccessStage         m_after;
    };

    //----------------------------------------------------------------------------------------------------
    /// @brief : Barrier description for a texture.
    // Barriers consist of two phases:
    // - before (source scope, 1st synchronization scope):
    //   - "AccessBits" corresponding with any relevant resource usage since the preceding barrier or the start of "QueueSubmit" scope
    //   - "PipelineStageBits" of all preceding GPU work that must be completed before executing the barrier (stages to wait before the barrier)
    //   - "Layout" is the image layout preceding the barrier.
    // - after (destination scope, 2nd synchronization scope):
    //   - "AccessBits" corresponding with any relevant resource usage after the barrier completes.
    //   - "PipelineStageBits" of all subsequent GPU work that must wait until the barrier execution is finished (stages to halt until the barrier is executed).
    //   - "Layout" is the layout after the barrier completes.
    //----------------------------------------------------------------------------------------------------
    struct TextureBarrierDesc
    {
        //GTexture*           m_pTexture;
        AccessLayoutStage   m_before;                                   // The layout the texture should be in.
        AccessLayoutStage   m_after;                                    // The layout the texture should transition to.

        // Subresource Range: what portions of the image should be affected? 
        uint32              m_baseMip       = 0;                        // Starting mip level.
        uint32              m_mipCount      = graphics::kUseRemaining;  // Number of mip levels from the baseMip.
        uint32              m_baseLayer     = 0;                        // Starting layer value.
        uint32              m_layerCount    = graphics::kUseRemaining;  // Number of layers from the baseLayer.
        EPlaneBits          m_planes        = EPlaneBits::Color;        // Which planes of the image data should be affected.

        // Queue ownership transfer is potentially needed only for "EXCLUSIVE" textures
        // https://registry.khronos.org/vulkan/specs/latest/html/vkspec.html#synchronization-queue-transfers
        DeviceQueue*        m_pSrcQueue     = nullptr;                      // The owning queue at the start.
        DeviceQueue*        m_pDstQueue     = nullptr;                      // The queue that should take ownership.
    };

    //----------------------------------------------------------------------------------------------------
    /// @brief : Grouping of global, buffer and texture barriers. 
    //----------------------------------------------------------------------------------------------------
    struct BarrierGroupDesc
    {
        const GlobalBarrierDesc*    m_pGlobals = nullptr;
        uint32                      m_globalsCount = 0;
        const BufferBarrierDesc*    m_pBuffers = nullptr;
        uint32                      m_bufferCount = 0;
        const TextureBarrierDesc*   m_pTextures = nullptr;
        uint32                      m_textureCount = 0;
    };

#pragma endregion

//============================================================================================================================================================================================
#pragma region [ Resources: creation ]
//============================================================================================================================================================================================

    enum class ETextureType : uint8
    {
        Texture1D,
        Texture2D,
        Texture3D,
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
    /// @brief : Bit flags for how a texture will be used. 
    //----------------------------------------------------------------------------------------------------
    enum class ETextureUsageBits : uint8
    {
                                                        // Min Compatible Access:               Usage:
        None                            = 0,
        ShaderResource                  = NES_BIT(0),   // ShaderResource                       Read-only shader resource (SRV)
        ShaderResourceStorage           = NES_BIT(1),   // ShaderResourceStorage                Read/write shader resource (UAV)
        ColorAttachment                 = NES_BIT(2),   // ColorAttachment                      Color attachment (render target)
        DepthStencilAttachment          = NES_BIT(3),   // DepthStencilAttachmentRead/Write     Depth-stencil attachment (depth-stencil target)
        ShadingRateAttachment           = NES_BIT(4),   // ShadingRateAttachment                Shading rate attachment (source)
    };
    NES_DEFINE_BIT_OPERATIONS_FOR_ENUM(ETextureUsageBits);

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
        ConstantBuffer                  = NES_BIT(4),   // ConstantBuffer                       Constant buffer.
        ArgumentBuffer                  = NES_BIT(5),   // ArgumentBuffer                       Argument buffer in "Indirect" commands
        ScratchBuffer                   = NES_BIT(6),   // ScratchBuffer                        Scratch buffer in "CmdBuild*" commands
        ShaderBindingTable              = NES_BIT(7),   // ShaderBindingTable                   Shader binding table (SBT) in "CmdDispatchRays*" commands
        AccelerationStructureBuildInput = NES_BIT(8),   // ShaderResource                       Read-only input in "CmdBuildAccelerationStructures" command      
        AccelerationStructureStorage    = NES_BIT(9),   // AccelerationStructureRead/Write      (INTERNAL) acceleration structure storage
        MicromapBuildInput              = NES_BIT(10),  // ShaderResource                       Read-only input in "CmdBuildMicromaps" command
        MicromapStorage                 = NES_BIT(11),  // MicromapRead/Write                   (INTERNAL) micromap storage
    };
    NES_DEFINE_BIT_OPERATIONS_FOR_ENUM(EBufferUsageBits);

    struct TextureDesc
    {
        ETextureType        m_type = ETextureType::Texture2D;
        ETextureUsageBits   m_usage = ETextureUsageBits::ShaderResource;
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
        // [TODO]: Move implementation to another file.
        /// @brief : Ensures that ranges are valid. 
        //----------------------------------------------------------------------------------------------------
        void                Validate()
        {
            m_height = math::Max(m_height, 1U);
            m_depth = math::Max(m_depth, 1U);
            m_mipCount = math::Max(m_mipCount, 1U);
            m_layerCount = math::Max(m_layerCount, 1U);
            m_sampleCount = math::Max(m_sampleCount, 1U);
        }
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
        uint64              m_size = 0;
        uint32              m_structuredStride = 0;
        EBufferUsageBits    m_usage = EBufferUsageBits::ShaderResource;
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
        // - Good for any resources that you frequently write and read on GPU, e.g. textures used as color attachments
        //   (aka "render targets"), depth-stencil attachments, images/buffers used as storage image/buffer
        //   (aka "Unordered Access View (UAV)"). 
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
        
        // System Memory. CPU reads and writes cached.
        // - Good for resources written by the GPU, read by the CPU - results of computations.
        // - The memory is directly accessed by both the CPU and GPU - you don't need to do an explicit transfer.
        // - Use for any resources read or randomly accessed on the CPU.
        HostReadback,
        
        MaxNum
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
    };
    static_assert(sizeof(DeviceMemoryType) == sizeof(DeviceMemoryTypeInfo));

    struct DeviceMemoryDesc
    {
        uint64              m_size;
        uint32              m_alignment;
        DeviceMemoryType    m_type;
        bool                m_mustBeDedicated; // Must be put into a dedicated memory object, containing only 1 object with offset = 0
    };

    struct AllocateBufferDesc
    {
        BufferDesc          m_desc;
        EMemoryLocation     m_location;
        float               m_priority;
        bool                m_isDedicated;
    };

    struct AllocateTextureDesc
    {
        TextureDesc         m_desc;
        EMemoryLocation     m_memoryLocation;
        float               m_priority = 0.f;
        bool                m_isDedicated = false;
    };

    struct BufferMemoryBindingDesc
    {
        DeviceBuffer*       m_pBuffer = nullptr;
        VmaAllocation       m_pMemory = nullptr;
        uint64              m_offset = 0;
    };

    struct TextureMemoryBindingDesc
    {
        Texture*            m_pTexture = nullptr;
        VmaAllocation       m_pMemory = nullptr;
        uint64              m_offset = 0;
    };

#pragma endregion

//============================================================================================================================================================================================
#pragma region [ Descriptor ]
//============================================================================================================================================================================================

    enum class ETexture1DViewType : uint8
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

    enum class ETexture2DViewType : uint8
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

    enum class ETexture3DViewType : uint8
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
    // https://learn.microsoft.com/en-us/windows/win32/api/d3d12/ne-d3d12-d3d12_filter
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
    // https://learn.microsoft.com/en-us/windows/win32/api/d3d12/ne-d3d12-d3d12_texture_address_mode
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
    /// @brief : Describes access to a 1D Texture resource. Used to create a Descriptor.
    //----------------------------------------------------------------------------------------------------
    struct Texture1DViewDesc
    {
        const DeviceImage*  m_pTexture;
        ETexture1DViewType  m_viewType;
        EFormat             m_format;
        uint32              m_mipOffset;
        uint32              m_mipCount = graphics::kUseRemaining;
        uint32              m_layerOffset;
        uint32              m_layerCount = graphics::kUseRemaining;
    };

    //----------------------------------------------------------------------------------------------------
    /// @brief : Describes access to a 2D Texture resource. Used to create a Descriptor.
    //----------------------------------------------------------------------------------------------------
    struct Texture2DViewDesc
    {
        const DeviceImage*  m_pTexture;
        ETexture2DViewType  m_viewType;
        EFormat             m_format;
        uint32              m_mipOffset;
        uint32              m_mipCount = graphics::kUseRemaining;
        uint32              m_layerOffset;
        uint32              m_layerCount = graphics::kUseRemaining;
    };

    //----------------------------------------------------------------------------------------------------
    /// @brief : Describes access to a 3D Texture resource. Used to create a Descriptor.
    //----------------------------------------------------------------------------------------------------
    struct Texture3DViewDesc
    {
        const DeviceImage*  m_pTexture;
        ETexture3DViewType  m_viewType;
        EFormat             m_format;
        uint32              m_mipOffset;
        uint32              m_mipCount = graphics::kUseRemaining;
        uint32              m_sliceOffset;
        uint32              m_sliceCount = graphics::kUseRemaining;
    };

    //----------------------------------------------------------------------------------------------------
    /// @brief : Describes access to a DeviceBuffer resource. Used to create a Descriptor.
    //----------------------------------------------------------------------------------------------------
    struct BufferViewDesc
    {
        const DeviceBuffer* m_pBuffer;
        EBufferViewType     m_viewType;
        EFormat             m_format;
        uint64              m_offset = 0;
        uint64              m_size = graphics::kUseRemaining;

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

    /*
    All indices are local in the currently bound pipeline layout.
    
    Pipeline layout example:
        Descriptor set                  #0          // "setIndex" - a descriptor set index in the pipeline layout, provided as an argument or bound to the pipeline
            Descriptor range                #0      // "rangeIndex" and "baseRange" - a descriptor range (base) index in the descriptor set
                Descriptor                      #0  // "descriptorIndex" and "baseDescriptor" - a descriptor (base) index in the descriptor range
                Descriptor                      #1
                Descriptor                      #2
            Descriptor range                #1
                Descriptor                      #0
                Descriptor                      #1
            Dynamic constant buffer         #0      // "baseDynamicConstantBuffer" - an offset in "dynamicConstantBuffers" in the currently bound pipeline layout for the provided descriptor set
            Dynamic constant buffer         #1
    
        Descriptor set                  #1
            Descriptor range                #0
                Descriptor                      #0
    
        Descriptor set                  #2
            Descriptor range                #0
                Descriptor                      #0
                Descriptor                      #1
                Descriptor                      #2
            Descriptor range                #1
                Descriptor                      #0
                Descriptor                      #1
            Descriptor range                #2
                Descriptor                      #0
            Dynamic constant buffer         #0
    
        RootConstantDesc                #0          // "rootConstantIndex" - an index in "rootConstants" in the currently bound pipeline layout
    
        RootDescriptorDesc              #0          // "rootDescriptorIndex" - an index in "rootDescriptors" in the currently bound pipeline layout
        RootDescriptorDesc              #1
    */

    enum class EPipelineLayoutBits : uint8
    {
        None                                = 0,
        IgnoreGlobalSPIRVOffsets            = NES_BIT(0), // VK: ignore "DeviceCreationDesc::vkBindingOffsets"
        EnableD3D12DrawParametersEmulation  = NES_BIT(1)  // D3D12: enable draw parameters emulation, not needed if all vertex shaders for this layout compiled with SM 6.8 (native support)
    };
    NES_DEFINE_BIT_OPERATIONS_FOR_ENUM(EPipelineLayoutBits);

    enum class EDescriptorPoolBits : uint8
    {
        None                                = 0,
        AllowUpdateAfterSet                 = NES_BIT(0)  // Allows "DescriptorSetBits::AllowUpdateAfterSet"
    };
    NES_DEFINE_BIT_OPERATIONS_FOR_ENUM(EDescriptorPoolBits);

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
    enum class EDescriptorType : uint8
    {
        None,
        Sampler,            
        ConstantBuffer,     // VK: Uniform Buffer
        Texture,            // VK: Sampled Image
        StorageTexture,     // VK: Storage Image
        Buffer,             // VK: Uniform Texel Buffer
        StorageBuffer,      // VK: Storage Texel Buffer
        AccelerationStructure,
    };

    constexpr bool DescriptorIsBufferType(const EDescriptorType type)
    {
        switch (type)
        {
            case EDescriptorType::Buffer:
            case EDescriptorType::ConstantBuffer:
            case EDescriptorType::StorageBuffer:
                return true;

            default: return false;
        }
    }

    constexpr bool DescriptorIsTextureType(const EDescriptorType type)
    {
        switch (type)
        {
            case EDescriptorType::Texture:
            case EDescriptorType::StorageTexture:
                return true;

            default: return false;
        }
    }

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
        uint32                      m_rootRegisterSpace;
        const PushConstantDesc*     m_pRootConstants;
        uint32                      m_rootConstantNum;
        const PushDescriptorDesc*   m_pRootDescriptors;
        uint32                      m_rootDescriptorNum;
        const DescriptorSetDesc*    m_pDescriptorSets;
        uint32                      m_descriptorSetNum;
        EPipelineStageBits          m_shaderStages;
        EPipelineLayoutBits         m_flags;
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
        //const Descriptor* const*   m_pDescriptors;
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

    //----------------------------------------------------------------------------------------------------
    /// @brief : Allowable types for an index buffer. 
    //----------------------------------------------------------------------------------------------------
    enum class EIndexType : uint8
    {
        U16,
        U32,
    };

    enum class EPrimitiveRestart : uint8
    {
        Disabled,   
        IndicesU16, // Index "0xFFFF" enforces primitive restart.
        IndicesU32  // Index "0xFFFFFFFF" enforces primitive restart.
    };

    //----------------------------------------------------------------------------------------------------
    // https://registry.khronos.org/vulkan/specs/latest/man/html/VkVertexInputRate.html
    //----------------------------------------------------------------------------------------------------
    enum class EVertexStreamStepRate : uint8
    {
        PerVertex,
        PerInstance,
    };

    //----------------------------------------------------------------------------------------------------
    // https://registry.khronos.org/vulkan/specs/latest/man/html/VkPrimitiveTopology.html
    // https://learn.microsoft.com/en-us/windows/win32/api/d3dcommon/ne-d3dcommon-d3d_primitive_topology
    // https://learn.microsoft.com/en-us/windows/win32/api/d3d12/ne-d3d12-d3d12_primitive_topology_type
    //----------------------------------------------------------------------------------------------------
    enum class ETopology : uint8
    {
        PointList,
        LineList,
        LineStrip,
        TriangleList,
        TriangleStrip,
        LineListWithAdjacency,
        LineStripWithAdjacency,
        TriangleListWithAdjacency,
        TriangleStripWithAdjacency,
    };

    struct InputAssemblyDesc
    {
        ETopology               m_topology;
        uint8                   m_tessControlPointNum;
        EPrimitiveRestart       m_primitiveRestart;
    };

    struct VertexAttributeDesc
    {
        uint32                  m_location;
        uint32                  m_offset;
        EFormat                 m_format;
        uint16                  m_streamIndex;
    };

    struct VertexStreamDesc
    {
        uint16                  m_bindingSlot;
        EVertexStreamStepRate   m_stepRate;
    };

    struct VertexInputDesc
    {
        const VertexAttributeDesc* m_pAttributes;
        uint8                   m_attributesNum;
        const VertexStreamDesc* m_pStreams;
        uint8                   m_streamNum;
    };

    struct VertexBufferDesc
    {
        const DeviceBuffer*     m_pBuffer;
        uint64                  m_offset;
        uint32                  m_stride;
    };

#pragma endregion

//============================================================================================================================================================================================
#pragma region [ Graphics Pipeline: Rasterization ]
//============================================================================================================================================================================================

    //----------------------------------------------------------------------------------------------------		
    // https://registry.khronos.org/vulkan/specs/latest/man/html/VkPolygonMode.html 
    //----------------------------------------------------------------------------------------------------
    enum class EFillMode : uint8
    {
        Solid,
        Wireframe,
    };

    //----------------------------------------------------------------------------------------------------
    // https://registry.khronos.org/vulkan/specs/latest/man/html/VkCullModeFlagBits.html
    //----------------------------------------------------------------------------------------------------
    enum class ECullMode : uint8
    {
        None,
        Front,
        Back
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
        float           m_constant;
        float           m_clamp;
        float           m_slope;

        bool            IsEnabled() const { return m_constant != 0.f || m_slope != 0.f; }
    };

    struct RasterizationDesc
    {
        DepthBiasDesc   m_depthBias;
        EFillMode       m_fillMode;
        ECullMode       m_cullMode;
        bool            m_frontIsCounterClockwise;
        bool            m_enableDepthClamp;
        bool            m_enableLineSmoothing;          // Requires "features.m_lineSmoothing"
        bool            m_enableConservativeRaster;     // Requires "tiers.m_conservativeRaster != 0"
        bool            m_enableShadingRate;            // Requires "tiers.m_shadingRate != 0", expects "CmdSetShadingRate" and optionally "AttachmentsDesc::m_shadingRate"
    };

    struct MultisampleDesc
    {
        uint32          m_sampleMask;
        uint32          m_sampleCount = 0;
        bool            m_alphaToCoverage = false;
        bool            m_sampleLocations = false;              // Requires "tiers.m_sampleLocations != 0", expects "CmdSetSampleLocations"
    };

    struct ShadingRateDesc
    {
        EShadingRate    m_shadingRate;
        EShadingRateCombiner m_primitiveCombiner;       // Requires "tiers.m_sampleLocations >= 2"
        EShadingRateCombiner m_attachmentCombiner;      // Requires "tiers.m_sampleLocations >= 2
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
    enum class EColorWriteBits : uint8
    {
        None    = 0,
        R       = NES_BIT(0),
        G       = NES_BIT(1),
        B       = NES_BIT(2),
        A       = NES_BIT(3),

        RGB     = R | G | B,
        RGBA    = RGB | A,
    };
    NES_DEFINE_BIT_OPERATIONS_FOR_ENUM(EColorWriteBits);

    //----------------------------------------------------------------------------------------------------
    // https://registry.khronos.org/vulkan/specs/latest/man/html/VkStencilOpState.html
    // https://learn.microsoft.com/en-us/windows/win32/api/d3d12/ns-d3d12-d3d12_depth_stencil_desc 
    //----------------------------------------------------------------------------------------------------
    struct StencilDesc
    {
        ECompareOp                  m_compareOp;        /// Comparison operation
        EStencilOp                  m_failOp;           /// Value to use on fail.
        EStencilOp                  m_passOp;           /// Value to use on pass.
        EStencilOp                  m_depthFailOp;      /// Value to use on depth fail.
        uint8                       m_writeMask;        /// Which color channels to write to.
        uint8                       m_compareMask;      /// Which color channels to compare.
    };

    struct DepthAttachmentDesc
    {
        ECompareOp                  m_compareOp;
        bool                        m_enableWrite;
        bool                        m_enableBoundsTest; /// Requires "features.m_depthBoundsTest", expects "CmdSetDepthBounds".   
    };

    struct StencilAttachmentDesc
    {
        StencilDesc                 m_front;
        StencilDesc                 m_back;             /// Requires "features.m_independentFrontAndBackStencilReferenceAndMasks" for "back.m_writeMask"
    };

    //----------------------------------------------------------------------------------------------------
    // https://registry.khronos.org/vulkan/specs/latest/man/html/VkPipelineColorBlendAttachmentState.html
    // https://learn.microsoft.com/en-us/windows/win32/api/d3d12/ns-d3d12-d3d12_render_target_blend_desc
    //----------------------------------------------------------------------------------------------------
    struct BlendDesc
    {
        EBlendFactor                m_srcFactor;
        EBlendFactor                m_dstFactor;
        EBlendOp                    m_op;
    };

    struct ColorAttachmentDesc
    {
        EFormat                     m_format;
        BlendDesc                   m_colorBlend;
        BlendDesc                   m_alphaBlend;
        EColorWriteBits             m_colorWriteMask;
        bool                        m_enableBlend;
    };

    struct OutputMergerDesc
    {
        const ColorAttachmentDesc*  m_pColors;
        uint32                      m_colorNum;
        DepthAttachmentDesc         m_depth;
        StencilAttachmentDesc       m_stencil;
        EFormat                     m_depthStencilFormat;
        ELogicOp                    m_logicOp;              /// Requires "features.m_logicOp".
    
        // Optional
        uint32                      m_viewMask;             /// If non-0, requires "viewMaximum > 0".
        EMultiview                  m_multiView;            /// if "m_viewMask != 0", requires "features.(xxx)Muliview".
    };

    struct AttachmentDesc
    {
        const Descriptor* const*    m_pColors = nullptr;
        uint32                      m_colorCount = 0;
    
        // Optional
        const Descriptor*           m_pDepthStencil = nullptr;
        const Descriptor*           m_pShadingRate = nullptr;
        uint32                      m_viewMask = 0;
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

    struct GraphicsPipelineDesc
    {
        const PipelineLayout*   m_pPipelineLayout;
        const VertexInputDesc*  m_pVertexInput;     // Optional.
        InputAssemblyDesc       m_inputAssembly;
        RasterizationDesc       m_rasterizer;
        const MultisampleDesc*  m_pMultisample;     // Optional.
        OutputMergerDesc        m_outputMerger;
        const ShaderDesc*       m_pShaders;
        uint32                  m_shaderNum;
        ERobustness             m_robustness;       // Optional.
    };

    struct ComputePipelineDesc
    {
        const PipelineLayout*   m_pPipelineLayout;
        ShaderDesc              m_shader;
        ERobustness             m_robustness;       // Optional.
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
        uint64      m_inputVertexNum;
        uint64      m_inputPrimitiveNum;
        uint64      m_vertexShaderInvocationNum;
        uint64      m_geometryShaderInvocationNum;
        uint64      m_geometryShaderPrimitiveNum;
        uint64      m_rasterizerInPrimitiveNum;
        uint64      m_rasterizerOutPrimitiveNum;
        uint64      m_fragmentShaderInvocationNum;
        uint64      m_tessControlShaderInvocationNum;
        uint64      m_tessEvaluationShaderInvocationNum;
        uint64      m_computeShaderInvocationNum;

        // If "features.m_meshShaderPipelineStats"
        uint64      m_meshControlShaderInvocationNum;
        uint64      m_meshEvaluationShaderInvocationNum;
        uint64      m_meshEvaluationShaderPrimitiveNum;
    };

#pragma endregion

//============================================================================================================================================================================================
#pragma region [ Command Signatures ]
//============================================================================================================================================================================================

    struct DrawDesc
    {
        uint32  m_vertexNum = 0;
        uint32  m_instanceNum = 0;
        uint32  m_baseVertex = 0;                   // Vertex buffer offset = CmdSetVertexBuffers.m_offset + m_baseVertex * VertexStreamDesc::m_stride
        uint32  m_baseInstance = 0;
    };

    struct DrawIndexedDesc
    {
        uint32  m_indexNum = 0;
        uint32  m_instanceNum = 0;
        uint32  m_baseIndex = 0;                    // Index buffer offset = CmdSetIndexBuffer.m_offset + m_baseIndex * sizeof(CmdSetIndexBuffer.m_indexType)
        int32   m_baseVertex = 0;                   // index += baseVertex;
        uint32  m_baseInstance = 0;
    };

    struct DispatchDesc
    {
        uint32  x = 0;
        uint32  y = 0;
        uint32  z = 0;
    };

#pragma endregion

//============================================================================================================================================================================================
#pragma region [ Other ]
//============================================================================================================================================================================================

    //----------------------------------------------------------------------------------------------------
    /// @brief : Describes a region of a texture for copy commands. 
    //----------------------------------------------------------------------------------------------------
    struct TextureRegionDesc
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

    struct TextureDataLayoutDesc
    {
        uint64      m_offset;       // A buffer offset must be a multiple of "m_uploadBufferTextureSliceAlignment" (data placement alignment)
        uint32      m_rowPitch;     // Must be a multiple of "m_uploadBufferTextureRowAlignment"
        uint32      m_slicePitch;   // Must be a multiple of "m_uploadBufferTextureSliceAlignment"
    };

    struct FenceSubmitDesc
    {
        // [TODO]: 
        //GFence*           m_pFence;
        uint64              m_value;
        EPipelineStageBits  m_stages;
    };

    struct QueueSubmitDesc
    {
        const FenceSubmitDesc*  m_pWaitFences;
        uint32                  m_waitFenceNum;
        const CommandBuffer*    m_pCommandBuffers;
        uint32                  m_commandBufferNum;
        const FenceSubmitDesc*  m_pSignalFences;
        uint32                  m_signalFenceNum;
    };

    struct ClearDesc
    {
        ClearValue              m_clearValue;
        EPlaneBits              m_planes;
        uint32                  m_colorAttachmentIndex;
    };

    //----------------------------------------------------------------------------------------------------
    // For any buffers and textures with integer formats:
    //  - Clears a storage view with bit-precise values, copying the lower "N" bits from "value.[f/ui/i].channel"
    //    to the corresponding channel, where "N" is the number of bits in the "channel" of the resource format
    // For textures with non-integer formats:
    //  - Clears a storage view with float values with format conversion from "FLOAT" to "UNORM/SNORM" where appropriate
    // For buffers:
    //  - To avoid discrepancies in behavior between GAPIs use "R32f/ui/i" formats for views
    //  - D3D: structured buffers are unsupported! 
    //----------------------------------------------------------------------------------------------------
    struct ClearStorageDesc
    {
        const Descriptor*       m_pStorage; // A "Storage" descriptor.
        Color                   m_value;
        uint32                  m_setIndex;
        uint32                  m_rangeIndex;
        uint32                  m_descriptorIndex;
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
        const char* m_extensionName = nullptr;      /// Name of the extension. Ex: VK_KHR_SWAPCHAIN_EXTENSION_NAME.
        void*       m_pFeature = nullptr;           /// [optional] Pointer to the feature structure for the extension.
        bool        m_isRequired = true;            /// [optional] If the extension is required.
        uint32      m_version = 0;                  /// [optional] Spec version of the extension, this version or higher.
        bool        m_requireExactVersion = false;  /// [optional] If true, the spec version must match exactly.
    };

    //----------------------------------------------------------------------------------------------------
    /// @brief : Helper function to return the EQueueType as an index value. (Casts to size_t).
    //----------------------------------------------------------------------------------------------------
    constexpr size_t GetQueueTypeIndex(const EQueueType type) { return static_cast<size_t>(type); }

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
            uint32                  m_bufferTextureGranularity      = 0;
            uint64                  m_bufferMaxSize                 = 0;
        } m_memory;

        // Memory Alignment Properties
        struct
        {
            uint32                  m_uploadBufferTextureRow        = 0;
            uint32                  m_uploadBufferTextureSlice      = 0;
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