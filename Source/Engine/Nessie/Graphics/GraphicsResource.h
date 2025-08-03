// GraphicsResource.h
#pragma once
#include "GraphicsCore.h"
#include "volk.h"

namespace nes
{
    class RenderDevice;

    //----------------------------------------------------------------------------------------------------
    // [TODO]: Add Ref Counting? Get rid of this all together in favor of a concept?
    /// @brief : Base class for all graphics resources that will be created with a RenderDevice. 
    //----------------------------------------------------------------------------------------------------
    class GraphicsResource
    {
    public:
        explicit        GraphicsResource(RenderDevice& device) : m_device(device) {}
        virtual         ~GraphicsResource() = default;

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
    concept GraphicsResourceType = requires(RenderDevice& device, Type type)
    {
        Type(device);
        { type.GetDevice() } -> std::same_as<RenderDevice&>;
    };
}
