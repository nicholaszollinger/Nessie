// CollisionGroup.cpp
#include "CollisionGroup.h"

namespace nes
{
    const CollisionGroup CollisionGroup::s_Invalid;
    
    bool CollisionGroup::CanCollide(const CollisionGroup& other) const
    {
        // Call CanCollide function of the first group filter that's not null.
        if (m_pFilter != nullptr)
            return m_pFilter->CanCollide(*this, other);
        else if (other.m_pFilter != nullptr)
            return other.m_pFilter->CanCollide(other, *this);

        // Default to true if no filters exist.
        return true;
    }
}
