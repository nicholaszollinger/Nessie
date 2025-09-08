// DeviceImage.h
#pragma once
#include "GraphicsCommon.h"
#include "DeviceObject.h"

namespace nes
{
    namespace graphics
    {
        //----------------------------------------------------------------------------------------------------
        /// @brief : Calculate the maximum number of mip levels for a 1D image. 
        //----------------------------------------------------------------------------------------------------
        inline uint32 CalculateMipLevelCount(const uint32 extent)
        {
            return static_cast<uint32>(std::floor(std::log2(extent))) + 1; 
        }

        //----------------------------------------------------------------------------------------------------
        /// @brief : Calculate the maximum number of mip levels for a 2D image. 
        //----------------------------------------------------------------------------------------------------
        inline uint32 CalculateMipLevelCount(const uint32 width, const uint32 height)
        {
            return static_cast<uint32>(std::floor(std::log2(math::Max(width, height)))) + 1; 
        }

        //----------------------------------------------------------------------------------------------------
        /// @brief : Calculate the maximum number of mip levels for a 3D image. 
        //----------------------------------------------------------------------------------------------------
        inline uint32 CalculateMipLevelCount(const uint32 width, const uint32 height, const uint32 depth)
        {
            return static_cast<uint32>(std::floor(std::log2(math::Max(width, height, depth)))) + 1; 
        }
    }
    
    //----------------------------------------------------------------------------------------------------
    /// @brief : A Device image is the device resource for a Texture.
    ///      It represents a multidimensional array of data (1D, 2D or 3D).
    //----------------------------------------------------------------------------------------------------
    class DeviceImage
    {
    public:
        DeviceImage(std::nullptr_t) {}
        DeviceImage(const DeviceImage&) = delete;
        DeviceImage(DeviceImage&& other) noexcept;
        DeviceImage& operator=(std::nullptr_t);
        DeviceImage& operator=(const DeviceImage&) = delete;
        DeviceImage& operator=(DeviceImage&& other) noexcept;
        ~DeviceImage();
        
        //----------------------------------------------------------------------------------------------------
        /// @brief : Creates a device image using an existing image. When this object is destroyed, the
        ///     image resource will not be destroyed. This is to be used for cases like the Swapchain.  
        //----------------------------------------------------------------------------------------------------
        DeviceImage(RenderDevice& device, const vk::Image image, const ImageDesc& imageDesc);

        //----------------------------------------------------------------------------------------------------
        /// @brief : Allocates a new image asset.
        //----------------------------------------------------------------------------------------------------
        DeviceImage(RenderDevice& device, const AllocateImageDesc& allocDesc);
        
        //----------------------------------------------------------------------------------------------------
        /// @brief : Set a debug name for this image. 
        //----------------------------------------------------------------------------------------------------
        void                SetDebugName(const std::string& name);

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
        vk::Image           GetVkImage() const      { return m_image; }

        //----------------------------------------------------------------------------------------------------
        /// @brief : Get the number of pixels for a particular mip level. Mip level 0 is used by default,
        ///     which is the original image.
        //----------------------------------------------------------------------------------------------------
        uint64              GetPixelCount(const uint16 mipLevel = 0) const;

        //----------------------------------------------------------------------------------------------------
        /// @brief : Get the number of bytes in a single pixel. 
        //----------------------------------------------------------------------------------------------------
        uint64              GetPixelSize() const;

        //----------------------------------------------------------------------------------------------------
        /// @brief : Get the size of a particular dimension (width = 0, height = 1, depth = 2) for a given mip level.
        ///     Mip level 0 is used by default, which is the original image.
        //----------------------------------------------------------------------------------------------------
        uint16              GetDimensionSize(const uint16 dimensionIndex, const uint32 mipLevel = 0) const;
        
        //----------------------------------------------------------------------------------------------------
        /// @brief : Advanced use. Get the native vulkan object handle, and the type.
        //----------------------------------------------------------------------------------------------------
        NativeVkObject      GetNativeVkObject() const;
    
    private:
        friend class DataUploader;
        
        //----------------------------------------------------------------------------------------------------
        /// @brief : Allocates the Image. 
        //----------------------------------------------------------------------------------------------------
        void                AllocateResource(const RenderDevice& device, const AllocateImageDesc& allocDesc);

        //----------------------------------------------------------------------------------------------------
        /// @brief : Submits the resource to the Renderer to be freed.
        //----------------------------------------------------------------------------------------------------
        void                FreeImage();
        
    private:
        RenderDevice*       m_pDevice = nullptr;
        vk::Image           m_image = nullptr;
        ImageDesc           m_desc{};
        VmaAllocation       m_allocation = nullptr;     // Device Memory associated with the Texture.
        bool                m_ownsNativeObjects = true; // If true, then on destruction the image will be freed.
    };

    static_assert(DeviceObjectType<DeviceImage>);
}