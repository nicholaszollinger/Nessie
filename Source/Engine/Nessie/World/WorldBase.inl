// WorldBase.inl
#pragma once

namespace nes
{
    template <ComponentSystemType Type>
    StrongPtr<Type> WorldBase::GetSystem() const
    {
        const auto typeID = entt::type_id<Type>();
        if (auto pResult = GetSystem(typeID.hash()))
        {
            return Cast<Type>(pResult);
        }

        NES_WARN("No System of type '{}' found in World!", typeID.name());
        return nullptr;
    }
    
    template <ComponentSystemType Type>
    StrongPtr<Type> WorldBase::AddComponentSystem()
    {
        StrongPtr<Type> pNewSystem = Create<Type>(*this);
        pNewSystem->RegisterComponentTypes();
        m_systems.emplace_back(pNewSystem);
        m_systemMap.emplace(entt::type_id<Type>().hash(), m_systems.size() - 1);
        return pNewSystem;
    }
}