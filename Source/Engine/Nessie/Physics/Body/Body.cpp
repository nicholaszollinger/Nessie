// Body.cpp
#include "Body.h"

namespace nes
{
    void Body::SetInBroadPhaseInternal(const bool isInBroadPhase)
    {
        if (isInBroadPhase)
            m_flags.fetch_or(static_cast<uint8_t>(Flags::IsInBroadPhase), std::memory_order::relaxed);
        else
            m_flags.fetch_and(static_cast<uint8_t>(~static_cast<uint8_t>(Flags::InvalidateContactCache)), std::memory_order::relaxed);
    }
}
