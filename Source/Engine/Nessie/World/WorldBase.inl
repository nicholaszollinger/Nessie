// WorldBase.inl
#pragma once

namespace nes
{
    template <ComponentSystemType Type>
    StrongPtr<Type> WorldBase::GetSystem()
    {
        for (auto& pSystem : m_systems)
        {
            if (StrongPtr<Type> pCastedSystem = Cast<Type>(pSystem))
            {
                return pCastedSystem;
            }
        }

        return nullptr;
    }
    
    template <ComponentSystemType Type>
    StrongPtr<Type> WorldBase::AddComponentSystem()
    {
        StrongPtr<Type> pNewSystem = Create<Type>(*this);
        pNewSystem->RegisterComponentTypes();
        m_systems.emplace_back(pNewSystem);
        return pNewSystem;
    }
}