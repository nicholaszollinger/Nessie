// Component.cpp
#include "Component.h"
#include "Entity.h"
#include "Debug/Assert.h"

namespace nes
{
    //----------------------------------------------------------------------------------------------------
    //		NOTES:
    //		
    ///		@brief : Called during Owner Entity's Init. However, other Components  are not guaranteed to
    ///             be initialized yet. Init() is meant for internal initialization. PostInit()
    ///             should be used for references between components. 
    ///		@returns : 
    //----------------------------------------------------------------------------------------------------
    bool Component::Init()
    {
        if (IsEnabled())
        {
            OnEnabled();
        }

        return true;
    }
    
    void Component::SetEnabled(const bool enabled)
    {
        if (m_isEnabled == enabled)
            return;

        m_isEnabled = enabled;
        
        if (m_isEnabled)
        {
            OnEnabled();
        }

        else
        {
            OnDisabled();
        }
    }

    bool Component::IsEnabled() const
    {
        // Should always have an owner.
        NES_ASSERT(m_pOwner);
        
        if (!m_pOwner->IsEnabled())
        {
            return false;
        }

        return m_isEnabled;
    }

    //----------------------------------------------------------------------------------------------------
    ///		@brief : Get a reference to the World that the owning Entity is in. 
    //----------------------------------------------------------------------------------------------------
    Scene* Component::GetScene() const
    {
        NES_ASSERT(m_pOwner);
        return m_pOwner->GetScene();
    }

    //----------------------------------------------------------------------------------------------------
    ///		@brief : Get the Domain that this Component is a part of. By default, a Component is a part of
    ///         the Abstract Domain.
    //----------------------------------------------------------------------------------------------------
    EntityDomain Component::GetDomain() const
    {
        return EntityDomain::Abstract;
    }
}
