// Texture.h
#pragma once
#include "DeviceImage.h"
#include "GraphicsCommon.h"
#include "Nessie/Asset/AssetBase.h"
#include "Nessie/Core/Memory/Buffer.h"

namespace nes
{
    //----------------------------------------------------------------------------------------------------
    /// @brief : Represents a 2D image asset.
    //----------------------------------------------------------------------------------------------------
    class Texture final : public AssetBase
    {
        NES_DEFINE_TYPE_INFO(Texture)
        
    public:
        virtual             ~Texture() override;

        //----------------------------------------------------------------------------------------------------
        /// @brief : Set a debug name for the device image. 
        //----------------------------------------------------------------------------------------------------
        void                SetDeviceDebugName(const char* name);
        
        //----------------------------------------------------------------------------------------------------
        /// @brief : Get the texture's properties.
        //----------------------------------------------------------------------------------------------------
        const TextureDesc&  GetDesc() const;

        //----------------------------------------------------------------------------------------------------
        /// @brief : Get the extent of the image.
        //----------------------------------------------------------------------------------------------------
        UInt3               GetExtent() const;

        //----------------------------------------------------------------------------------------------------
        /// @brief : Get the device image asset for this texture. 
        //----------------------------------------------------------------------------------------------------
        DeviceImage*        GetDeviceImage() { return m_pImage; }

        //----------------------------------------------------------------------------------------------------
        /// @brief : Get the device image asset for this texture (const version). 
        //----------------------------------------------------------------------------------------------------
        const DeviceImage*  GetDeviceImage() const { return m_pImage; }

    private:
        //----------------------------------------------------------------------------------------------------
        /// @brief : Load the Texture from a file.
        //----------------------------------------------------------------------------------------------------
        virtual ELoadResult LoadFromFile(const std::filesystem::path& path) override;

    private:
        DeviceImage*        m_pImage = nullptr;         // Device Image Asset.
        Buffer              m_imageData{};              // Raw image data.
    };
}

    