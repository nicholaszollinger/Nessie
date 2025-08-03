// GraphicsHelpers.h
#pragma once

namespace nes
{
    #define _ 0
    #define X 1

    namespace graphics
    {
        constexpr std::array<FormatProps, static_cast<size_t>(EFormat::MaxNum)> kFormatProps =
        {{
     
            {"UNKNOWN",                 EFormat::Unknown,                   0,  0,  0,  0,  1,  0, 0, _, _, _, _, _, _, _, _, _, _, _}, // UNKNOWN
            {"R8_UNORM",                EFormat::R8_UNORM,                  8,  0,  0,  0,  1,  1, 1, _, _, _, _, _, _, _, X, _, _, _}, // R8_UNORM
            {"R8_SNORM",                EFormat::R8_SNORM,                  8,  0,  0,  0,  1,  1, 1, _, _, _, _, _, _, _, X, X, _, _}, // R8_SNORM
            {"R8_UINT",                 EFormat::R8_UINT,                   8,  0,  0,  0,  1,  1, 1, _, _, _, _, _, _, X, _, _, _, _}, // R8_UINT
            {"R8_SINT",                 EFormat::R8_SINT,                   8,  0,  0,  0,  1,  1, 1, _, _, _, _, _, _, X, _, X, _, _}, // R8_SINT
            {"RG8_UNORM",               EFormat::RG8_UNORM,                 8,  8,  0,  0,  2,  1, 1, _, _, _, _, _, _, _, X, _, _, _}, // RG8_UNORM
            {"RG8_SNORM",               EFormat::RG8_SNORM,                 8,  8,  0,  0,  2,  1, 1, _, _, _, _, _, _, _, X, X, _, _}, // RG8_SNORM
            {"RG8_UINT",                EFormat::RG8_UINT,                  8,  8,  0,  0,  2,  1, 1, _, _, _, _, _, _, X, _, _, _, _}, // RG8_UINT
            {"RG8_SINT",                EFormat::RG8_SINT,                  8,  8,  0,  0,  2,  1, 1, _, _, _, _, _, _, X, _, X, _, _}, // RG8_SINT
            {"BGRA8_UNORM",             EFormat::BGRA8_UNORM,               8,  8,  8,  8,  4,  1, 1, X, _, _, _, _, _, _, X, _, _, _}, // BGRA8_UNORM
            {"BGRA8_SRGB",              EFormat::BGRA8_SRGB,                8,  8,  8,  8,  4,  1, 1, X, _, _, _, _, _, _, _, _, X, _}, // BGRA8_SRGB
            {"RGBA8_UNORM",             EFormat::RGBA8_UNORM,               8,  8,  8,  8,  4,  1, 1, _, _, _, _, _, _, _, X, _, _, _}, // RGBA8_UNORM
            {"RGBA8_SRGB",              EFormat::RGBA8_SRGB,                8,  8,  8,  8,  4,  1, 1, _, _, _, _, _, _, _, _, _, X, _}, // RGBA8_SRGB
            {"RGBA8_SNORM",             EFormat::RGBA8_SNORM,               8,  8,  8,  8,  4,  1, 1, _, _, _, _, _, _, _, X, X, _, _}, // RGBA8_SNORM
            {"RGBA8_UINT",              EFormat::RGBA8_UINT,                8,  8,  8,  8,  4,  1, 1, _, _, _, _, _, _, X, _, _, _, _}, // RGBA8_UINT
            {"RGBA8_SINT",              EFormat::RGBA8_SINT,                8,  8,  8,  8,  4,  1, 1, _, _, _, _, _, _, X, _, X, _, _}, // RGBA8_SINT
            {"R16_UNORM",               EFormat::R16_UNORM,                 16, 0,  0,  0,  2,  1, 1, _, _, _, _, _, _, _, X, _, _, _}, // R16_UNORM
            {"R16_SNORM",               EFormat::R16_SNORM,                 16, 0,  0,  0,  2,  1, 1, _, _, _, _, _, _, _, X, X, _, _}, // R16_SNORM
            {"R16_UINT",                EFormat::R16_UINT,                  16, 0,  0,  0,  2,  1, 1, _, _, _, _, _, _, X, _, _, _, _}, // R16_UINT
            {"R16_SINT",                EFormat::R16_SINT,                  16, 0,  0,  0,  2,  1, 1, _, _, _, _, _, _, X, _, X, _, _}, // R16_SINT
            {"R16_SFLOAT",              EFormat::R16_SFLOAT,                16, 0,  0,  0,  2,  1, 1, _, _, _, _, X, _, _, _, X, _, _}, // R16_SFLOAT
            {"RG16_UNORM",              EFormat::RG16_UNORM,                16, 16, 0,  0,  4,  1, 1, _, _, _, _, _, _, _, X, _, _, _}, // RG16_UNORM
            {"RG16_SNORM",              EFormat::RG16_SNORM,                16, 16, 0,  0,  4,  1, 1, _, _, _, _, _, _, _, X, X, _, _}, // RG16_SNORM
            {"RG16_UINT",               EFormat::RG16_UINT,                 16, 16, 0,  0,  4,  1, 1, _, _, _, _, _, _, X, _, _, _, _}, // RG16_UINT
            {"RG16_SINT",               EFormat::RG16_SINT,                 16, 16, 0,  0,  4,  1, 1, _, _, _, _, _, _, X, _, X, _, _}, // RG16_SINT
            {"RG16_SFLOAT",             EFormat::RG16_SFLOAT,               16, 16, 0,  0,  4,  1, 1, _, _, _, _, X, _, _, _, X, _, _}, // RG16_SFLOAT
            {"RGBA16_UNORM",            EFormat::RGBA16_UNORM,              16, 16, 16, 16, 8,  1, 1, _, _, _, _, _, _, _, X, _, _, _}, // RGBA16_UNORM
            {"RGBA16_SNORM",            EFormat::RGBA16_SNORM,              16, 16, 16, 16, 8,  1, 1, _, _, _, _, _, _, _, X, X, _, _}, // RGBA16_SNORM
            {"RGBA16_UINT",             EFormat::RGBA16_UINT,               16, 16, 16, 16, 8,  1, 1, _, _, _, _, _, _, X, _, _, _, _}, // RGBA16_UINT
            {"RGBA16_SINT",             EFormat::RGBA16_SINT,               16, 16, 16, 16, 8,  1, 1, _, _, _, _, _, _, X, _, X, _, _}, // RGBA16_SINT
            {"RGBA16_SFLOAT",           EFormat::RGBA16_SFLOAT,             16, 16, 16, 16, 8,  1, 1, _, _, _, _, X, _, _, _, X, _, _}, // RGBA16_SFLOAT
            {"R32_UINT",                EFormat::R32_UINT,                  32, 32, 0,  0,  4,  1, 1, _, _, _, _, _, _, X, _, _, _, _}, // R32_UINT
            {"R32_SINT",                EFormat::R32_SINT,                  32, 32, 0,  0,  4,  1, 1, _, _, _, _, _, _, X, _, X, _, _}, // R32_SINT
            {"R32_SFLOAT",              EFormat::R32_SFLOAT,                32, 32, 0,  0,  4,  1, 1, _, _, _, _, X, _, _, _, X, _, _}, // R32_SFLOAT
            {"RG32_UINT",               EFormat::RG32_UINT,                 32, 32, 0,  0,  8,  1, 1, _, _, _, _, _, _, X, _, _, _, _}, // RG32_UINT
            {"RG32_SINT",               EFormat::RG32_SINT,                 32, 32, 0,  0,  8,  1, 1, _, _, _, _, _, _, X, _, X, _, _}, // RG32_SINT
            {"RG32_SFLOAT",             EFormat::RG32_SFLOAT,               32, 32, 0,  0,  8,  1, 1, _, _, _, _, X, _, _, _, X, _, _}, // RG32_SFLOAT
            {"RGB32_UINT",              EFormat::RGB32_UINT,                32, 32, 32, 0,  12, 1, 1, _, _, _, _, _, _, X, _, _, _, _}, // RGB32_UINT
            {"RGB32_SINT",              EFormat::RGB32_SINT,                32, 32, 32, 0,  12, 1, 1, _, _, _, _, _, _, X, _, X, _, _}, // RGB32_SINT
            {"RGB32_SFLOAT",            EFormat::RGB32_SFLOAT,              32, 32, 32, 0,  12, 1, 1, _, _, _, _, X, _, _, _, X, _, _}, // RGB32_SFLOAT
            {"RGBA32_UINT",             EFormat::RGBA32_UINT,               32, 32, 32, 32, 16, 1, 1, _, _, _, _, _, _, X, _, _, _, _}, // RGBA32_UINT
            {"RGBA32_SINT",             EFormat::RGBA32_SINT,               32, 32, 32, 32, 16, 1, 1, _, _, _, _, _, _, X, _, X, _, _}, // RGBA32_SINT
            {"RGBA32_SFLOAT",           EFormat::RGBA32_SFLOAT,             32, 32, 32, 32, 16, 1, 1, _, _, _, _, X, _, _, _, X, _, _}, // RGBA32_SFLOAT
            {"B5_G6_R5_UNORM",          EFormat::B5_G6_R5_UNORM,            5,  6,  5,  0,  2,  1, 1, X, _, _, _, _, X, _, X, _, _, _}, // B5_G6_R5_UNORM
            {"B5_G5_R5_A1_UNORM",       EFormat::B5_G5_R5_A1_UNORM,         5,  5,  5,  1,  2,  1, 1, X, _, _, _, _, X, _, X, _, _, _}, // B5_G5_R5_A1_UNORM
            {"B4_G4_R4_A4_UNORM",       EFormat::B4_G4_R4_A4_UNORM,         4,  4,  4,  4,  2,  1, 1, X, _, _, _, _, X, _, X, _, _, _}, // B4_G4_R4_A4_UNORM
            {"R10_G10_B10_A2_UNORM",    EFormat::R10_G10_B10_A2_UNORM,      10, 10, 10, 2,  4,  1, 1, _, _, _, _, _, X, _, X, _, _, _}, // R10_G10_B10_A2_UNORM
            {"R10_G10_B10_A2_UINT",     EFormat::R10_G10_B10_A2_UINT,       10, 10, 10, 2,  4,  1, 1, _, _, _, _, _, X, X, _, _, _, _}, // R10_G10_B10_A2_UINT
            {"R11_G11_B10_UFLOAT",      EFormat::R11_G11_B10_UFLOAT,        11, 11, 10, 0,  4,  1, 1, _, _, _, _, X, X, _, _, _, _, _}, // R11_G11_B10_UFLOAT
            {"R9_G9_B9_E5_UFLOAT",      EFormat::R9_G9_B9_E5_UFLOAT,        9,  9,  9,  5,  4,  1, 1, _, _, _, X, X, X, _, _, _, _, _}, // R9_G9_B9_E5_UFLOAT
            {"BC1_RGBA_UNORM",          EFormat::BC1_RGBA_UNORM,            5,  6,  5,  1,  8,  4, 4, _, X, _, _, _, _, _, X, _, _, _}, // BC1_RGBA_UNORM
            {"BC1_RGBA_SRGB",           EFormat::BC1_RGBA_SRGB,             5,  6,  5,  1,  8,  4, 4, _, X, _, _, _, _, _, _, _, X, _}, // BC1_RGBA_SRGB
            {"BC2_RGBA_UNORM",          EFormat::BC2_RGBA_UNORM,            5,  6,  5,  4,  16, 4, 4, _, X, _, _, _, _, _, X, _, _, _}, // BC2_RGBA_UNORM
            {"BC2_RGBA_SRGB",           EFormat::BC2_RGBA_SRGB,             5,  6,  5,  4,  16, 4, 4, _, X, _, _, _, _, _, _, _, X, _}, // BC2_RGBA_SRGB
            {"BC3_RGBA_UNORM",          EFormat::BC3_RGBA_UNORM,            5,  6,  5,  8,  16, 4, 4, _, X, _, _, _, _, _, X, _, _, _}, // BC3_RGBA_UNORM
            {"BC3_RGBA_SRGB",           EFormat::BC3_RGBA_SRGB,             5,  6,  5,  8,  16, 4, 4, _, X, _, _, _, _, _, _, _, X, _}, // BC3_RGBA_SRGB
            {"BC4_R_UNORM",             EFormat::BC4_R_UNORM,               8,  0,  0,  0,  8,  4, 4, _, X, _, _, _, _, _, X, _, _, _}, // BC4_R_UNORM
            {"BC4_R_SNORM",             EFormat::BC4_R_SNORM,               8,  0,  0,  0,  8,  4, 4, _, X, _, _, _, _, _, X, X, _, _}, // BC4_R_SNORM
            {"BC5_RG_UNORM",            EFormat::BC5_RG_UNORM,              8,  8,  0,  0,  16, 4, 4, _, X, _, _, _, _, _, X, _, _, _}, // BC5_RG_UNORM
            {"BC5_RG_SNORM",            EFormat::BC5_RG_SNORM,              8,  8,  0,  0,  16, 4, 4, _, X, _, _, _, _, _, X, X, _, _}, // BC5_RG_SNORM
            {"BC6H_RGB_UFLOAT",         EFormat::BC6H_RGB_UFLOAT,           16, 16, 16, 0,  16, 4, 4, _, X, _, _, X, _, _, _, _, _, _}, // BC6H_RGB_UFLOAT
            {"BC6H_RGB_SFLOAT",         EFormat::BC6H_RGB_SFLOAT,           16, 16, 16, 0,  16, 4, 4, _, X, _, _, X, _, _, _, X, _, _}, // BC6H_RGB_SFLOAT
            {"BC7_RGBA_UNORM",          EFormat::BC7_RGBA_UNORM,            8,  8,  8,  8,  16, 4, 4, _, X, _, _, _, _, _, X, _, _, _}, // BC7_RGBA_UNORM
            {"BC7_RGBA_SRGB",           EFormat::BC7_RGBA_SRGB,             8,  8,  8,  8,  16, 4, 4, _, X, _, _, _, _, _, _, _, X, _}, // BC7_RGBA_SRGB
            {"D16_UNORM",               EFormat::D16_UNORM,                 16, 0,  0,  0,  2,  1, 1, _, _, X, _, _, _, _, X, _, _, _}, // D16_UNORM
            {"D24_UNORM_S8_UINT",       EFormat::D24_UNORM_S8_UINT,         24, 8,  0,  0,  4,  1, 1, _, _, X, _, _, _, X, X, _, _, X}, // D24_UNORM_S8_UINT
            {"D32_SFLOAT",              EFormat::D32_SFLOAT,                32, 0,  0,  0,  4,  1, 1, _, _, X, _, X, _, _, _, X, _, _}, // D32_SFLOAT
            {"D32_SFLOAT_S8_UINT_X24",  EFormat::D32_SFLOAT_S8_UINT_X24,    32, 8,  0,  0,  8,  1, 1, _, _, X, _, X, _, X, _, X, _, X}, // D32_SFLOAT_S8_UINT_X24
            {"R24_UNORM_X8",            EFormat::R24_UNORM_X8,              24, 8,  0,  0,  4,  1, 1, _, _, X, _, _, _, _, X, _, _, _}, // R24_UNORM_X8
            {"X24_G8_UINT",             EFormat::X24_G8_UINT,               24, 8,  0,  0,  4,  1, 1, _, _, _, _, _, _, X, _, _, _, X}, // X24_G8_UINT
            {"R32_SFLOAT_X8_X24",       EFormat::R32_SFLOAT_X8_X24,         32, 8,  0,  0,  8,  1, 1, _, _, X, _, X, _, _, _, X, _, _}, // R32_SFLOAT_X8_X24
            {"X32_G8_UINT_X24",         EFormat::X32_G8_UINT_X24,           32, 8,  0,  0,  8,  1, 1, _, _, _, _, _, _, X, _, _, _, X}, // X32_G8_UINT_X24
        }};
    }

    #undef _
    #undef X

    constexpr const FormatProps& GetFormatProps(const EFormat format)
    {
        return graphics::kFormatProps[static_cast<size_t>(format)];
    }

    constexpr EDescriptorType GetDescriptorType(const ETexture1DViewType type)
    {
        switch (type)
        {
            case ETexture1DViewType::ShaderResourceStorage1D:
            case ETexture1DViewType::ShaderResourceStorage1DArray:
                return EDescriptorType::StorageTexture;
                
            default: return EDescriptorType::Texture;
        }
    }

    constexpr EDescriptorType GetDescriptorType(const ETexture2DViewType type)
    {
        switch (type)
        {
            case ETexture2DViewType::ShaderResourceStorage2D:
            case ETexture2DViewType::ShaderResourceStorage2DArray:
                return EDescriptorType::StorageTexture;
                
            default: return EDescriptorType::Texture;
        }
    }

    constexpr EDescriptorType GetDescriptorType(const ETexture3DViewType type)
    {
        switch (type)
        {
            case ETexture3DViewType::ShaderResourceStorage3D:
                return EDescriptorType::StorageTexture;
            
            default: return EDescriptorType::Texture;
        }
    }

    constexpr EDescriptorType GetDescriptorType(const EBufferViewType type)
    {
        switch (type)
        {
            case EBufferViewType::Constant:
                return EDescriptorType::ConstantBuffer;

            case EBufferViewType::ShaderResourceStorage:
                return EDescriptorType::StorageBuffer;
            
            default: return EDescriptorType::Buffer;
        }
    }

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

    //----------------------------------------------------------------------------------------------------
    /// @brief : Pack DeviceMemoryTypeInfo into the DeviceMemoryType. (It's a Reinterpret Cast).
    //----------------------------------------------------------------------------------------------------
    inline DeviceMemoryType Pack(const DeviceMemoryTypeInfo& memoryTypeInfo)
    {
        return *(reinterpret_cast<const DeviceMemoryType*>(&memoryTypeInfo));
    }

    //----------------------------------------------------------------------------------------------------
    /// @brief : Unpack DeviceMemoryType into the DeviceMemoryTypeInfo. (It's a Reinterpret Cast).
    //----------------------------------------------------------------------------------------------------
    inline DeviceMemoryTypeInfo Unpack(const DeviceMemoryType& memoryType)
    {
        return *(reinterpret_cast<const DeviceMemoryTypeInfo*>(&memoryType));
    }
}