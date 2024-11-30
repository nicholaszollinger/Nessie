#pragma once
// IDComponent.h
#include <cstdint>

namespace nes
{
    using EntityID = uint64_t;

    //----------------------------------------------------------------------------------------------------
    //		NOTES:
    //		
    ///		@brief : An IDComponent contains a unique Identifier for an Entity. This Component
    ///		        should be added to every Entity, because this can be serialized to keep any Entity
    ///             references consistent.
    //----------------------------------------------------------------------------------------------------
    class IDComponent
    {
    public:
        static constexpr EntityID kInvalidID = 0;

    private:
        EntityID m_id = kInvalidID;

    public:
        IDComponent();
        explicit IDComponent(const EntityID& id);

        [[nodiscard]] EntityID GetID() const { return m_id; }
    };

    // [TODO]: Serializer
}