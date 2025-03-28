// Entity3DComponent.cpp
#include "Entity3DComponent.h"

#include "Scene/TickGroup.h"
#include "World/Entity3D.h"

namespace nes
{
    void Entity3DComponentTickFunction::ExecuteTick(const TickDeltaTime& deltaTime)
    {
        if (!m_tickWhilePaused && deltaTime.m_isPaused)
            return;
        
        m_pTarget->Tick(deltaTime.m_deltaTime);
    }

    void TickableEntity3DComponent::SetTickEnabled(const bool enabled)
    {
        m_tickFunction.SetTickEnabled(enabled);
    }

    void TickableEntity3DComponent::RegisterTickToGroup(TickGroup* pTickGroup)
    {
        NES_ASSERT(pTickGroup != nullptr);
        m_tickFunction.RegisterTick(pTickGroup);
    }

    bool TickableEntity3DComponent::Init()
    {
        m_tickFunction.m_pTarget = this;
        return Entity3DComponent::Init();
    }

    void TickableEntity3DComponent::OnEnabled()
    {
        m_tickFunction.SetTickEnabled(true);
    }

    void TickableEntity3DComponent::OnDisabled()
    {
        m_tickFunction.SetTickEnabled(false);
    }
}
