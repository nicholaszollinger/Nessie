// VulkanConversions.h
#pragma once
#include "VulkanCore.h"
#include "Nessie/Graphics/GraphicsCommon.h"

namespace nes
{
    template <typename Type>
    concept VulkanObjectType = requires
    {
        std::same_as<Type, VkBuffer>
        || std::same_as<Type, VkBufferView>
        || std::same_as<Type, VkCommandBuffer>
        || std::same_as<Type, VkCommandPool>
        || std::same_as<Type, VkDescriptorPool>
        || std::same_as<Type, VkDescriptorSet>
        || std::same_as<Type, VkDescriptorSetLayout>
        || std::same_as<Type, VkDevice>
        || std::same_as<Type, VkDeviceMemory>
        || std::same_as<Type, VkFence>
        || std::same_as<Type, VkFramebuffer>
        || std::same_as<Type, VkImage>
        || std::same_as<Type, VkImageView>
        || std::same_as<Type, VkInstance>
        || std::same_as<Type, VkPipeline>
        || std::same_as<Type, VkPipelineCache>
        || std::same_as<Type, VkPipelineLayout>
        || std::same_as<Type, VkQueryPool>
        || std::same_as<Type, VkRenderPass>
        || std::same_as<Type, VkSampler>
        || std::same_as<Type, VkSemaphore>
        || std::same_as<Type, VkShaderModule>
        || std::same_as<Type, VkSurfaceKHR>
        || std::same_as<Type, VkSwapchainKHR>
        || std::same_as<Type, VkPhysicalDevice>
        || std::same_as<Type, VkQueue>
        || std::same_as<Type, VkShaderEXT>
        || std::same_as<Type, VkAccelerationStructureKHR>;
    };
    
    //----------------------------------------------------------------------------------------------------
    /// @brief : Convert to EVendor from a raw vendor id value.
    //----------------------------------------------------------------------------------------------------
    constexpr EVendor      GetVendorFromID(const uint32 vendorID)
    {
        switch (vendorID)
        {
            case 0x10DE:
                return EVendor::NVIDIA;
            case 0x1002:
                return EVendor::AMD;
            case 0x8086:
                return EVendor::INTEL;
            
            default: break;
        }
        return EVendor::Unknown;
    }

    constexpr EPhysicalDeviceType GetPhysicalDeviceTypeFromVulkanType(const VkPhysicalDeviceType type)
    {
        switch (type)
        {
            case VK_PHYSICAL_DEVICE_TYPE_INTEGRATED_GPU: return EPhysicalDeviceType::Integrated;
            case VK_PHYSICAL_DEVICE_TYPE_DISCRETE_GPU: return EPhysicalDeviceType::DiscreteGPU;
            case VK_PHYSICAL_DEVICE_TYPE_VIRTUAL_GPU: return EPhysicalDeviceType::VirtualGPU;
            case VK_PHYSICAL_DEVICE_TYPE_CPU: return EPhysicalDeviceType::CPU;

            default: return EPhysicalDeviceType::Unknown;
        }
    }

    //----------------------------------------------------------------------------------------------------
    /// @brief : Convert from a VkResult to an Error code.
    //----------------------------------------------------------------------------------------------------
    constexpr EGraphicsResult ConvertVkResultToGraphics(const VkResult vkResult)
    {
        if (vkResult >= 0)
            return EGraphicsResult::Success;
    
        switch (vkResult)
        {
            case VK_ERROR_INITIALIZATION_FAILED:
                return EGraphicsResult::InitializationFailed;
            
            case VK_ERROR_DEVICE_LOST:
                return EGraphicsResult::DeviceLost;

            case VK_ERROR_SURFACE_LOST_KHR:
            case VK_ERROR_OUT_OF_DATE_KHR:
                return EGraphicsResult::SwapchainOutOfDate;
        
            case VK_ERROR_NATIVE_WINDOW_IN_USE_KHR:
            case VK_ERROR_INCOMPATIBLE_DISPLAY_KHR:
            case VK_ERROR_FORMAT_NOT_SUPPORTED:
            case VK_ERROR_INCOMPATIBLE_DRIVER:
            case VK_ERROR_FEATURE_NOT_PRESENT:
            case VK_ERROR_EXTENSION_NOT_PRESENT:
            case VK_ERROR_LAYER_NOT_PRESENT:
                return EGraphicsResult::Unsupported;

            case VK_ERROR_OUT_OF_HOST_MEMORY:
            case VK_ERROR_OUT_OF_DEVICE_MEMORY:
            case VK_ERROR_OUT_OF_POOL_MEMORY:
            case VK_ERROR_FRAGMENTATION:
            case VK_ERROR_FRAGMENTED_POOL:
                return EGraphicsResult::OutOfMemory;

            default:
                return EGraphicsResult::Failure;
        }
    }

    constexpr std::array<EFormat, 131> kVulkanFormatTable =
    {
        EFormat::Unknown,                // VK_FORMAT_UNDEFINED = 0
        EFormat::Unknown,                // VK_FORMAT_R4G4_UNORM_PACK8 = 1
        EFormat::Unknown,                // VK_FORMAT_R4G4B4A4_UNORM_PACK16 = 2
        EFormat::Unknown,                // VK_FORMAT_B4G4R4A4_UNORM_PACK16 = 3
        EFormat::B5_G6_R5_UNORM,         // VK_FORMAT_R5G6B5_UNORM_PACK16 = 4
        EFormat::Unknown,                // VK_FORMAT_B5G6R5_UNORM_PACK16 = 5
        EFormat::Unknown,                // VK_FORMAT_R5G5B5A1_UNORM_PACK16 = 6
        EFormat::Unknown,                // VK_FORMAT_B5G5R5A1_UNORM_PACK16 = 7
        EFormat::B5_G5_R5_A1_UNORM,      // VK_FORMAT_A1R5G5B5_UNORM_PACK16 = 8
        EFormat::R8_UNORM,               // VK_FORMAT_R8_UNORM = 9
        EFormat::R8_SNORM,               // VK_FORMAT_R8_SNORM = 10
        EFormat::Unknown,                // VK_FORMAT_R8_USCALED = 11
        EFormat::Unknown,                // VK_FORMAT_R8_SSCALED = 12
        EFormat::R8_UINT,                // VK_FORMAT_R8_UINT = 13
        EFormat::R8_SINT,                // VK_FORMAT_R8_SINT = 14
        EFormat::Unknown,                // VK_FORMAT_R8_SRGB = 15
        EFormat::RG8_UNORM,              // VK_FORMAT_R8G8_UNORM = 16
        EFormat::RG8_SNORM,              // VK_FORMAT_R8G8_SNORM = 17
        EFormat::Unknown,                // VK_FORMAT_R8G8_USCALED = 18
        EFormat::Unknown,                // VK_FORMAT_R8G8_SSCALED = 19
        EFormat::RG8_UINT,               // VK_FORMAT_R8G8_UINT = 20
        EFormat::RG8_SINT,               // VK_FORMAT_R8G8_SINT = 21
        EFormat::Unknown,                // VK_FORMAT_R8G8_SRGB = 22
        EFormat::Unknown,                // VK_FORMAT_R8G8B8_UNORM = 23
        EFormat::Unknown,                // VK_FORMAT_R8G8B8_SNORM = 24
        EFormat::Unknown,                // VK_FORMAT_R8G8B8_USCALED = 25
        EFormat::Unknown,                // VK_FORMAT_R8G8B8_SSCALED = 26
        EFormat::Unknown,                // VK_FORMAT_R8G8B8_UINT = 27
        EFormat::Unknown,                // VK_FORMAT_R8G8B8_SINT = 28
        EFormat::Unknown,                // VK_FORMAT_R8G8B8_SRGB = 29
        EFormat::Unknown,                // VK_FORMAT_B8G8R8_UNORM = 30
        EFormat::Unknown,                // VK_FORMAT_B8G8R8_SNORM = 31
        EFormat::Unknown,                // VK_FORMAT_B8G8R8_USCALED = 32
        EFormat::Unknown,                // VK_FORMAT_B8G8R8_SSCALED = 33
        EFormat::Unknown,                // VK_FORMAT_B8G8R8_UINT = 34
        EFormat::Unknown,                // VK_FORMAT_B8G8R8_SINT = 35
        EFormat::Unknown,                // VK_FORMAT_B8G8R8_SRGB = 36
        EFormat::RGBA8_UNORM,            // VK_FORMAT_R8G8B8A8_UNORM = 37
        EFormat::RGBA8_SNORM,            // VK_FORMAT_R8G8B8A8_SNORM = 38
        EFormat::Unknown,                // VK_FORMAT_R8G8B8A8_USCALED = 39
        EFormat::Unknown,                // VK_FORMAT_R8G8B8A8_SSCALED = 40
        EFormat::RGBA8_UINT,             // VK_FORMAT_R8G8B8A8_UINT = 41
        EFormat::RGBA8_SINT,             // VK_FORMAT_R8G8B8A8_SINT = 42
        EFormat::RGBA8_SRGB,             // VK_FORMAT_R8G8B8A8_SRGB = 43
        EFormat::BGRA8_UNORM,            // VK_FORMAT_B8G8R8A8_UNORM = 44
        EFormat::Unknown,                // VK_FORMAT_B8G8R8A8_SNORM = 45
        EFormat::Unknown,                // VK_FORMAT_B8G8R8A8_USCALED = 46
        EFormat::Unknown,                // VK_FORMAT_B8G8R8A8_SSCALED = 47
        EFormat::Unknown,                // VK_FORMAT_B8G8R8A8_UINT = 48
        EFormat::Unknown,                // VK_FORMAT_B8G8R8A8_SINT = 49
        EFormat::BGRA8_SRGB,             // VK_FORMAT_B8G8R8A8_SRGB = 50
        EFormat::Unknown,                // VK_FORMAT_A8B8G8R8_UNORM_PACK32 = 51
        EFormat::Unknown,                // VK_FORMAT_A8B8G8R8_SNORM_PACK32 = 52
        EFormat::Unknown,                // VK_FORMAT_A8B8G8R8_USCALED_PACK32 = 53
        EFormat::Unknown,                // VK_FORMAT_A8B8G8R8_SSCALED_PACK32 = 54
        EFormat::Unknown,                // VK_FORMAT_A8B8G8R8_UINT_PACK32 = 55
        EFormat::Unknown,                // VK_FORMAT_A8B8G8R8_SINT_PACK32 = 56
        EFormat::Unknown,                // VK_FORMAT_A8B8G8R8_SRGB_PACK32 = 57
        EFormat::Unknown,                // VK_FORMAT_A2R10G10B10_UNORM_PACK32 = 58
        EFormat::Unknown,                // VK_FORMAT_A2R10G10B10_SNORM_PACK32 = 59
        EFormat::Unknown,                // VK_FORMAT_A2R10G10B10_USCALED_PACK32 = 60
        EFormat::Unknown,                // VK_FORMAT_A2R10G10B10_SSCALED_PACK32 = 61
        EFormat::Unknown,                // VK_FORMAT_A2R10G10B10_UINT_PACK32 = 62
        EFormat::Unknown,                // VK_FORMAT_A2R10G10B10_SINT_PACK32 = 63
        EFormat::R10_G10_B10_A2_UNORM,   // VK_FORMAT_A2B10G10R10_UNORM_PACK32 = 64
        EFormat::Unknown,                // VK_FORMAT_A2B10G10R10_SNORM_PACK32 = 65
        EFormat::Unknown,                // VK_FORMAT_A2B10G10R10_USCALED_PACK32 = 66
        EFormat::Unknown,                // VK_FORMAT_A2B10G10R10_SSCALED_PACK32 = 67
        EFormat::R10_G10_B10_A2_UINT,    // VK_FORMAT_A2B10G10R10_UINT_PACK32 = 68
        EFormat::Unknown,                // VK_FORMAT_A2B10G10R10_SINT_PACK32 = 69
        EFormat::R16_UNORM,              // VK_FORMAT_R16_UNORM = 70
        EFormat::R16_SNORM,              // VK_FORMAT_R16_SNORM = 71
        EFormat::Unknown,                // VK_FORMAT_R16_USCALED = 72
        EFormat::Unknown,                // VK_FORMAT_R16_SSCALED = 73
        EFormat::R16_UINT,               // VK_FORMAT_R16_UINT = 74
        EFormat::R16_SINT,               // VK_FORMAT_R16_SINT = 75
        EFormat::R16_SFLOAT,             // VK_FORMAT_R16_SFLOAT = 76
        EFormat::RG16_UNORM,             // VK_FORMAT_R16G16_UNORM = 77
        EFormat::RG16_SNORM,             // VK_FORMAT_R16G16_SNORM = 78
        EFormat::Unknown,                // VK_FORMAT_R16G16_USCALED = 79
        EFormat::Unknown,                // VK_FORMAT_R16G16_SSCALED = 80
        EFormat::RG16_UINT,              // VK_FORMAT_R16G16_UINT = 81
        EFormat::RG16_SINT,              // VK_FORMAT_R16G16_SINT = 82
        EFormat::RG16_SFLOAT,            // VK_FORMAT_R16G16_SFLOAT = 83
        EFormat::Unknown,                // VK_FORMAT_R16G16B16_UNORM = 84
        EFormat::Unknown,                // VK_FORMAT_R16G16B16_SNORM = 85
        EFormat::Unknown,                // VK_FORMAT_R16G16B16_USCALED = 86
        EFormat::Unknown,                // VK_FORMAT_R16G16B16_SSCALED = 87
        EFormat::Unknown,                // VK_FORMAT_R16G16B16_UINT = 88
        EFormat::Unknown,                // VK_FORMAT_R16G16B16_SINT = 89
        EFormat::Unknown,                // VK_FORMAT_R16G16B16_SFLOAT = 90
        EFormat::RGBA16_UNORM,           // VK_FORMAT_R16G16B16A16_UNORM = 91
        EFormat::RGBA16_SNORM,           // VK_FORMAT_R16G16B16A16_SNORM = 92
        EFormat::Unknown,                // VK_FORMAT_R16G16B16A16_USCALED = 93
        EFormat::Unknown,                // VK_FORMAT_R16G16B16A16_SSCALED = 94
        EFormat::RGBA16_UINT,            // VK_FORMAT_R16G16B16A16_UINT = 95
        EFormat::RGBA16_SINT,            // VK_FORMAT_R16G16B16A16_SINT = 96
        EFormat::RGBA16_SFLOAT,          // VK_FORMAT_R16G16B16A16_SFLOAT = 97
        EFormat::R32_UINT,               // VK_FORMAT_R32_UINT = 98
        EFormat::R32_SINT,               // VK_FORMAT_R32_SINT = 99
        EFormat::R32_SFLOAT,             // VK_FORMAT_R32_SFLOAT = 100
        EFormat::RG32_UINT,              // VK_FORMAT_R32G32_UINT = 101
        EFormat::RG32_SINT,              // VK_FORMAT_R32G32_SINT = 102
        EFormat::RG32_SFLOAT,            // VK_FORMAT_R32G32_SFLOAT = 103
        EFormat::RGB32_UINT,             // VK_FORMAT_R32G32B32_UINT = 104
        EFormat::RGB32_SINT,             // VK_FORMAT_R32G32B32_SINT = 105
        EFormat::RGB32_SFLOAT,           // VK_FORMAT_R32G32B32_SFLOAT = 106
        EFormat::RGBA32_UINT,            // VK_FORMAT_R32G32B32A32_UINT = 107
        EFormat::RGBA32_SINT,            // VK_FORMAT_R32G32B32A32_SINT = 108
        EFormat::RGBA32_SFLOAT,          // VK_FORMAT_R32G32B32A32_SFLOAT = 109
        EFormat::Unknown,                // VK_FORMAT_R64_UINT = 110
        EFormat::Unknown,                // VK_FORMAT_R64_SINT = 111
        EFormat::Unknown,                // VK_FORMAT_R64_SFLOAT = 112
        EFormat::Unknown,                // VK_FORMAT_R64G64_UINT = 113
        EFormat::Unknown,                // VK_FORMAT_R64G64_SINT = 114
        EFormat::Unknown,                // VK_FORMAT_R64G64_SFLOAT = 115
        EFormat::Unknown,                // VK_FORMAT_R64G64B64_UINT = 116
        EFormat::Unknown,                // VK_FORMAT_R64G64B64_SINT = 117
        EFormat::Unknown,                // VK_FORMAT_R64G64B64_SFLOAT = 118
        EFormat::Unknown,                // VK_FORMAT_R64G64B64A64_UINT = 119
        EFormat::Unknown,                // VK_FORMAT_R64G64B64A64_SINT = 120
        EFormat::Unknown,                // VK_FORMAT_R64G64B64A64_SFLOAT = 121
        EFormat::R11_G11_B10_UFLOAT,     // VK_FORMAT_B10G11R11_UFLOAT_PACK32 = 122
        EFormat::R9_G9_B9_E5_UFLOAT,     // VK_FORMAT_E5B9G9R9_UFLOAT_PACK32 = 123
        EFormat::D16_UNORM,              // VK_FORMAT_D16_UNORM = 124
        EFormat::D24_UNORM_S8_UINT,      // VK_FORMAT_X8_D24_UNORM_PACK32 = 125
        EFormat::D32_SFLOAT,             // VK_FORMAT_D32_SFLOAT = 126
        EFormat::Unknown,                // VK_FORMAT_S8_UINT = 127
        EFormat::Unknown,                // VK_FORMAT_D16_UNORM_S8_UINT = 128
        EFormat::D24_UNORM_S8_UINT,      // VK_FORMAT_D24_UNORM_S8_UINT = 129
        EFormat::D32_SFLOAT_S8_UINT_X24, // VK_FORMAT_D32_SFLOAT_S8_UINT = 130
    };

    // Each depth/stencil format is only compatible with itself in Vulkan
    constexpr std::array<VkFormat, static_cast<size_t>(EFormat::MaxNum)> kVulkanFormats =
    {
        VK_FORMAT_UNDEFINED,                // Unknown
        VK_FORMAT_R8_UNORM,                 // R8_UNORM
        VK_FORMAT_R8_SNORM,                 // R8_SNORM
        VK_FORMAT_R8_UINT,                  // R8_UINT
        VK_FORMAT_R8_SINT,                  // R8_SINT
        VK_FORMAT_R8G8_UNORM,               // RG8_UNORM
        VK_FORMAT_R8G8_SNORM,               // RG8_SNORM
        VK_FORMAT_R8G8_UINT,                // RG8_UINT
        VK_FORMAT_R8G8_SINT,                // RG8_SINT
        VK_FORMAT_B8G8R8A8_UNORM,           // BGRA8_UNORM
        VK_FORMAT_B8G8R8A8_SRGB,            // BGRA8_SRGB
        VK_FORMAT_R8G8B8A8_UNORM,           // RGBA8_UNORM
        VK_FORMAT_R8G8B8A8_SRGB,            // RGBA8_SRGB
        VK_FORMAT_R8G8B8A8_SNORM,           // RGBA8_SNORM
        VK_FORMAT_R8G8B8A8_UINT,            // RGBA8_UINT
        VK_FORMAT_R8G8B8A8_SINT,            // RGBA8_SINT
        VK_FORMAT_R16_UNORM,                // R16_UNORM
        VK_FORMAT_R16_SNORM,                // R16_SNORM
        VK_FORMAT_R16_UINT,                 // R16_UINT
        VK_FORMAT_R16_SINT,                 // R16_SINT
        VK_FORMAT_R16_SFLOAT,               // R16_SFLOAT
        VK_FORMAT_R16G16_UNORM,             // RG16_UNORM
        VK_FORMAT_R16G16_SNORM,             // RG16_SNORM
        VK_FORMAT_R16G16_UINT,              // RG16_UINT
        VK_FORMAT_R16G16_SINT,              // RG16_SINT
        VK_FORMAT_R16G16_SFLOAT,            // RG16_SFLOAT
        VK_FORMAT_R16G16B16A16_UNORM,       // RGBA16_UNORM
        VK_FORMAT_R16G16B16A16_SNORM,       // RGBA16_SNORM
        VK_FORMAT_R16G16B16A16_UINT,        // RGBA16_UINT
        VK_FORMAT_R16G16B16A16_SINT,        // RGBA16_SINT
        VK_FORMAT_R16G16B16A16_SFLOAT,      // RGBA16_SFLOAT
        VK_FORMAT_R32_UINT,                 // R32_UINT
        VK_FORMAT_R32_SINT,                 // R32_SINT
        VK_FORMAT_R32_SFLOAT,               // R32_SFLOAT
        VK_FORMAT_R32G32_UINT,              // RG32_UINT
        VK_FORMAT_R32G32_SINT,              // RG32_SINT
        VK_FORMAT_R32G32_SFLOAT,            // RG32_SFLOAT
        VK_FORMAT_R32G32B32_UINT,           // RGB32_UINT
        VK_FORMAT_R32G32B32_SINT,           // RGB32_SINT
        VK_FORMAT_R32G32B32_SFLOAT,         // RGB32_SFLOAT
        VK_FORMAT_R32G32B32A32_UINT,        // RGB32_UINT
        VK_FORMAT_R32G32B32A32_SINT,        // RGB32_SINT
        VK_FORMAT_R32G32B32A32_SFLOAT,      // RGB32_SFLOAT
        VK_FORMAT_R5G6B5_UNORM_PACK16,      // B5_G6_R5_UNORM
        VK_FORMAT_A1R5G5B5_UNORM_PACK16,    // B5_G5_R5_A1_UNORM
        VK_FORMAT_A4R4G4B4_UNORM_PACK16,    // B4_G4_R4_A4_UNORM
        VK_FORMAT_A2B10G10R10_UNORM_PACK32, // R10_G10_B10_A2_UNORM
        VK_FORMAT_A2B10G10R10_UINT_PACK32,  // R10_G10_B10_A2_UINT
        VK_FORMAT_B10G11R11_UFLOAT_PACK32,  // R11_G11_B10_UFLOAT
        VK_FORMAT_E5B9G9R9_UFLOAT_PACK32,   // R9_G9_B9_E5_UFLOAT
        VK_FORMAT_BC1_RGBA_UNORM_BLOCK,     // BC1_RGBA_UNORM
        VK_FORMAT_BC1_RGBA_SRGB_BLOCK,      // BC1_RGBA_SRGB
        VK_FORMAT_BC2_UNORM_BLOCK,          // BC2_RGBA_UNORM
        VK_FORMAT_BC2_SRGB_BLOCK,           // BC2_RGBA_SRGB
        VK_FORMAT_BC3_UNORM_BLOCK,          // BC3_RGBA_UNORM
        VK_FORMAT_BC3_SRGB_BLOCK,           // BC3_RGBA_SRGB
        VK_FORMAT_BC4_UNORM_BLOCK,          // BC4_R_UNORM
        VK_FORMAT_BC4_SNORM_BLOCK,          // BC4_R_SNORM
        VK_FORMAT_BC5_UNORM_BLOCK,          // BC5_RG_UNORM
        VK_FORMAT_BC5_SNORM_BLOCK,          // BC5_RG_SNORM
        VK_FORMAT_BC6H_UFLOAT_BLOCK,        // BC6H_RGB_UFLOAT
        VK_FORMAT_BC6H_SFLOAT_BLOCK,        // BC6H_RGB_SFLOAT
        VK_FORMAT_BC7_UNORM_BLOCK,          // BC7_RGBA_UNORM
        VK_FORMAT_BC7_SRGB_BLOCK,           // BC7_RGBA_SRGB
        VK_FORMAT_D16_UNORM,                // D16_UNORM
        VK_FORMAT_D24_UNORM_S8_UINT,        // D24_UNORM_S8_UINT
        VK_FORMAT_D32_SFLOAT,               // D32_SFLOAT
        VK_FORMAT_D32_SFLOAT_S8_UINT,       // D32_SFLOAT_S8_UINT_X24
        VK_FORMAT_D24_UNORM_S8_UINT,        // R24_UNORM_X8
        VK_FORMAT_D24_UNORM_S8_UINT,        // X24_G8_UINT
        VK_FORMAT_D32_SFLOAT_S8_UINT,       // R32_SFLOAT_X8_X24
        VK_FORMAT_D32_SFLOAT_S8_UINT,       // X32_G8_UINT_X24
    };

    constexpr EFormat ConvertFromVKFormat(const uint32 vkFormat)
    {
        if (vkFormat < kVulkanFormatTable.size())
            return kVulkanFormatTable[vkFormat];
        else if (vkFormat == VK_FORMAT_A4R4G4B4_UNORM_PACK16)
            return EFormat::B4_G4_R4_A4_UNORM;

        return EFormat::Unknown;
    }

    constexpr uint32 ConvertToVKFormat(const EFormat format)
    {
        return static_cast<uint32>(kVulkanFormats[static_cast<uint32>(format)]);
    }

    constexpr EQueryType ConvertFromVkQueryType(const uint32 queryTypeVk)
    {
        if (queryTypeVk == VK_QUERY_TYPE_OCCLUSION)
            return EQueryType::Occlusion;

        if (queryTypeVk == VK_QUERY_TYPE_PIPELINE_STATISTICS)
            return EQueryType::PipelineStatistics;

        if (queryTypeVk == VK_QUERY_TYPE_ACCELERATION_STRUCTURE_SIZE_KHR)
            return EQueryType::AccelerationStructureSize;

        if (queryTypeVk == VK_QUERY_TYPE_ACCELERATION_STRUCTURE_COMPACTED_SIZE_KHR)
            return EQueryType::AccelerationStructureCompactedSize;

        if (queryTypeVk == VK_QUERY_TYPE_MICROMAP_COMPACTED_SIZE_EXT)
            return EQueryType::MicromapCompactedSize;

        return EQueryType::MaxNum;
    }

    //----------------------------------------------------------------------------------------------------
    /// @brief : Get the VkObjectType value from a Vulkan Type.
    //----------------------------------------------------------------------------------------------------
    template <VulkanObjectType Type>
    constexpr VkObjectType GetVulkanObjectType()
    {
        if constexpr(std::is_same_v<Type, VkBuffer>)
            return VK_OBJECT_TYPE_BUFFER;
        else if constexpr(std::is_same_v<Type, VkBufferView>)
            return VK_OBJECT_TYPE_BUFFER_VIEW;
        else if constexpr(std::is_same_v<Type, VkCommandBuffer>)
            return VK_OBJECT_TYPE_COMMAND_BUFFER;
        else if constexpr(std::is_same_v<Type, VkCommandPool>)
            return VK_OBJECT_TYPE_COMMAND_POOL;
        else if constexpr(std::is_same_v<Type, VkDescriptorPool>)
            return VK_OBJECT_TYPE_DESCRIPTOR_POOL;
        else if constexpr(std::is_same_v<Type, VkDescriptorSet>)
            return VK_OBJECT_TYPE_DESCRIPTOR_SET;
        else if constexpr(std::is_same_v<Type, VkDescriptorSetLayout>)
            return VK_OBJECT_TYPE_DESCRIPTOR_SET_LAYOUT;
        else if constexpr(std::is_same_v<Type, VkDevice>)
            return VK_OBJECT_TYPE_DEVICE;
        else if constexpr(std::is_same_v<Type, VkDeviceMemory>)
            return VK_OBJECT_TYPE_DEVICE_MEMORY;
        else if constexpr(std::is_same_v<Type, VkFence>)
            return VK_OBJECT_TYPE_FENCE;
        else if constexpr(std::is_same_v<Type, VkFramebuffer>)
            return VK_OBJECT_TYPE_FRAMEBUFFER;
        else if constexpr(std::is_same_v<Type, VkImage>)
            return VK_OBJECT_TYPE_IMAGE;
        else if constexpr(std::is_same_v<Type, VkImageView>)
            return VK_OBJECT_TYPE_IMAGE_VIEW;
        else if constexpr(std::is_same_v<Type, VkInstance>)
            return VK_OBJECT_TYPE_INSTANCE;
        else if constexpr(std::is_same_v<Type, VkPipeline>)
            return VK_OBJECT_TYPE_PIPELINE;
        else if constexpr(std::is_same_v<Type, VkPipelineCache>)
            return VK_OBJECT_TYPE_PIPELINE_CACHE;
        else if constexpr(std::is_same_v<Type, VkPipelineLayout>)
            return VK_OBJECT_TYPE_PIPELINE_LAYOUT;
        else if constexpr(std::is_same_v<Type, VkQueryPool>)
            return VK_OBJECT_TYPE_QUERY_POOL;
        else if constexpr(std::is_same_v<Type, VkRenderPass>)
            return VK_OBJECT_TYPE_RENDER_PASS;
        else if constexpr(std::is_same_v<Type, VkSampler>)
            return VK_OBJECT_TYPE_SAMPLER;
        else if constexpr(std::is_same_v<Type, VkSemaphore>)
            return VK_OBJECT_TYPE_SEMAPHORE;
        else if constexpr(std::is_same_v<Type, VkShaderModule>)
            return VK_OBJECT_TYPE_SHADER_MODULE;
        else if constexpr(std::is_same_v<Type, VkSurfaceKHR>)
            return VK_OBJECT_TYPE_SURFACE_KHR;
        else if constexpr(std::is_same_v<Type, VkSwapchainKHR>)
            return VK_OBJECT_TYPE_SWAPCHAIN_KHR;
        else if constexpr(std::is_same_v<Type, VkPhysicalDevice>)
            return VK_OBJECT_TYPE_PHYSICAL_DEVICE;
        else if constexpr(std::is_same_v<Type, VkQueue>)
            return VK_OBJECT_TYPE_QUEUE;
        else if constexpr(std::is_same_v<Type, VkShaderEXT>)
            return VK_OBJECT_TYPE_SHADER_EXT;
        else if constexpr(std::is_same_v<Type, VkAccelerationStructureKHR>)
            return VK_OBJECT_TYPE_ACCELERATION_STRUCTURE_KHR;
        else
        {
            static_assert(!std::is_same_v<Type, Type>, "Unsupported Vulkan object type");
            return VK_OBJECT_TYPE_UNKNOWN;
        }
    }

    // [TODO]: 
    // constexpr vk::BufferUsageFlags ConvertToVkBufferUsage(const EBufferUsageBits bufferUsageBits, const uint32_t structureStride, const bool isDeviceAddressSupported)
    // {
    //     vk::BufferUsageFlags flags = vk::BufferUsageFlagBits::eTransferSrc | vk::BufferUsageFlagBits::eTransferDst; // TODO: ban "the opposite" for UPLOAD/READBACK?
    //
    //     if (isDeviceAddressSupported)
    //         flags |= vk::BufferUsageFlagBits::eShaderDeviceAddress;
    //
    //     if (bufferUsageBits & EBufferUsageBits::VertexBuffer)
    //         flags |= vk::BufferUsageFlagBits::eVertexBuffer;
    //
    //     if (bufferUsageBits & EBufferUsageBits::IndexBuffer)
    //         flags |= vk::BufferUsageFlagBits::eIndexBuffer;
    //
    //     if (bufferUsageBits & EBufferUsageBits::ConstantBuffer)
    //         flags |= vk::BufferUsageFlagBits::eUniformBuffer;
    //
    //     if (bufferUsageBits & EBufferUsageBits::ArgumentBuffer)
    //         flags |= vk::BufferUsageFlagBits::eIndirectBuffer;
    //
    //     if (bufferUsageBits & EBufferUsageBits::ScratchBuffer)
    //         flags |= vk::BufferUsageFlagBits::eStorageBuffer;
    //
    //     if (bufferUsageBits & EBufferUsageBits::ShaderBindingTable)
    //         flags |= vk::BufferUsageFlagBits::eShaderBindingTableKHR;
    //
    //     if (bufferUsageBits & EBufferUsageBits::AccelerationStructureStorage)
    //         flags |= vk::BufferUsageFlagBits::eAccelerationStructureStorageKHR;
    //
    //     if (bufferUsageBits & EBufferUsageBits::AccelerationStructureBuildInput)
    //         flags |= vk::BufferUsageFlagBits::eAccelerationStructureBuildInputReadOnlyKHR;
    //
    //     if (bufferUsageBits & EBufferUsageBits::MicromapStorage)
    //         flags |= vk::BufferUsageFlagBits::eMicromapStorageEXT;
    //
    //     if (bufferUsageBits & EBufferUsageBits::MicromapBuildInput)
    //         flags |= vk::BufferUsageFlagBits::eMicromapBuildInputReadOnlyEXT;
    //
    //     if (bufferUsageBits & EBufferUsageBits::ShaderResource)
    //         flags |= structureStride ? vk::BufferUsageFlagBits::eStorageBuffer : vk::BufferUsageFlagBits::eUniformTexelBuffer;
    //
    //     if (bufferUsageBits & EBufferUsageBits::ShaderResourceStorage)
    //         flags |= structureStride ? vk::BufferUsageFlagBits::eStorageBuffer : vk::BufferUsageFlagBits::eStorageTexelBuffer;
    //
    //     return flags;
    // }
    //
    // constexpr vk::ImageUsageFlags ConvertToVkImageUsage(const ETextureUsageBits textureUsageBits)
    // {
    //     vk::ImageUsageFlags flags = vk::ImageUsageFlagBits::eTransferSrc | vk::ImageUsageFlagBits::eTransferDst;
    //
    //     if (textureUsageBits & ETextureUsageBits::ShaderResource)
    //         flags |= vk::ImageUsageFlagBits::eSampled;
    //
    //     if (textureUsageBits & ETextureUsageBits::ShaderResourceStorage)
    //         flags |= vk::ImageUsageFlagBits::eStorage;
    //
    //     if (textureUsageBits & ETextureUsageBits::ColorAttachment)
    //         flags |= vk::ImageUsageFlagBits::eColorAttachment;
    //
    //     if (textureUsageBits & ETextureUsageBits::DepthStencilAttachment)
    //         flags |= vk::ImageUsageFlagBits::eDepthStencilAttachment;
    //
    //     if (textureUsageBits & ETextureUsageBits::ShadingRateAttachment)
    //         flags |= vk::ImageUsageFlagBits::eFragmentShadingRateAttachmentKHR;
    //
    //     return flags;
    // }
}

