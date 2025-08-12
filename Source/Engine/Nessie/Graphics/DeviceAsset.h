// GraphicsResource.h
#pragma once
#include "GraphicsCore.h"
#include "volk.h"

namespace nes
{
    class RenderDevice;

    //----------------------------------------------------------------------------------------------------
    /// @brief : Base class for all graphics assets that will be created with a RenderDevice. 
    //----------------------------------------------------------------------------------------------------
    class DeviceAsset
    {
    public:
        explicit        DeviceAsset(RenderDevice& device) : m_device(device) {}
        virtual         ~DeviceAsset() = default;

        //----------------------------------------------------------------------------------------------------
        /// @brief : Get the Render Device that created this resource. 
        //----------------------------------------------------------------------------------------------------
        RenderDevice&   GetDevice() const { return m_device; }

        //----------------------------------------------------------------------------------------------------
        /// @brief : Set the debug name for a resource.
        //----------------------------------------------------------------------------------------------------
        virtual void    SetDebugName(const char* name) = 0;

    protected:
        RenderDevice&   m_device;
    };

    //----------------------------------------------------------------------------------------------------
    /// @brief : Concept that requires that a type has a constructor that only takes in a RenderDevice&, and
    ///     a public GetDevice() function that returns the RenderDevice&.
    //----------------------------------------------------------------------------------------------------
    template <typename Type>
    concept DeviceAssetType = requires(RenderDevice& device, Type type)
    {
        Type(device);
        { type.GetDevice() } -> std::same_as<RenderDevice&>;
    };
}
