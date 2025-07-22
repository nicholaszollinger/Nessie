// Helpers.h
#pragma once
#include "GraphicsCommon.h"

NES_BEGIN_GRAPHICS_NAMESPACE

struct VideoMemoryInfo
{
    uint64              m_budgetSize;    /// The OS-provided video memory budget. If "m_usageSize" > "m_budgetSize", the application may incur stuttering or performance penalties
    uint64              m_usageSize;     /// Specifies the application’s current video memory usage.
};

struct TextureSubresourceUploadDesc
{
    const void*         m_pSlices;
    uint32              m_numSlices;
    uint32              m_rowPitch;
    uint32              m_slicePitch;
};

struct TextureUploadDesc
{
    const TextureSubresourceUploadDesc* m_pSubresources; // If provided, must include ALL subresources = layerNum * mipNum
    GTexture*           m_pTexture;
    AccessLayoutStage   m_after;
    EPlaneBits          m_planes;
};

struct BufferUploadDesc
{
    const void*         m_pInitialData; // If provided, must be the data for the whole buffer.
    GBuffer*            m_pBuffer;
    AccessStage         m_after;
};

struct ResourceGroupDesc
{
    EMemoryLocation     m_memoryLocation;
    GTexture* const*    m_pTextures;
    uint32              m_numTextures;
    GBuffer* const*     m_pBuffers;
    uint32              m_numBuffers;
    uint64              m_preferredMemorySize; // Desired chunk size (but can be greater if a resource doesn't fit), 256 Mb if 0
};

struct FormatProps
{
    const char*         m_name;                 // Format name.
    EFormat             m_format;               // The format itself.
    uint8               m_redBits;              // R (or depth) bits.
    uint8               m_greenBits;            // G (or stencil) bits (is 0 if channels < 2).
    uint8               m_blueBits;             // B bits (is 0 if channels < 3).
    uint8               m_alphaBits;            // A (or shared exponent) bits (is 0 if channels < 4).
    uint32              m_stride        : 6;    // Block size in bytes.
    uint32              m_blockWidth    : 4;    // 1 for plain formats, >1 for compressed.
    uint32              m_blockHeight   : 4;    // 1 for plain formats, >1 for compressed
    uint32              m_isBGR         : 1;    // Reversed channels (RGBA => BGRA).
    uint32              m_isCompressed  : 1;    // Block-compressed format.
    uint32              m_isDepth       : 1;    // Has a depth component.
    uint32              m_isExpShared   : 1;    // Has a shared exponent in the alpha channel.
    uint32              m_isFloat       : 1;    // Floating point.
    uint32              m_isPacked      : 1;    // 16- or 32- bit packed.
    uint32              m_isInteger     : 1;    // Integer
    uint32              m_isNorm        : 1;    // Is normalized [0, 1].
    uint32              m_isSigned      : 1;    // Signed.
    uint32              m_isSRGB        : 1;    // sRGB format.
    uint32              m_isStencil     : 1;    // Has a stencil component.
    uint32              m_unused        : 7;    
};

//----------------------------------------------------------------------------------------------------
/// @brief : Get the properties of a particular format.
//----------------------------------------------------------------------------------------------------
const FormatProps&      GetFormatProps(const EFormat format); 

//----------------------------------------------------------------------------------------------------
/// @brief : Thread-safe helper functions. 
//----------------------------------------------------------------------------------------------------
class HelperInterface
{
public:
    //----------------------------------------------------------------------------------------------------
    /// @brief : Calculate the number of allocations for a resource group. 
    //----------------------------------------------------------------------------------------------------
    uint32              (*CalculateAllocationNumber) (const Device& device, const ResourceGroupDesc& resourceGroup);

    //----------------------------------------------------------------------------------------------------
    /// @brief : Optimized memory allocation for a group of resources.
    ///     "pOutAllocations" must have entries >= returned by "m_calculateAllocationNumber"
    //----------------------------------------------------------------------------------------------------
    EGraphicsErrorCodes             (*AllocateAndBindMemory) (Device& device, const ResourceGroupDesc& resourceGroup, GMemory* pOutAllocations);

    //----------------------------------------------------------------------------------------------------
    /// @brief : Populate resources with data (not for streaming!). 
    //----------------------------------------------------------------------------------------------------
    EGraphicsErrorCodes             (*UploadData) (DeviceQueue& queue, const TextureUploadDesc* pTextureUploads, const uint32 textureUploadNum, const BufferUploadDesc* pBufferUploads, const uint32 bufferUploadNum);

    //----------------------------------------------------------------------------------------------------
    /// @brief : Get information about video memory. 
    //----------------------------------------------------------------------------------------------------
    EGraphicsErrorCodes             (*QueryVideoMemoryInfo) (const Device& device, const EMemoryLocation location, VideoMemoryInfo& outVideoMemoryInfo);
};

//----------------------------------------------------------------------------------------------------
/// @brief : Get the string representation of a Graphics API type.
//----------------------------------------------------------------------------------------------------
const char*             GetGraphicsAPIString(const EGraphicsAPI graphicsAPI);

//----------------------------------------------------------------------------------------------------
/// @brief : A convenient way to fit pipeline layout settings into the device limits, respecting various restrictions 
//----------------------------------------------------------------------------------------------------
struct PipelineLayoutSettingsDesc
{
    uint32 m_numDescriptorSets;
    uint32 m_numDescriptorRanges;
    uint32 m_rootConstantSize;
    uint32 m_numRootDescriptors;
    bool   m_preferRootDescriptorsOverConstants = false;
    bool   m_enableD3D12DrawParametersEmulation = false; // Not needed in Vulkan, unsupported in D3D11
};

//----------------------------------------------------------------------------------------------------
/// @brief : Ensure the pipelineLayoutSettings values are within the device limits.
//----------------------------------------------------------------------------------------------------
static inline PipelineLayoutSettingsDesc FitPipelineLayoutSettingsIntoDeviceLimits(const DeviceDesc& deviceDesc, const PipelineLayoutSettingsDesc& pipelineLayoutSettings)
{
    uint32 numDescriptorSets = pipelineLayoutSettings.m_numDescriptorSets;
    uint32 numDescriptorRanges = pipelineLayoutSettings.m_numDescriptorRanges;
    uint32 rootConstantSize = pipelineLayoutSettings.m_rootConstantSize;
    uint32 numRootDescriptors = pipelineLayoutSettings.m_numRootDescriptors;

    // Apply global limits
    rootConstantSize = nes::math::Min(deviceDesc.m_pipelineLayout.m_rootConstantMaxSize, rootConstantSize);
    numRootDescriptors = nes::math::Min(deviceDesc.m_pipelineLayout.m_maxNumRootDescriptors, numRootDescriptors);

    // Get the maximum number of pipeline layout descriptor sets.
    uint32 maxNumPipelineLayoutDescriptorSets = deviceDesc.m_pipelineLayout.m_maxNumDescriptorSets;
    
    
    if (numRootDescriptors)
        --maxNumPipelineLayoutDescriptorSets;

    numDescriptorSets = nes::math::Min(maxNumPipelineLayoutDescriptorSets, numDescriptorSets);

    // Return the modified properties.
    PipelineLayoutSettingsDesc modifiedPipelineLayoutLimits = pipelineLayoutSettings;
    modifiedPipelineLayoutLimits.m_numDescriptorSets = numDescriptorSets;
    modifiedPipelineLayoutLimits.m_numDescriptorRanges = numDescriptorRanges;
    modifiedPipelineLayoutLimits.m_rootConstantSize = rootConstantSize;
    modifiedPipelineLayoutLimits.m_numRootDescriptors = numRootDescriptors;
    return modifiedPipelineLayoutLimits;
}

static inline GTextureBarrierDesc CreateTextureBarrierFromState(GTextureBarrierDesc& prevState, const AccessLayoutStage after, const DimType mipOffset = 0, const DimType mipNum = 0)
{
    prevState.m_baseMip = mipOffset;
    prevState.m_NumMips = mipNum;
    prevState.m_before = prevState.m_after;
    prevState.m_after = after;
    return prevState;
}

NES_END_GRAPHICS_NAMESPACE