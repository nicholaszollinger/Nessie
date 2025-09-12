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
    class Texture : public AssetBase
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
        const ImageDesc&    GetDesc() const;

        //----------------------------------------------------------------------------------------------------
        /// @brief : Get the extent of the image.
        //----------------------------------------------------------------------------------------------------
        UInt3               GetExtent() const;

        //----------------------------------------------------------------------------------------------------
        /// @brief : Get the device image asset for this texture. 
        //----------------------------------------------------------------------------------------------------
        DeviceImage&        GetDeviceImage() { return m_image; }

        //----------------------------------------------------------------------------------------------------
        /// @brief : Get the device image asset for this texture (const version). 
        //----------------------------------------------------------------------------------------------------
        const DeviceImage&  GetDeviceImage() const { return m_image; }

    private:
        //----------------------------------------------------------------------------------------------------
        /// @brief : Load the Texture from a file.
        //----------------------------------------------------------------------------------------------------
        virtual ELoadResult LoadFromFile(const std::filesystem::path& path) override;

    protected:
        DeviceImage         m_image = nullptr;         // Device Image Asset.
        Buffer              m_imageData{};             // Raw image data.
    };

    static_assert(ValidAssetType<Texture>);

    //----------------------------------------------------------------------------------------------------
    // [TODO]: Consider making this a Texture Array over a Cube.
    //		
    /// @brief : A Texture Cube is a group of 6 images that can be used for graphical applications like a
    ///     skybox.
    //----------------------------------------------------------------------------------------------------
    class TextureCube final : public Texture
    {
        NES_DEFINE_TYPE_INFO(TextureCube)
        
        //----------------------------------------------------------------------------------------------------
        /// @brief : Load the Texture Cube from a file.
        //----------------------------------------------------------------------------------------------------
        virtual ELoadResult LoadFromFile(const std::filesystem::path& path) override;

        //----------------------------------------------------------------------------------------------------
        /// @brief : Load the Skybox from the YAML file.
        //----------------------------------------------------------------------------------------------------
        ELoadResult         LoadFromYAML(const YAML::Node& node);
    };

    static_assert(ValidAssetType<TextureCube>);
}

    