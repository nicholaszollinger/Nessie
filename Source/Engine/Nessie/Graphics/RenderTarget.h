// RenderTargets.h
#pragma once
#include "Descriptor.h"
#include "DeviceImage.h"

namespace nes
{
    struct RenderTargetDesc
    {
        std::string     m_name{};
        uint32          m_sampleCount = 1;
        EFormat         m_format = EFormat::Unknown;
        EImagePlaneBits m_planes = EImagePlaneBits::Color;  // Can be equal to either of: Color, Depth, Stencil, or Depth|Stencil.
        ClearValue      m_clearValue{};
        UInt2           m_size{};                           // Current size of the image.   
    };

    //----------------------------------------------------------------------------------------------------
    /// @brief : A Render Target is a single image that is used as either a color or depth/stencil attachment
    ///     when rendering. Render Targets are grouped into a RenderTargetSet to be used with
    ///     CommandBuffer::BeginRendering().
    //----------------------------------------------------------------------------------------------------
    class RenderTarget
    {
    public:
        RenderTarget(std::nullptr_t) {}
        RenderTarget(const RenderTarget&) = delete;
        RenderTarget(RenderTarget&& other) noexcept;
        RenderTarget& operator=(std::nullptr_t);
        RenderTarget& operator=(const RenderTarget&) = delete;
        RenderTarget& operator=(RenderTarget&& other) noexcept;
        ~RenderTarget();

        //----------------------------------------------------------------------------------------------------
        /// @brief : Create a Render Target with the given description. The sample count will be clamped to the
        /// max supported value for the format. The format must be valid (not EFormat::Unknown).
        //----------------------------------------------------------------------------------------------------
        RenderTarget(RenderDevice& device, const RenderTargetDesc& desc);

        //----------------------------------------------------------------------------------------------------
        /// @brief : Resizes the image to the given size.
        //----------------------------------------------------------------------------------------------------
        void                        Resize(const uint32 width, const uint32 height);
        UInt2                       GetSize() const                             { return m_desc.m_size; }
        DeviceImage&                GetImage()                                  { return m_image; }
        Descriptor&                 GetView()                                   { return m_view; }
        const RenderTargetDesc&     GetDesc() const                             { return m_desc; }

        //----------------------------------------------------------------------------------------------------
        /// @brief : Get the pixel format of the image.
        //----------------------------------------------------------------------------------------------------
        EFormat                     GetFormat() const                           { return m_desc.m_format; }

        //----------------------------------------------------------------------------------------------------
        /// @brief : Set the value used to clear the image during CommandBuffer::ClearTargets.  
        //----------------------------------------------------------------------------------------------------
        void                        SetClearValue(const ClearValue& value)      { m_desc.m_clearValue = value; }
        const ClearValue&           GetClearValue() const                       { return m_desc.m_clearValue; }
        const std::string&          GetName() const                             { return m_desc.m_name; }

        //----------------------------------------------------------------------------------------------------
        /// @brief : Sets the sample count to the maximum supported value for the image format. 
        //----------------------------------------------------------------------------------------------------
        void                        SetMaxSupportedSampleCount();

        //----------------------------------------------------------------------------------------------------
        /// @brief : Sets the sample count to the given value, or the maximum supported value if too high.
        ///     Values must be 1, 2, 4, 8, or 16 
        //----------------------------------------------------------------------------------------------------
        void                        SetSampleCount(const uint32 sampleCount);
        uint32                      GetSampleCount() const                      { return m_desc.m_sampleCount; }       

        //----------------------------------------------------------------------------------------------------
        /// @brief : Returns whether this Render Target is for color render output. 
        //----------------------------------------------------------------------------------------------------
        bool                        IsColorTarget() const;

        //----------------------------------------------------------------------------------------------------
        /// @brief : Returns whether this Render Target can be used as a depth output.
        //----------------------------------------------------------------------------------------------------
        bool                        IsDepthTarget() const;

        //----------------------------------------------------------------------------------------------------
        /// @brief : Returns whether this Render Target can be used for stencil output.
        //----------------------------------------------------------------------------------------------------
        bool                        IsStencilTarget() const;

    private:
        //----------------------------------------------------------------------------------------------------
        /// @brief : Frees the image and descriptor resource, but preserves the description value so that the image can be
        ///     rebuilt.
        //----------------------------------------------------------------------------------------------------
        void                        FreeResources();

        //----------------------------------------------------------------------------------------------------
        /// @brief : Frees the resources and invalidates the description value. 
        //----------------------------------------------------------------------------------------------------
        void                        Destroy();

    private:
        RenderDevice*               m_pDevice = nullptr;
        DeviceImage                 m_image = nullptr;
        Descriptor                  m_view = nullptr;
        RenderTargetDesc            m_desc{};
    };

    //----------------------------------------------------------------------------------------------------
    /// @brief : Returns the maximum supported sample count for the range of targets.
    //----------------------------------------------------------------------------------------------------
    uint32 GetMaxSampleCountForTargets(const std::vector<RenderTarget*>& targets);
}