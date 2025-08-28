// DeviceImage.h
#pragma once
#include "GraphicsCommon.h"
#include "DeviceAsset.h"

namespace nes
{
    //----------------------------------------------------------------------------------------------------
    /// @brief : A Device image is the device resource for a Texture.
    ///      It represents a multidimensional array of data (1D, 2D or 3D).
    //----------------------------------------------------------------------------------------------------
    class DeviceImage final : public DeviceAsset
    {
    public:
        explicit            DeviceImage(RenderDevice& device) : DeviceAsset(device) {}
        virtual             ~DeviceImage() override;
        
        //----------------------------------------------------------------------------------------------------
        /// @brief : Create a device image using an existing image. When this object is destroyed, the
        ///     image resource will not be destroyed. This is to be used for cases like the Swapchain.  
        //----------------------------------------------------------------------------------------------------
        EGraphicsResult     Init(const vk::Image image, const ImageDesc& imageDesc);
        
        //----------------------------------------------------------------------------------------------------
        /// @brief : Allocates a new image asset.
        //----------------------------------------------------------------------------------------------------
        EGraphicsResult     Init(const AllocateImageDesc& allocDesc);

        //----------------------------------------------------------------------------------------------------
        /// @brief : Set a debug name for this image. 
        //----------------------------------------------------------------------------------------------------
        virtual void        SetDebugName(const std::string& name) override;

        //----------------------------------------------------------------------------------------------------
        /// @brief : Get the image's properties.
        //----------------------------------------------------------------------------------------------------
        const ImageDesc&    GetDesc() const         { return m_desc; }

        //----------------------------------------------------------------------------------------------------
        /// @brief : Get the extent of the image.
        //----------------------------------------------------------------------------------------------------
        VkExtent3D          GetExtent() const       { return { m_desc.m_width, m_desc.m_height, m_desc.m_depth }; }

        //----------------------------------------------------------------------------------------------------
        /// @brief : Get the Vulkan Image object.
        //----------------------------------------------------------------------------------------------------
        vk::Image           GetVkImage() const       { return m_image; }

        //----------------------------------------------------------------------------------------------------
        /// @brief : Get the size of a particular dimension (width = 0, height = 1, depth = 2) for a given mip level.
        //----------------------------------------------------------------------------------------------------
        uint16              GetSize(const uint16 dimensionIndex, const uint16 mip = 0) const;
    
    private:
        vk::Image           m_image;
        ImageDesc           m_desc{};
        VmaAllocation       m_allocation = nullptr;     // Device Memory associated with the Texture.
        bool                m_ownsNativeObjects = true; // If true, then on destruction the image will be freed.
    };
}