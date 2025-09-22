// GBuffers.h
#pragma once
#include "Descriptor.h"
#include "DeviceImage.h"

namespace nes
{
    //----------------------------------------------------------------------------------------------------
    /// @brief : Properties of a GBuffer. 
    //----------------------------------------------------------------------------------------------------
    struct GBufferDesc
    {
        //----------------------------------------------------------------------------------------------------
        /// @brief : Set the array of color formats for the GBuffer. For each format, a color image will be
        ///     created.
        //----------------------------------------------------------------------------------------------------
        GBufferDesc&            SetColors(const std::vector<EFormat>& formats);

        //----------------------------------------------------------------------------------------------------
        /// @brief : Set the Depth format for the GBuffers.
        //----------------------------------------------------------------------------------------------------
        GBufferDesc&            SetDepth(EFormat depthFormat);

        //----------------------------------------------------------------------------------------------------
        /// @brief : Sets the maximum supported sample count across all set color and depth formats.
        /// @note : This should be called after setting the color and depth formats!
        //----------------------------------------------------------------------------------------------------
        GBufferDesc&            SetMaxSampleCount();
        
        std::vector<EFormat>    m_colorFormats{};
        EFormat                 m_depthFormat = EFormat::Unknown;
        uint32                  m_sampleCount = 1;
    };
    
    //----------------------------------------------------------------------------------------------------
    /// @brief : A GBuffer is a series of color attachments with optional depth management.
    ///
    /// This class manages multiple color buffers and a depth buffer for deferred rendering or other
    /// multi-target rendering techniques.
    /// - Supports MSAA
    /// - Depth Buffer is optional.
    /// - Resource cleanup is handled in the destructor.
    ///
    /// The GBuffer class acts the same as Device Objects; it is move only and can be set to nullptr to
    /// clean up resources.
    //----------------------------------------------------------------------------------------------------
    class GBuffer
    {
    public:
        GBuffer(std::nullptr_t) {}
        GBuffer(const GBuffer&) = delete;
        GBuffer(GBuffer&& other) noexcept;
        GBuffer& operator=(std::nullptr_t);
        GBuffer& operator=(const GBuffer&) = delete;
        GBuffer& operator=(GBuffer&& other) noexcept;
        ~GBuffer();

        //----------------------------------------------------------------------------------------------------
        /// @brief : Sets the description, but resources will not be created. Resize must be called.
        //----------------------------------------------------------------------------------------------------
        GBuffer(const GBufferDesc& desc);

        //----------------------------------------------------------------------------------------------------
        /// @brief : Resize the GBuffer.
        /// @note : All images will be in the layout: EImageLayout::Undefined!
        //----------------------------------------------------------------------------------------------------
        void                        Resize(RenderDevice& device, const uint32 width, const uint32 height);

        //----------------------------------------------------------------------------------------------------
        /// @brief : Get the color image at the given index.
        //----------------------------------------------------------------------------------------------------
        DeviceImage&                GetColorImage(const uint32 colorIndex = 0);

        //----------------------------------------------------------------------------------------------------
        /// @brief : Get the color image descriptor at the given index. 
        //----------------------------------------------------------------------------------------------------
        Descriptor&                 GetColorImageView(const uint32 colorIndex = 0);

        //----------------------------------------------------------------------------------------------------
        /// @brief : Get the format of a color image at the given index. 
        //----------------------------------------------------------------------------------------------------
        EFormat                     GetColorFormat(const uint32 colorIndex = 0) const;

        //----------------------------------------------------------------------------------------------------
        /// @brief : Get all color formats of the GBuffer.
        //----------------------------------------------------------------------------------------------------
        const std::vector<EFormat>& GetColorFormats() const                         { return m_desc.m_colorFormats; };

        //----------------------------------------------------------------------------------------------------
        /// @brief : Get the depth image.
        //----------------------------------------------------------------------------------------------------
        DeviceImage&                GetDepthImage()                                 { return m_depthImageAndView.m_image; }

        //----------------------------------------------------------------------------------------------------
        /// @brief : Get the depth image descriptor.
        //----------------------------------------------------------------------------------------------------
        Descriptor&                 GetDepthImageView()                             { return m_depthImageAndView.m_view; }

        //----------------------------------------------------------------------------------------------------
        /// @brief : Get the depth format used for the GBuffer. If equal to EFormat::Unknown, then the depth buffer
        ///     is not used.
        //----------------------------------------------------------------------------------------------------
        EFormat                     GetDepthFormat() const                          { return m_desc.m_depthFormat; }

        //----------------------------------------------------------------------------------------------------
        /// @brief : Get the current size of the GBuffer in pixels.
        //----------------------------------------------------------------------------------------------------
        UInt2                       GetSize() const                                 { return m_size; }
        
        //----------------------------------------------------------------------------------------------------
        /// @brief : Get the number of samples for the GBuffer. If equal to 1, then multisampling is disabled.
        //----------------------------------------------------------------------------------------------------
        uint32                      GetSampleCount() const                          { return m_desc.m_sampleCount; }
    
    private:
        //----------------------------------------------------------------------------------------------------
        /// @brief : Free all Color and Depth image resources and their views. 
        //----------------------------------------------------------------------------------------------------
        void                        FreeResources();

        //----------------------------------------------------------------------------------------------------
        /// @brief : Calls FreeResources() and invalidates the size and description. 
        //----------------------------------------------------------------------------------------------------
        void                        Destroy();

    private:
        struct ImageAndView
        {
            DeviceImage             m_image = nullptr;
            Descriptor              m_view = nullptr;
        };
        
        std::vector<ImageAndView>   m_colorImageAndViews{};    
        ImageAndView                m_depthImageAndView{};  // Optional depth image and view.
        UInt2                       m_size{};               // Width and height of the buffers.
        GBufferDesc                 m_desc{};               // Buffer properties.
    };
}