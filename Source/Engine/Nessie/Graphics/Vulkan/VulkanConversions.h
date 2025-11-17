// VulkanConversions.h
#pragma once
#include "Nessie/Graphics/GraphicsCommon.h"

namespace nes
{
    //----------------------------------------------------------------------------------------------------
    /// @brief : Convert to EVendor from a raw vendor id value.
    //----------------------------------------------------------------------------------------------------
    constexpr EVendor GetVendorFromID(const uint32 vendorID)
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

    //----------------------------------------------------------------------------------------------------
    /// @brief : Convert from a vk::PhysicalDeviceType to nes::EPhysicalDeviceType.
    //----------------------------------------------------------------------------------------------------
    constexpr EPhysicalDeviceType GetPhysicalDeviceTypeFromVulkanType(const vk::PhysicalDeviceType type)
    {
        switch (type)
        {
            case vk::PhysicalDeviceType::eDiscreteGpu: return EPhysicalDeviceType::DiscreteGPU;
            case vk::PhysicalDeviceType::eIntegratedGpu: return EPhysicalDeviceType::Integrated;
            case vk::PhysicalDeviceType::eVirtualGpu: return EPhysicalDeviceType::VirtualGPU;
            case vk::PhysicalDeviceType::eCpu: return EPhysicalDeviceType::CPU;
            
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

    namespace vulkan
    {
        constexpr std::array<VkFilter, static_cast<size_t>(EFilterType::MaxNum)> kFilters =
        {
            VK_FILTER_NEAREST,
            VK_FILTER_LINEAR,
        };

        constexpr std::array<VkImageType, static_cast<size_t>(EImageType::MaxNum)> kImageTypes =
        {
            VK_IMAGE_TYPE_1D,
            VK_IMAGE_TYPE_2D,
            VK_IMAGE_TYPE_3D,
        };

        constexpr std::array<VkSamplerMipmapMode, static_cast<size_t>(EFilterType::MaxNum)> kMipmapModes =
        {
            VK_SAMPLER_MIPMAP_MODE_NEAREST, // Nearest
            VK_SAMPLER_MIPMAP_MODE_LINEAR,  // Linear
        };

        constexpr std::array<VkBlendFactor, static_cast<size_t>(EBlendFactor::MaxNum)> kBlendFactors =
        {
            VK_BLEND_FACTOR_ZERO,                     // ZERO
            VK_BLEND_FACTOR_ONE,                      // ONE
            VK_BLEND_FACTOR_SRC_COLOR,                // SRC_COLOR
            VK_BLEND_FACTOR_ONE_MINUS_SRC_COLOR,      // ONE_MINUS_SRC_COLOR
            VK_BLEND_FACTOR_DST_COLOR,                // DST_COLOR
            VK_BLEND_FACTOR_ONE_MINUS_DST_COLOR,      // ONE_MINUS_DST_COLOR
            VK_BLEND_FACTOR_SRC_ALPHA,                // SRC_ALPHA
            VK_BLEND_FACTOR_ONE_MINUS_SRC_ALPHA,      // ONE_MINUS_SRC_ALPHA
            VK_BLEND_FACTOR_DST_ALPHA,                // DST_ALPHA
            VK_BLEND_FACTOR_ONE_MINUS_DST_ALPHA,      // ONE_MINUS_DST_ALPHA
            VK_BLEND_FACTOR_CONSTANT_COLOR,           // CONSTANT_COLOR
            VK_BLEND_FACTOR_ONE_MINUS_CONSTANT_COLOR, // ONE_MINUS_CONSTANT_COLOR
            VK_BLEND_FACTOR_CONSTANT_ALPHA,           // CONSTANT_ALPHA
            VK_BLEND_FACTOR_ONE_MINUS_CONSTANT_ALPHA, // ONE_MINUS_CONSTANT_ALPHA
            VK_BLEND_FACTOR_SRC_ALPHA_SATURATE,       // SRC_ALPHA_SATURATE
            VK_BLEND_FACTOR_SRC1_COLOR,               // SRC1_COLOR
            VK_BLEND_FACTOR_ONE_MINUS_SRC1_COLOR,     // ONE_MINUS_SRC1_COLOR
            VK_BLEND_FACTOR_SRC1_ALPHA,               // SRC1_ALPHA
            VK_BLEND_FACTOR_ONE_MINUS_SRC1_ALPHA,     // ONE_MINUS_SRC1_ALPHA
        };

        constexpr std::array<VkCompareOp, static_cast<size_t>(ECompareOp::MaxNum)> kCompareOps =
        {
            VK_COMPARE_OP_NEVER,            // NONE
            VK_COMPARE_OP_ALWAYS,           // ALWAYS
            VK_COMPARE_OP_NEVER,            // NEVER
            VK_COMPARE_OP_EQUAL,            // EQUAL
            VK_COMPARE_OP_NOT_EQUAL,        // NOT_EQUAL
            VK_COMPARE_OP_LESS,             // LESS
            VK_COMPARE_OP_LESS_OR_EQUAL,    // LESS_EQUAL
            VK_COMPARE_OP_GREATER,          // GREATER
            VK_COMPARE_OP_GREATER_OR_EQUAL, // GREATER_EQUAL
        };
        
        constexpr std::array<EFormat, 131> kFormatTable =
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
        constexpr std::array<VkFormat, static_cast<size_t>(EFormat::MaxNum)> kFormats =
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

        constexpr std::array<VkSamplerReductionMode, static_cast<size_t>(EReductionMode::MaxNum)> kSamplerReductionModes =
        {
            VK_SAMPLER_REDUCTION_MODE_WEIGHTED_AVERAGE, // NONE
            VK_SAMPLER_REDUCTION_MODE_MIN,              // MIN
            VK_SAMPLER_REDUCTION_MODE_MAX,              // MAX
        };

        constexpr std::array<vk::ImageUsageFlags, static_cast<size_t>(EImage1DViewType::MaxNum)> kImageViewUsage1D =
        {
            vk::ImageUsageFlagBits::eSampled,                  // SHADER_RESOURCE_1D,
            vk::ImageUsageFlagBits::eSampled,                  // SHADER_RESOURCE_1D_ARRAY,
            vk::ImageUsageFlagBits::eStorage,                  // SHADER_RESOURCE_STORAGE_1D,
            vk::ImageUsageFlagBits::eStorage,                  // SHADER_RESOURCE_STORAGE_1D_ARRAY,
            vk::ImageUsageFlagBits::eColorAttachment,         // COLOR_ATTACHMENT,
            vk::ImageUsageFlagBits::eDepthStencilAttachment, // DEPTH_STENCIL_ATTACHMENT
            vk::ImageUsageFlagBits::eDepthStencilAttachment, // DEPTH_READONLY_STENCIL_ATTACHMENT,
            vk::ImageUsageFlagBits::eDepthStencilAttachment, // DEPTH_ATTACHMENT_STENCIL_READONLY,
            vk::ImageUsageFlagBits::eDepthStencilAttachment, // DEPTH_STENCIL_READONLY,
        };

        constexpr std::array<vk::ImageUsageFlags, static_cast<size_t>(EImage2DViewType::MaxNum)> kImageViewUsage2D =
        {
            vk::ImageUsageFlagBits::eSampled,                              // SHADER_RESOURCE_2D,
            vk::ImageUsageFlagBits::eSampled,                              // SHADER_RESOURCE_2D_ARRAY,
            vk::ImageUsageFlagBits::eSampled,                              // SHADER_RESOURCE_CUBE,
            vk::ImageUsageFlagBits::eSampled,                              // SHADER_RESOURCE_CUBE_ARRAY,
            vk::ImageUsageFlagBits::eStorage,                              // SHADER_RESOURCE_STORAGE_2D,
            vk::ImageUsageFlagBits::eStorage,                              // SHADER_RESOURCE_STORAGE_2D_ARRAY,
            vk::ImageUsageFlagBits::eColorAttachment,                     // COLOR_ATTACHMENT,
            vk::ImageUsageFlagBits::eDepthStencilAttachment,             // DEPTH_STENCIL_ATTACHMENT
            vk::ImageUsageFlagBits::eDepthStencilAttachment,             // DEPTH_READONLY_STENCIL_ATTACHMENT,
            vk::ImageUsageFlagBits::eDepthStencilAttachment,             // DEPTH_ATTACHMENT_STENCIL_READONLY,
            vk::ImageUsageFlagBits::eDepthStencilAttachment,             // DEPTH_STENCIL_READONLY,
            vk::ImageUsageFlagBits::eFragmentShadingRateAttachmentKHR, // SHADING_RATE_ATTACHMENT
        };

        constexpr std::array<vk::ImageUsageFlags, static_cast<size_t>(EImage3DViewType::MaxNum)> kImageViewUsage3D =
        {
            vk::ImageUsageFlagBits::eSampled,          // SHADER_RESOURCE_3D,
            vk::ImageUsageFlagBits::eStorage,          // SHADER_RESOURCE_STORAGE_3D,
            vk::ImageUsageFlagBits::eColorAttachment,   // COLOR_ATTACHMENT
        };

        constexpr std::array<vk::ImageLayout, static_cast<size_t>(EImage1DViewType::MaxNum)> kImageViewLayout1D =
        {
            vk::ImageLayout::eShaderReadOnlyOptimal,                   // SHADER_RESOURCE_1D,
            vk::ImageLayout::eShaderReadOnlyOptimal,                   // SHADER_RESOURCE_1D_ARRAY,
            vk::ImageLayout::eGeneral,                                    // SHADER_RESOURCE_STORAGE_1D,
            vk::ImageLayout::eGeneral,                                    // SHADER_RESOURCE_STORAGE_1D_ARRAY,
            vk::ImageLayout::eColorAttachmentOptimal,                   // COLOR_ATTACHMENT,
            vk::ImageLayout::eDepthStencilAttachmentOptimal,           // DEPTH_STENCIL_ATTACHMENT
            vk::ImageLayout::eDepthReadOnlyStencilAttachmentOptimal, // DEPTH_READONLY_STENCIL_ATTACHMENT,
            vk::ImageLayout::eDepthAttachmentStencilReadOnlyOptimal, // DEPTH_ATTACHMENT_STENCIL_READONLY,
            vk::ImageLayout::eDepthStencilReadOnlyOptimal,            // DEPTH_STENCIL_READONLY,
        };

        constexpr std::array<vk::ImageLayout, static_cast<size_t>(EImage2DViewType::MaxNum)> kImageViewLayout2D =
        {
            vk::ImageLayout::eShaderReadOnlyOptimal,                   // SHADER_RESOURCE_2D,
            vk::ImageLayout::eShaderReadOnlyOptimal,                   // SHADER_RESOURCE_2D_ARRAY,
            vk::ImageLayout::eShaderReadOnlyOptimal,                   // SHADER_RESOURCE_CUBE,
            vk::ImageLayout::eShaderReadOnlyOptimal,                   // SHADER_RESOURCE_CUBE_ARRAY,
            vk::ImageLayout::eGeneral,                                 // SHADER_RESOURCE_STORAGE_2D,
            vk::ImageLayout::eGeneral,                                 // SHADER_RESOURCE_STORAGE_2D_ARRAY,
            vk::ImageLayout::eColorAttachmentOptimal,                  // COLOR_ATTACHMENT,
            vk::ImageLayout::eDepthStencilAttachmentOptimal,           // DEPTH_STENCIL_ATTACHMENT
            vk::ImageLayout::eDepthReadOnlyStencilAttachmentOptimal,   // DEPTH_READONLY_STENCIL_ATTACHMENT,
            vk::ImageLayout::eDepthAttachmentStencilReadOnlyOptimal,   // DEPTH_ATTACHMENT_STENCIL_READONLY,
            vk::ImageLayout::eDepthStencilReadOnlyOptimal,             // DEPTH_STENCIL_READONLY,
            vk::ImageLayout::eFragmentShadingRateAttachmentOptimalKHR, // SHADING_RATE_ATTACHMENT
        };

        constexpr std::array<vk::ImageLayout, static_cast<size_t>(EImage3DViewType::MaxNum)> kImageViewLayout3D =
        {
            vk::ImageLayout::eShaderReadOnlyOptimal,    // SHADER_RESOURCE_3D,
            vk::ImageLayout::eGeneral,                  // SHADER_RESOURCE_STORAGE_3D,
            vk::ImageLayout::eColorAttachmentOptimal,   // COLOR_ATTACHMENT
        };

        constexpr std::array<vk::StencilOp, static_cast<size_t>(EStencilOp::MaxNum)> kStencilOp =
        {
            vk::StencilOp::eKeep, 
            vk::StencilOp::eZero,
            vk::StencilOp::eReplace,
            vk::StencilOp::eIncrementAndClamp,
            vk::StencilOp::eDecrementAndClamp,
            vk::StencilOp::eInvert,
            vk::StencilOp::eIncrementAndWrap,
            vk::StencilOp::eDecrementAndWrap,
        };

        constexpr std::array<vk::ImageLayout, static_cast<size_t>(EImageLayout::MaxNum)> kImageLayouts =
        {
            vk::ImageLayout::eUndefined,                                // Undefined
            vk::ImageLayout::eGeneral,                                  // General
            vk::ImageLayout::ePresentSrcKHR,                            // Present
            vk::ImageLayout::eColorAttachmentOptimal,                   // ColorAttachment
            vk::ImageLayout::eFragmentShadingRateAttachmentOptimalKHR,  // ShadingRateAttachment
            vk::ImageLayout::eDepthStencilAttachmentOptimal,            // DepthStencilAttachment
            vk::ImageLayout::eDepthStencilReadOnlyOptimal,              // DepthStencilReadOnly
            vk::ImageLayout::eShaderReadOnlyOptimal,                    // ShaderResource
            vk::ImageLayout::eGeneral,                                  // ShaderResourceStorage
            vk::ImageLayout::eTransferSrcOptimal,                       // CopySource
            vk::ImageLayout::eTransferDstOptimal,                       // CopyDestination
            vk::ImageLayout::eTransferSrcOptimal,                       // ResolveSource
            vk::ImageLayout::eTransferDstOptimal,                       // ResolveDestination
        };
    }
    
    constexpr EFormat GetFormat(const uint32 vkFormat)
    {
        if (vkFormat < vulkan::kFormatTable.size())
            return vulkan::kFormatTable[vkFormat];
        else if (vkFormat == VK_FORMAT_A4R4G4B4_UNORM_PACK16)
            return EFormat::B4_G4_R4_A4_UNORM;

        return EFormat::Unknown;
    }

    constexpr vk::Format GetVkFormat(const EFormat format)
    {
        return static_cast<vk::Format>(vulkan::kFormats[static_cast<uint32>(format)]);
    }

    constexpr EQueryType GetQueryType(const uint32 queryTypeVk)
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

    constexpr vk::Filter GetVkFilterType(const EFilterType type)
    {
        return static_cast<vk::Filter>(vulkan::kFilters[static_cast<size_t>(type)]);
    }

    constexpr vk::ImageType GetVkImageType(const EImageType type)
    {
        return static_cast<vk::ImageType>(vulkan::kImageTypes[static_cast<size_t>(type)]);
    }

    constexpr vk::SamplerMipmapMode GetVkSamplerMipMode(const EFilterType type)
    {
        return static_cast<vk::SamplerMipmapMode>(vulkan::kMipmapModes[static_cast<size_t>(type)]);
    }

    constexpr vk::LogicOp GetVkLogicOp(const ELogicOp op)
    {
        switch (op)
        {
            case ELogicOp::Clear:           return vk::LogicOp::eClear;
            case ELogicOp::And:             return vk::LogicOp::eAnd;
            case ELogicOp::AndReverse:      return vk::LogicOp::eAndReverse;
            case ELogicOp::Copy:            return vk::LogicOp::eCopy;
            case ELogicOp::AndInverted:     return vk::LogicOp::eAndInverted;
            case ELogicOp::Xor:             return vk::LogicOp::eXor;
            case ELogicOp::Or:              return vk::LogicOp::eOr;
            case ELogicOp::Nor:             return vk::LogicOp::eNor;
            case ELogicOp::Equivalent:      return vk::LogicOp::eEquivalent;
            case ELogicOp::Invert:          return vk::LogicOp::eInvert;
            case ELogicOp::OrReverse:       return vk::LogicOp::eOrReverse;
            case ELogicOp::CopyInverted:    return vk::LogicOp::eCopyInverted;
            case ELogicOp::OrInverted:      return vk::LogicOp::eOrInverted;
            case ELogicOp::Nand:            return vk::LogicOp::eNand;
            case ELogicOp::Set:             return vk::LogicOp::eSet;

            default: return vk::LogicOp::eNoOp;
        }
    }

    constexpr vk::BlendOp GetVkBlendOp(const EBlendOp op)
    {
        switch (op)
        {
            case EBlendOp::Subtract:            return vk::BlendOp::eSubtract;
            case EBlendOp::ReverseSubtract:     return vk::BlendOp::eReverseSubtract;
            case EBlendOp::Min:                 return vk::BlendOp::eMin;
            case EBlendOp::Max:                 return vk::BlendOp::eMax;

            // Default Add:
            default: return vk::BlendOp::eAdd;
        }
    }

    constexpr vk::BlendFactor GetVkBlendFactor(const EBlendFactor blendFactor)
    {
        return static_cast<vk::BlendFactor>(vulkan::kBlendFactors[static_cast<size_t>(blendFactor)]);
    }

    constexpr vk::ColorComponentFlags GetVkColorComponentFlags(const EColorComponentBits colorMask)
    {
        return static_cast<vk::ColorComponentFlagBits>(colorMask & EColorComponentBits::RGBA);
    }

    constexpr vk::CompareOp GetVkCompareOp(const ECompareOp compareOp)
    {
        return static_cast<vk::CompareOp>(vulkan::kCompareOps[static_cast<size_t>(compareOp)]);
    }

    constexpr vk::SamplerReductionMode GetVkSamplerReductionMode(const EReductionMode reductionMode)
    {
        return static_cast<vk::SamplerReductionMode>(vulkan::kSamplerReductionModes[static_cast<size_t>(reductionMode)]);
    }

    constexpr vk::SamplerAddressMode GetVkSamplerAddressMode(const EAddressMode addressMode)
    {
        return static_cast<vk::SamplerAddressMode>(VK_SAMPLER_ADDRESS_MODE_REPEAT + static_cast<uint32>(addressMode));
    }

    constexpr vk::ImageViewType GetVkImageViewType(const EImage1DViewType type, const uint32 numLayers)
    {
        if (type == EImage1DViewType::ShaderResource1DArray || type == EImage1DViewType::ShaderResourceStorage1DArray)
            return vk::ImageViewType::e1DArray;

        return numLayers > 1 ? vk::ImageViewType::e1DArray : vk::ImageViewType::e1D;
    }

    constexpr vk::ImageViewType GetVkImageViewType(const EImage2DViewType type, const uint32 numLayers)
    {
        if (type == EImage2DViewType::ShaderResource2DArray || type == EImage2DViewType::ShaderResourceStorage2DArray)
            return vk::ImageViewType::e2DArray;

        if (type == EImage2DViewType::ShaderResourceCube || (type == EImage2DViewType::ShaderResource2D && numLayers == 6))
            return vk::ImageViewType::eCube;

        if (type == EImage2DViewType::ShaderResourceCubeArray)
            return vk::ImageViewType::eCubeArray;

        return numLayers > 1 ? vk::ImageViewType::e2DArray : vk::ImageViewType::e2D;
    }

    constexpr vk::ImageViewType GetVkImageViewType(const EImage3DViewType /*type*/, const uint32 /*numLayers*/)
    {
        return vk::ImageViewType::e3D;
    }

    constexpr vk::BufferUsageFlags GetVkBufferUsageFlags(const EBufferUsageBits usage, const uint32_t structureStride, const bool isDeviceAddressSupported)
    {
        vk::BufferUsageFlags flags = vk::BufferUsageFlagBits::eTransferSrc | vk::BufferUsageFlagBits::eTransferDst; // TODO: ban "the opposite" for Upload/Readback?

        if (isDeviceAddressSupported)
            flags |= vk::BufferUsageFlagBits::eShaderDeviceAddress;

        if (usage & EBufferUsageBits::VertexBuffer)
            flags |= vk::BufferUsageFlagBits::eVertexBuffer;

        if (usage & EBufferUsageBits::IndexBuffer)
            flags |= vk::BufferUsageFlagBits::eIndexBuffer;

        if (usage & EBufferUsageBits::UniformBuffer)
            flags |= vk::BufferUsageFlagBits::eUniformBuffer;

        if (usage & EBufferUsageBits::ArgumentBuffer)
            flags |= vk::BufferUsageFlagBits::eIndirectBuffer;

        if (usage & EBufferUsageBits::ScratchBuffer)
            flags |= vk::BufferUsageFlagBits::eStorageBuffer;

        if (usage & EBufferUsageBits::ShaderBindingTable)
            flags |= vk::BufferUsageFlagBits::eShaderBindingTableKHR;

        if (usage & EBufferUsageBits::AccelerationStructureStorage)
            flags |= vk::BufferUsageFlagBits::eAccelerationStructureStorageKHR;

        if (usage & EBufferUsageBits::AccelerationStructureBuildInput)
            flags |= vk::BufferUsageFlagBits::eAccelerationStructureBuildInputReadOnlyKHR;

        if (usage & EBufferUsageBits::MicromapStorage)
            flags |= vk::BufferUsageFlagBits::eMicromapStorageEXT;

        if (usage & EBufferUsageBits::MicromapBuildInput)
            flags |= vk::BufferUsageFlagBits::eMicromapBuildInputReadOnlyEXT;

        if (usage & EBufferUsageBits::ShaderResource)
            flags |= structureStride ? vk::BufferUsageFlagBits::eStorageBuffer : vk::BufferUsageFlagBits::eUniformTexelBuffer;

        if (usage & EBufferUsageBits::ShaderResourceStorage)
            flags |= structureStride ? vk::BufferUsageFlagBits::eStorageBuffer : vk::BufferUsageFlagBits::eStorageTexelBuffer;

        return flags;
    }

    constexpr vk::ImageUsageFlags GetVkImageUsageFlags(const EImageUsageBits usage)
    {
        vk::ImageUsageFlags flags = vk::ImageUsageFlagBits::eTransferSrc | vk::ImageUsageFlagBits::eTransferDst;
        
        if (usage & EImageUsageBits::ShaderResource)
            flags |= vk::ImageUsageFlagBits::eSampled;

        if (usage & EImageUsageBits::ShaderResourceStorage)
            flags |= vk::ImageUsageFlagBits::eStorage;

        if (usage & EImageUsageBits::ColorAttachment)
            flags |= vk::ImageUsageFlagBits::eColorAttachment;

        if (usage & EImageUsageBits::DepthStencilAttachment)
            flags |= vk::ImageUsageFlagBits::eDepthStencilAttachment;

        if (usage & EImageUsageBits::ShadingRateAttachment)
            flags |= vk::ImageUsageFlagBits::eFragmentShadingRateAttachmentKHR;

        return flags;
    }

    constexpr vk::ImageAspectFlags GetVkImageAspectFlags(const EFormat format)
    {
        switch (format)
        {
            case EFormat::D16_UNORM:
            case EFormat::D32_SFLOAT:
            case EFormat::R24_UNORM_X8:
            case EFormat::R32_SFLOAT_X8_X24:
                return vk::ImageAspectFlagBits::eDepth;

            case EFormat::D24_UNORM_S8_UINT:
            case EFormat::D32_SFLOAT_S8_UINT_X24:
                return vk::ImageAspectFlagBits::eDepth | vk::ImageAspectFlagBits::eStencil;

            case EFormat::X32_G8_UINT_X24:
            case EFormat::X24_G8_UINT:
                return vk::ImageAspectFlagBits::eStencil;

            default:
                return vk::ImageAspectFlagBits::eColor;
        }
    }

    constexpr vk::ImageAspectFlags GetVkImageAspectFlags(const EImagePlaneBits planes)
    {
        if (planes & EImagePlaneBits::All)
            return vk::ImageAspectFlagBits::eColor | vk::ImageAspectFlagBits::eDepth | vk::ImageAspectFlagBits::eStencil;
        
        vk::ImageAspectFlags aspectFlags{};
        if (planes & EImagePlaneBits::Color)
            aspectFlags |= vk::ImageAspectFlagBits::eColor;
        if (planes & EImagePlaneBits::Depth)
            aspectFlags |= vk::ImageAspectFlagBits::eDepth;
        if (planes & EImagePlaneBits::Stencil)
            aspectFlags |= vk::ImageAspectFlagBits::eStencil;

        return aspectFlags;
    }

    constexpr vk::ImageUsageFlags GetVkImageViewUsage(const EImage1DViewType type)
    {
        return vulkan::kImageViewUsage1D[static_cast<size_t>(type)];
    }

    constexpr vk::ImageUsageFlags GetVkImageViewUsage(const EImage2DViewType type)
    {
        return vulkan::kImageViewUsage2D[static_cast<size_t>(type)];
    }

    constexpr vk::ImageUsageFlags GetVkImageViewUsage(const EImage3DViewType type)
    {
        return vulkan::kImageViewUsage3D[static_cast<size_t>(type)];
    }

    constexpr vk::ImageLayout GetVkImageViewLayout(const EImage1DViewType type)
    {
        return vulkan::kImageViewLayout1D[static_cast<size_t>(type)];
    }

    constexpr vk::ImageLayout GetVkImageViewLayout(const EImage2DViewType type)
    {
        return vulkan::kImageViewLayout2D[static_cast<size_t>(type)];
    }

    constexpr vk::ImageLayout GetVkImageViewLayout(const EImage3DViewType type)
    {
        return vulkan::kImageViewLayout3D[static_cast<size_t>(type)];
    }

    constexpr vk::StencilOp GetVkStencilOp(const EStencilOp op)
    {
        return vulkan::kStencilOp[static_cast<size_t>(op)];
    }

    constexpr vk::CullModeFlagBits GetVkCullMode(const ECullMode mode)
    {
        switch (mode)
        {
            case ECullMode::None: return vk::CullModeFlagBits::eNone;
            case ECullMode::Front: return vk::CullModeFlagBits::eFront;
            case ECullMode::Back: return vk::CullModeFlagBits::eBack;
            case ECullMode::Both: return vk::CullModeFlagBits::eFrontAndBack;
        }

        return vk::CullModeFlagBits::eNone;
    }

    constexpr vk::PolygonMode GetVkPolygonMode(const EFillMode mode)
    {
        switch (mode)
        {
            case EFillMode::Solid: return vk::PolygonMode::eFill;
            case EFillMode::Wireframe: return vk::PolygonMode::eLine;
            case EFillMode::Point: return vk::PolygonMode::ePoint;
        }

        return vk::PolygonMode::eFill;
    }

    constexpr vk::FrontFace GetVkFrontFace(const EFrontFaceWinding winding)
    {
        switch (winding)
        {
            case EFrontFaceWinding::Clockwise: return vk::FrontFace::eClockwise;
            default: return vk::FrontFace::eCounterClockwise;
        }
    }

    constexpr vk::PrimitiveTopology GetVkTopology(const ETopology topology)
    {
        return static_cast<vk::PrimitiveTopology>(topology);
    }

    constexpr vk::IndexType GetVkIndexType(const EIndexType type)
    {
        if (type == EIndexType::U32)
            return vk::IndexType::eUint32;
            
        return vk::IndexType::eUint16;   
    }

    constexpr vk::PipelineStageFlags2 GetVkPipelineStageFlags(const EPipelineStageBits stages)
    {
        if (stages == EPipelineStageBits::All)
            return vk::PipelineStageFlagBits2::eAllCommands;

        if (stages == EPipelineStageBits::None)
            return vk::PipelineStageFlagBits2::eNone;

        vk::PipelineStageFlags2 flags = {};

        if (stages & EPipelineStageBits::TopOfPipe)
            flags |= vk::PipelineStageFlagBits2::eTopOfPipe;

        if (stages & EPipelineStageBits::BottomOfPipe)
            flags |= vk::PipelineStageFlagBits2::eBottomOfPipe;

        if (stages & EPipelineStageBits::IndexInput)
            flags |= vk::PipelineStageFlagBits2::eIndexInput;

        if (stages & EPipelineStageBits::VertexShader)
            flags |= vk::PipelineStageFlagBits2::eVertexShader;

        if (stages & EPipelineStageBits::TessControlShader)
            flags |= vk::PipelineStageFlagBits2::eTessellationControlShader;

        if (stages & EPipelineStageBits::TessEvaluationShader)
            flags |= vk::PipelineStageFlagBits2::eTessellationEvaluationShader;

        if (stages & EPipelineStageBits::GeometryShader)
            flags |= vk::PipelineStageFlagBits2::eGeometryShader;

        if (stages & EPipelineStageBits::MeshControlShader)
            flags |= vk::PipelineStageFlagBits2::eTaskShaderEXT;

        if (stages & EPipelineStageBits::MeshEvaluationShader)
            flags |= vk::PipelineStageFlagBits2::eMeshShaderEXT;

        if (stages & EPipelineStageBits::FragmentShader)
            flags |= vk::PipelineStageFlagBits2::eFragmentShader;

        if (stages & EPipelineStageBits::DepthStencilAttachment)
            flags |= vk::PipelineStageFlagBits2::eEarlyFragmentTests | vk::PipelineStageFlagBits2::eLateFragmentTests;

        if (stages & EPipelineStageBits::ColorAttachment)
            flags |= vk::PipelineStageFlagBits2::eColorAttachmentOutput;

        if (stages & EPipelineStageBits::ComputeShader)
            flags |= vk::PipelineStageFlagBits2::eComputeShader;

        if (stages & EPipelineStageBits::RayTracingShaders)
            flags |= vk::PipelineStageFlagBits2::eRayTracingShaderKHR;

        if (stages & EPipelineStageBits::Indirect)
            flags |= vk::PipelineStageFlagBits2::eDrawIndirect;

        if (stages & (EPipelineStageBits::Copy | EPipelineStageBits::ClearStorage | EPipelineStageBits::Resolve))
            flags |= vk::PipelineStageFlagBits2::eTransfer;

        if (stages & EPipelineStageBits::AccelerationStructure)
            flags |= vk::PipelineStageFlagBits2::eAccelerationStructureBuildKHR;

        if (stages & EPipelineStageBits::MicroMap)
            flags |= vk::PipelineStageFlagBits2::eMicromapBuildEXT;

        if (stages & EPipelineStageBits::TopOfPipe)
            flags |= vk::PipelineStageFlagBits2::eTopOfPipe;

        if (stages & EPipelineStageBits::BottomOfPipe)
            flags |= vk::PipelineStageFlagBits2::eBottomOfPipe;
        
        return flags;
    }

    constexpr vk::ShaderStageFlags GetVkShaderStageFlags(const EPipelineStageBits stage)
    {
        if (stage == EPipelineStageBits::All)
            return vk::ShaderStageFlagBits::eAll;

        vk::ShaderStageFlags flags{};

        if (stage & EPipelineStageBits::VertexShader)
            flags |= vk::ShaderStageFlagBits::eVertex;

        if (stage & EPipelineStageBits::TessControlShader)
            flags |= vk::ShaderStageFlagBits::eTessellationControl;

        if (stage & EPipelineStageBits::TessEvaluationShader)
            flags |= vk::ShaderStageFlagBits::eTessellationEvaluation;

        if (stage & EPipelineStageBits::GeometryShader)
            flags |= vk::ShaderStageFlagBits::eGeometry;

        if (stage & EPipelineStageBits::FragmentShader)
            flags |= vk::ShaderStageFlagBits::eFragment;

        if (stage & EPipelineStageBits::ComputeShader)
            flags |= vk::ShaderStageFlagBits::eCompute;

        if (stage & EPipelineStageBits::RayGenShader)
            flags |= vk::ShaderStageFlagBits::eRaygenKHR;

        if (stage & EPipelineStageBits::MissShader)
            flags |= vk::ShaderStageFlagBits::eMissKHR;

        if (stage & EPipelineStageBits::IntersectionShader)
            flags |= vk::ShaderStageFlagBits::eIntersectionKHR;

        if (stage & EPipelineStageBits::ClosestHitShader)
            flags |= vk::ShaderStageFlagBits::eClosestHitKHR;

        if (stage & EPipelineStageBits::AnyHitShader)
            flags |= vk::ShaderStageFlagBits::eAnyHitKHR;

        if (stage & EPipelineStageBits::CallableShader)
            flags |= vk::ShaderStageFlagBits::eCallableKHR;

        if (stage & EPipelineStageBits::MeshControlShader)
            flags |= vk::ShaderStageFlagBits::eTaskEXT;

        if (stage & EPipelineStageBits::MeshEvaluationShader)
            flags |= vk::ShaderStageFlagBits::eMeshEXT;
        
        return flags;
    }

    constexpr vk::ShaderStageFlagBits GetVkShaderStageFlagBits(const EPipelineStageBits stage)
    {
        if (stage & EPipelineStageBits::VertexShader)
            return vk::ShaderStageFlagBits::eVertex;

        if (stage & EPipelineStageBits::TessControlShader)
            return vk::ShaderStageFlagBits::eTessellationControl;

        if (stage & EPipelineStageBits::TessEvaluationShader)
            return vk::ShaderStageFlagBits::eTessellationEvaluation;

        if (stage & EPipelineStageBits::GeometryShader)
            return vk::ShaderStageFlagBits::eGeometry;

        if (stage & EPipelineStageBits::FragmentShader)
            return vk::ShaderStageFlagBits::eFragment;

        if (stage & EPipelineStageBits::ComputeShader)
            return vk::ShaderStageFlagBits::eCompute;

        if (stage & EPipelineStageBits::RayGenShader)
            return vk::ShaderStageFlagBits::eRaygenKHR;

        if (stage & EPipelineStageBits::MissShader)
            return vk::ShaderStageFlagBits::eMissKHR;

        if (stage & EPipelineStageBits::IntersectionShader)
            return vk::ShaderStageFlagBits::eIntersectionKHR;

        if (stage & EPipelineStageBits::ClosestHitShader)
            return vk::ShaderStageFlagBits::eClosestHitKHR;

        if (stage & EPipelineStageBits::AnyHitShader)
            return vk::ShaderStageFlagBits::eAnyHitKHR;

        if (stage & EPipelineStageBits::CallableShader)
            return vk::ShaderStageFlagBits::eCallableKHR;

        if (stage & EPipelineStageBits::MeshControlShader)
            return vk::ShaderStageFlagBits::eTaskEXT;

        if (stage & EPipelineStageBits::MeshEvaluationShader)
            return vk::ShaderStageFlagBits::eMeshEXT;

        NES_ASSERT(false, "Failed to find specific Shader stage!");
        return vk::ShaderStageFlagBits::eVertex;
    }

    constexpr vk::AccessFlags2 GetVkAccessFlags(const EAccessBits access)
    {
        vk::AccessFlags2 flags = vk::AccessFlagBits2::eNone;
        
        if (access & EAccessBits::None)
            return flags;

        if (access & EAccessBits::IndexBuffer)
            flags |= vk::AccessFlagBits2::eIndexRead;

        if (access & EAccessBits::VertexBuffer)
            flags |= vk::AccessFlagBits2::eVertexAttributeRead;

        if (access & EAccessBits::UniformBuffer)
            flags |= vk::AccessFlagBits2::eUniformRead;

        if (access & EAccessBits::ArgumentBuffer)
            flags |= vk::AccessFlagBits2::eIndirectCommandRead;

        if (access & EAccessBits::ScratchBuffer)
            flags |= vk::AccessFlagBits2::eAccelerationStructureReadKHR | vk::AccessFlagBits2::eAccelerationStructureWriteKHR;

        if (access & EAccessBits::ColorAttachment)
            flags |= vk::AccessFlagBits2::eColorAttachmentRead | vk::AccessFlagBits2::eColorAttachmentWrite;

        if (access & EAccessBits::ShadingRateAttachment)
            flags |= vk::AccessFlagBits2::eFragmentShadingRateAttachmentReadKHR;

        if (access & EAccessBits::DepthStencilAttachmentRead)
            flags |= vk::AccessFlagBits2::eDepthStencilAttachmentRead;

        if (access & EAccessBits::DepthStencilAttachmentWrite)
            flags |= vk::AccessFlagBits2::eDepthStencilAttachmentWrite;

        if (access & EAccessBits::AccelerationStructureRead)
            flags |= vk::AccessFlagBits2::eAccelerationStructureReadKHR;

        if (access & EAccessBits::AccelerationStructureWrite)
            flags |= vk::AccessFlagBits2::eAccelerationStructureWriteKHR;

        if (access & EAccessBits::MicromapRead)
            flags |= vk::AccessFlagBits2::eMicromapReadEXT;

        if (access & EAccessBits::MicromapWrite)
            flags |= vk::AccessFlagBits2::eMicromapWriteEXT;

        if (access & EAccessBits::ShaderBindingTable)
            flags |= vk::AccessFlagBits2::eShaderBindingTableReadKHR;

        if (access & EAccessBits::ShaderResourceRead)
            flags |= vk::AccessFlagBits2::eShaderRead;

        if (access & EAccessBits::ShaderResourceStorage)
            flags |= vk::AccessFlagBits2::eShaderRead | vk::AccessFlagBits2::eShaderWrite;

        if (access & (EAccessBits::CopySource | EAccessBits::ResolveSource))
            flags |= vk::AccessFlagBits2::eTransferRead;

        if (access & (EAccessBits::CopyDestination | EAccessBits::ResolveDestination))
            flags |= vk::AccessFlagBits2::eTransferWrite;
        
        return flags;
    }

    constexpr vk::DescriptorType GetVkDescriptorType(const EDescriptorType type)
    {
        switch (type)
        {
            case EDescriptorType::Image:                    return vk::DescriptorType::eSampledImage;
            case EDescriptorType::StorageImage:             return vk::DescriptorType::eStorageImage;
            case EDescriptorType::Buffer:                   return vk::DescriptorType::eUniformTexelBuffer;
            case EDescriptorType::StorageBuffer:            return vk::DescriptorType::eStorageBuffer;
            case EDescriptorType::UniformBuffer:           return vk::DescriptorType::eUniformBuffer;
            case EDescriptorType::Sampler:                  return vk::DescriptorType::eSampler;
            case EDescriptorType::AccelerationStructure:    return vk::DescriptorType::eAccelerationStructureKHR;

            case EDescriptorType::None:
                break;
        }

        NES_ASSERT(false, "Unknown descriptor type!");
        return vk::DescriptorType{};
    }

    constexpr vk::ImageLayout GetVkImageLayout(const EImageLayout type)
    {
        return vulkan::kImageLayouts[static_cast<size_t>(type)];
    }
    
    vk::ImageMemoryBarrier2 CreateVkImageMemoryBarrier(const ImageBarrierDesc& desc);

    constexpr uint32 GetMaxSampleCount(const vk::SampleCountFlags sampleCount)
    {
        if ((sampleCount & vk::SampleCountFlagBits::e64) == vk::SampleCountFlagBits::e64)
            return 64;
        if ((sampleCount & vk::SampleCountFlagBits::e32) == vk::SampleCountFlagBits::e32)
            return 32;
        if ((sampleCount & vk::SampleCountFlagBits::e16) == vk::SampleCountFlagBits::e16)
            return 16;
        if ((sampleCount & vk::SampleCountFlagBits::e8) == vk::SampleCountFlagBits::e8)
            return 8;
        if ((sampleCount & vk::SampleCountFlagBits::e4) == vk::SampleCountFlagBits::e4)
            return 4;
        if ((sampleCount & vk::SampleCountFlagBits::e2) == vk::SampleCountFlagBits::e2)
            return 2;
        
        return 1;
    }

    constexpr vk::SampleCountFlagBits GetVkSampleCountFlags(const uint32 numSamples)
    {
        if (numSamples == 64)
            return vk::SampleCountFlagBits::e64;
        else if (numSamples == 32)
            return vk::SampleCountFlagBits::e32;
        else if (numSamples == 16)
            return vk::SampleCountFlagBits::e16;
        else if (numSamples == 8)
            return vk::SampleCountFlagBits::e8;
        else if (numSamples == 4)
            return vk::SampleCountFlagBits::e4;
        else if (numSamples == 2)
            return vk::SampleCountFlagBits::e2;

        NES_ASSERT(numSamples == 1, "Invalid Sample Count!");
        return vk::SampleCountFlagBits::e1;
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

