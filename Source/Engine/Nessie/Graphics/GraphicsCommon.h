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

namespace nes
{
    static constexpr uint32 kMaxRenderTargets = 8;
    static constexpr uint32 kMaxViewports = 16;
    static constexpr uint32 kMaxVertexAttributes = 16;
    static constexpr uint32 kMaxBindingLayouts = 8;
    static constexpr uint32 kMaxBindlessRegisterSpaces = 16;
    static constexpr uint32 kMaxVolatileConstantBuffersPerLayout = 6;
    static constexpr uint32 kMaxVolatileConstantBuffers = 32;
    static constexpr uint32 kMaxPushConstantSize = 128; // D3D12: root signature is 256 bytes max., Vulkan: 128 bytes of push constants guaranteed
    static constexpr uint32 kConstantBufferOffsetSizeAlignment = 256;

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
        float           m_depth;
        uint8           m_stencil;
    };

    union ClearValue
    {
        DepthStencil    m_depthStencil;
        Color           m_color;
    };

    struct SampleLocation
    {
        int8 x, y = 0;
    };

    //----------------------------------------------------------------------------------------------------
    /// @brief : "2 Dimensions": width & height
    //----------------------------------------------------------------------------------------------------
    struct Dim2
    {
        DimType         m_width;
        DimType         m_height;
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
        // Compatible "EAccessBits".
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

    struct AccessStage
    {
        EAccessBits         m_access;
        EPipelineStageBits  m_stages;
    };

    struct AccessLayoutStage
    {
        EAccessBits         m_access;
        ELayout             m_layout;
        EPipelineStageBits  m_stages;
    };

    struct GlobalBarrierDesc
    {
        AccessStage         m_before;
        AccessStage         m_after;
    };

    struct GBufferBarrierDesc
    {
        GBuffer*            m_pBuffer;
        AccessStage         m_before;
        AccessStage         m_after;
    };

    struct GTextureBarrierDesc
    {
        GTexture*           m_pTexture;
        AccessLayoutStage   m_before;
        AccessLayoutStage   m_after;
        DimType             m_baseMip;
        DimType             m_NumMips;
        DimType             m_baseLayer;
        DimType             m_numLayers;
        EPlaneBits          m_planes;

        // Queue ownership transfer is potentially needed only for "EXCLUSIVE" textures
        // https://registry.khronos.org/vulkan/specs/latest/html/vkspec.html#synchronization-queue-transfers
        DeviceQueue*             m_pSrcQueue;
        DeviceQueue*             m_pDstQueue;
    };

    //----------------------------------------------------------------------------------------------------
    /// @brief : Description of a group of global, buffer and texture barriers. 
    //----------------------------------------------------------------------------------------------------
    struct GBarrierGroupDesc
    {
        const GlobalBarrierDesc*    m_pGlobals;
        uint32                      m_numGlobals;
        const GBufferBarrierDesc*    m_pBuffers;
        uint32                      m_numBuffers;
        const GTextureBarrierDesc*   m_pTextures;
        uint32                      m_numTextures;
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

    enum class EBufferUsageBits : uint16
    {
        // Min Compatible Access:               Usage:
        None                            = 0,
        ShaderResource                  = NES_BIT(0),   // ShaderResource                       Read-only shader resource (SRV)
        ShaderResourceStorage           = NES_BIT(1),   // ShaderResourceStorage                Read/write shader resource (UAV)
        VertexBuffer                    = NES_BIT(2),   // VertexBuffer                         Vertex Buffer
        IndexBuffer                     = NES_BIT(3),   // IndexBuffer                          Index Buffer
        ConstantBuffer                  = NES_BIT(4),   // ConstantBuffer                       Constant buffer (D3D11: can't be combined with other usages)
        ArgumentBuffer                  = NES_BIT(5),   // ArgumentBuffer                       Argument buffer in "Indirect" commands
        ScratchBuffer                   = NES_BIT(6),   // ScratchBuffer                        Scratch buffer in "CmdBuild*" commands
        ShaderBindingTable              = NES_BIT(7),   // ShaderBindingTable                   Shader binding table (SBT) in "CmdDispatchRays*" commands
        AccelerationStructureBuildInput = NES_BIT(8),   // ShaderResource                       Read-only input in "CmdBuildAccelerationStructures" command      
        AccelerationStructureStorage    = NES_BIT(9),   // AccelerationStructureRead/Write      (INTERNAL) acceleration structure storage
        MicromapBuildInput              = NES_BIT(10),  // ShaderResource                       Read-only input in "CmdBuildMicromaps" command
        MicromapStorage                 = NES_BIT(11),  // MicromapRead/Write                   (INTERNAL) micromap storage
    };
    NES_DEFINE_BIT_OPERATIONS_FOR_ENUM(EBufferUsageBits);

    struct GTextureDesc
    {
        ETextureType        m_type = ETextureType::Texture2D;
        ETextureUsageBits   m_usage = ETextureUsageBits::ShaderResource;
        EFormat             m_format = EFormat::Unknown;
        DimType             m_width = 1;
        DimType             m_height = 1;
        DimType             m_depth = 1;
        DimType             m_numMipLevels = 1;
        DimType             m_numLayers = 1;
        SampleType          m_sampleCount = 1;
        ESharingMode        m_sharingMode = ESharingMode::Exclusive;
        ClearValue          m_clearValue{};  // D3D12: not needed on desktop, since any HW can track many clear values.
    };

    //----------------------------------------------------------------------------------------------------
    /// @brief : Used to create a buffer resource.
    /// Available "m_structuredStride" Values:
    //      0   = allows "typed" views.
    //      4   = allows "types", "byte address" (raw) and "structured" views.
    //      >4  = allows "structured" and potentially "typed" views.
    //  VK: Buffers are always created with "VK_SHARING_MODE_CONCURRENT" to match D3D12 spec.
    //----------------------------------------------------------------------------------------------------
    struct GBufferDesc
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
    /// @brief : Where the memory will be allocated.
    //  https://learn.microsoft.com/en-us/windows/win32/api/d3d12/ne-d3d12-d3d12_heap_type
    //----------------------------------------------------------------------------------------------------
    enum class EMemoryLocation : uint8
    {
        Device,
        DeviceUpload,
        HostUpload,
        HostReadback,
        MaxNum
    };

    //----------------------------------------------------------------------------------------------------
    /// @brief : Returns true if the Host (CPU) can read the Memory at a given location.
    //----------------------------------------------------------------------------------------------------
    constexpr bool IsHostVisibleMemory(const EMemoryLocation location)
    {
        return location > EMemoryLocation::Device;
    }

    //----------------------------------------------------------------------------------------------------
    /// @brief : Returns true if the Host (CPU) owns the memory at a given location. 
    //----------------------------------------------------------------------------------------------------
    constexpr bool IsHostMemory(const EMemoryLocation location)
    {
        return location > EMemoryLocation::DeviceUpload;
    }

    struct GHeapDesc
    {
        uint64      m_size;
        uint32      m_alignment;
        GMemoryType m_type;
        bool        m_mustBeDedicated; // Must be put into a dedicated memory object, containing only 1 object with offset = 0
    };

    struct AllocateGMemoryDesc
    {
        uint64      m_size;
        GMemoryType m_type;
        float       m_priority; // [-1; 1]: low < 0, normal = 0, high > 0
    };

    struct GBufferMemoryBindingDesc
    {
        GBuffer*    m_pBuffer;
        GMemory*    m_pMemory;
        uint64      m_offset;
    };

    struct GTextureMemoryBindingDesc
    {
        GTexture*   m_pTexture;
        GMemory*    m_pMemory;
        uint64      m_offset;
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
    };

    //----------------------------------------------------------------------------------------------------
    // https://registry.khronos.org/vulkan/specs/latest/man/html/VkSamplerReductionMode.html
    //----------------------------------------------------------------------------------------------------
    enum class EReductionMode : uint8
    {
        Average,    // A weighted average of values in the footprint
        Min,        // A component-wise minimum of values in the footprint with non-zero weights
        Max,        // A component-wise maximum of values in the footprint with non-zero weights
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
    };

    struct GTexture1DViewDesc
    {
        const GTexture*     m_pTexture;
        ETexture1DViewType  m_viewType;
        EFormat             m_format;
        DimType             m_mipOffset;
        DimType             m_mipNum;
        DimType             m_layerOffset;
        DimType             m_layerNum;
    };

    struct GTexture2DViewDesc
    {
        const GTexture*     m_pTexture;
        ETexture2DViewType  m_viewType;
        EFormat             m_format;
        DimType             m_mipOffset;
        DimType             m_mipNum;
        DimType             m_layerOffset;
        DimType             m_layerNum;
    };

    struct GTexture3DViewDesc
    {
        const GTexture*     m_pTexture;
        ETexture3DViewType  m_viewType;
        EFormat             m_format;
        DimType             m_mipOffset;
        DimType             m_mipNum;
        DimType             m_layerOffset;
        DimType             m_layerNum;
    };

    struct GBufferViewDesc
    {
        const GBuffer*      m_pBuffer;
        EBufferViewType     m_viewType;
        EFormat             m_format;
        uint64              m_offset;
        uint64              m_size;

        // Optional
        uint32              m_structuredStride; // This will equal the structure stride from "GBufferDesc" if not provided.
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
        EFilterType m_max;
        EFilterType m_mip;
        EReductionMode m_ext; // Requires features.m_textureFilterMinMax.
    };

    //----------------------------------------------------------------------------------------------------
    // https://registry.khronos.org/vulkan/specs/latest/man/html/VkSamplerCreateInfo.html
    // https://learn.microsoft.com/en-us/windows/win32/api/d3d12/ns-d3d12-d3d12_sampler_desc
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
        Color               m_borderColor;
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
        Sampler,
        ConstantBuffer,
        Texture,
        StorageTexture,
        Buffer,
        StorageBuffer,
        StructuredBuffer,
        StorageStructuredBuffer,
        AccelerationStructure,
    };

    //----------------------------------------------------------------------------------------------------
    /// @brief : A Descriptor Range consists of "Descriptor" entities. 
    //----------------------------------------------------------------------------------------------------
    struct DescriptorRangeDesc
    {
        uint32                      m_baseRegisterIndex;
        uint32                      m_descriptorNum;        /// Treated as max size if "VARIABLE_SIZED_ARRAY" flag is set
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
    struct RootConstantDesc
    {
        uint32                      m_registerIndex;
        uint32                      m_size;
        EPipelineStageBits          m_shaderStages;
    };

    //----------------------------------------------------------------------------------------------------
    /// @brief : "PipelineLayout" consists of "DescriptorSet" descriptions and root parameters.
    ///     This is also known as a "push descriptor".
    //----------------------------------------------------------------------------------------------------
    struct RootDescriptorDesc
    {
        uint32                      m_registerIndex;
        EDescriptorType             m_descriptorType; // Can be ConstantBuffer, StructuredBuffer or StorageStructuredBuffer.
        EPipelineStageBits          m_shaderStages;
    };

    //----------------------------------------------------------------------------------------------------
    // https://registry.khronos.org/vulkan/specs/latest/man/html/VkPipelineLayoutCreateInfo.html
    // https://microsoft.github.io/DirectX-Specs/d3d/ResourceBinding.html#root-signature
    // https://microsoft.github.io/DirectX-Specs/d3d/ResourceBinding.html#root-signature-version-11
    //----------------------------------------------------------------------------------------------------
    struct PipelineLayoutDesc
    {
        uint32                      m_rootRegisterSpace;
        const RootConstantDesc*     m_pRootConstants;
        uint32                      m_rootConstantNum;
        const RootDescriptorDesc*   m_pRootDescriptors;
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
        const Descriptor* const*   m_pDescriptors;
        uint32                      m_descriptorNum;
        uint32                      m_baseDescriptor;
    };

    struct DescriptorSetCopyDesc
    {
        const DescriptorSet*        m_pSrcDescriptorSet;
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

    struct VertexAttributeD3D
    {
        const char*             m_semanticName;
        uint32                  m_semanticIndex;
    };

    struct VertexAttributeVk
    {
        uint32                  m_location;
    };

    struct VertexAttributeDesc
    {
        VertexAttributeD3D      m_d3d;
        VertexAttributeVk       m_vk;
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
        const GBuffer*          m_pBuffer;
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
        SampleType      m_sampleNum;
        bool            m_alphaToCoverage;
        bool            m_sampleLocations;              // Requires "tiers.m_sampleLocations != 0", expects "CmdSetSampleLocations"
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
        OneMinusSrc1Alpha       // 1 - S1.a                          1 - S1.a
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
        const Descriptor* const*    m_pColors;
        uint32                      m_colorNum;
    
        // Optional
        const Descriptor*           m_pDepthStencil;
        const Descriptor*           m_pShadingRate;         /// Requires "tiers.m_shadingRate >= 2".
        uint32                      m_viewMask;             /// If non-0, requires "m_viewMaxNum > 1".
    };

#pragma endregion

//============================================================================================================================================================================================
#pragma region [ Pipelines ]
//============================================================================================================================================================================================

    enum class ERobustness : uint8
    {
        Default,    // Don't care, follow device settings (VK level when used on a device).
        Off,        // No overhead, no robust access (out-of-bounds access is not allowed).
        VK,         // Minimal overhead, partial robust access.
        D3D12,      // Moderate overhead, D3D12-level robust access (requires "VK_EXT_robustness2", soft fallback to VK mode).
    };

    struct ShaderDesc
    {
        EPipelineStageBits      m_stage;
        const void*             m_pByteCode;
        uint64                  m_size;
        const char*             m_entryPointName = "main";
    };

    struct GraphicsPipelineDesc
    {
        const PipelineLayout*  m_pPipelineLayout;
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
        const PipelineLayout*  m_pPipelineLayout;
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
    
        // D3D12: if "features.m_meshShaderPipelineStats"
        uint64      m_meshEvaluationShaderPrimitiveNum;
    };

#pragma endregion

//============================================================================================================================================================================================
#pragma region [ Command Signatures ]
//============================================================================================================================================================================================

    struct DrawDesc
    {
        uint32  m_vertexNum;
        uint32  m_instanceNum;
        uint32  m_baseVertex;                   // Vertex buffer offset = CmdSetVertexBuffers.m_offset + m_baseVertex * VertexStreamDesc::m_stride
        uint32  m_baseInstance;
    };

    struct DrawIndexedDesc
    {
        uint32  m_indexNum;
        uint32  m_instanceNum;
        uint32  m_baseIndex;                    // Index buffer offset = CmdSetIndexBuffer.m_offset + m_baseIndex * sizeof(CmdSetIndexBuffer.m_indexType)
        int32   m_baseVertex;                   // index += baseVertex;
        uint32  m_baseInstance;
    };

    struct DispatchDesc
    {
        uint32  x;
        uint32  y;
        uint32  z;
    };

    // D3D12: modified draw command signatures, if the bound pipeline layout has "PipelineLayoutBits::ENABLE_D3D12_DRAW_PARAMETERS_EMULATION"
    //  - the following structs must be used instead

    struct DrawBaseDesc
    {
        uint32  m_shaderEmulatedBaseVertex;     // Root constant.
        uint32  m_shaderEmulatedBaseInstance;   // Root constant.
        uint32  m_vertexNum;
        uint32  m_instanceNum;
        uint32  m_baseVertex;                   // Vertex buffer offset = CmdSetVertexBuffers.m_offset + m_baseVertex * VertexStreamDesc::m_stride
        uint32  m_baseInstance;
    };

    struct DrawIndexedBaseDesc
    {
        uint32  m_shaderEmulatedBaseVertex;     // Root constant.
        uint32  m_shaderEmulatedBaseInstance;   // Root constant.
        uint32  m_indexNum;
        uint32  m_instanceNum;
        uint32  m_baseIndex;                    // Index buffer offset = CmdSetIndexBuffer.m_offset + m_baseIndex * sizeof(CmdSetIndexBuffer.m_indexType)
        int32   m_baseVertex;                   // index += baseVertex;
        uint32  m_baseInstance;
    };

#pragma endregion

//============================================================================================================================================================================================
#pragma region [ Other ]
//============================================================================================================================================================================================

    //----------------------------------------------------------------------------------------------------
    /// @brief : Decribes a region of a texture for copy commands. 
    //----------------------------------------------------------------------------------------------------
    struct TextureRegionDesc
    {
        DimType     x;
        DimType     y;
        DimType     z;
        DimType     m_width;
        DimType     m_height;
        DimType     m_depth;
        DimType     m_mipOffset;
        DimType     m_layerOffset;
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
        GFence*     m_pFence;
        uint64      m_value;
        EPipelineStageBits  m_stages;
    };

    struct QueueSubmitDesc
    {
        const FenceSubmitDesc*  m_pWaitFences;
        uint32                  m_waitFenceNum;
        const GCommandBuffer*   m_pCommandBuffers;
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
        const Descriptor*      m_pStorage; // A "Storage" descriptor.
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

    struct PhysicalDeviceDesc
    {
        char                m_name[256]{};                                          /// Name of the Device.
        uint32              m_deviceID{};                                           /// Unique identifier for the Device.
        uint32              m_driverVersion{};
        uint32              m_apiSupport{};
        uint64              m_videoMemorySize{};
        uint64              m_sharedSystemMemorySize{};
        uint32              m_numQueuesByType[static_cast<uint32>(EQueueType::MaxNum)];
        uint32              m_queueFamilyIndices[static_cast<uint32>(EQueueType::MaxNum)]; 
        EVendor             m_vendor = EVendor::Unknown;                            /// Vendor that made the device.
        EPhysicalDeviceType m_architecture = EPhysicalDeviceType::Unknown;          /// What type of device it is.
    };

    //----------------------------------------------------------------------------------------------------
    /// @brief : Information about the hardware device. Including feature support coverage.
    //      https://vulkan.gpuinfo.org/
    //----------------------------------------------------------------------------------------------------
    struct DeviceDesc
    {
        PhysicalDeviceDesc  m_physicalDeviceDesc;
        EGraphicsAPI        m_graphicsAPI = EGraphicsAPI::Vulkan;
    };
#pragma endregion
}
