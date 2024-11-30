// IDComponent.cpp

#include "IDComponent.h"
#include "Random/UUID.h"

namespace nes
{
    static UUIDGenerator g_idGenerator{};

    //----------------------------------------------------------------------------------------------------
    ///		@brief : Generates a unique ID on Default Construction.
    //----------------------------------------------------------------------------------------------------
    IDComponent::IDComponent()
        : m_id(g_idGenerator.GenerateUUID().GetValue())
    {
        //
    }

    //----------------------------------------------------------------------------------------------------
    ///		@brief : Sets the ID to a given value.
    //----------------------------------------------------------------------------------------------------
    IDComponent::IDComponent(const EntityID& id)
        : m_id(id)
    {
        //
    }
}