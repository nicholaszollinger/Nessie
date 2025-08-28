// GraphicsHelpers.h
#pragma once

namespace nes
{
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
    
    constexpr EDescriptorType GetDescriptorType(const EImage1DViewType type)
    {
        switch (type)
        {
            case EImage1DViewType::ShaderResourceStorage1D:
            case EImage1DViewType::ShaderResourceStorage1DArray:
                return EDescriptorType::StorageTexture;
                
            default: return EDescriptorType::Texture;
        }
    }

    constexpr EDescriptorType GetDescriptorType(const EImage2DViewType type)
    {
        switch (type)
        {
            case EImage2DViewType::ShaderResourceStorage2D:
            case EImage2DViewType::ShaderResourceStorage2DArray:
                return EDescriptorType::StorageTexture;
                
            default: return EDescriptorType::Texture;
        }
    }

    constexpr EDescriptorType GetDescriptorType(const EImage3DViewType type)
    {
        switch (type)
        {
            case EImage3DViewType::ShaderResourceStorage3D:
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