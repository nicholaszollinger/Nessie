// EntityRegistry.inl
#pragma once
namespace nes
{
    void EntityRegistry::MarkEntitiesForDestruction(auto& view)
    {
        for (auto entity : view)
        {
            TryDisableEntity(entity);
        }
        
        m_registry.insert<PendingDestruction>(view.begin(), view.end());
    }
    
    template <ComponentType Type>
    auto EntityRegistry::OnComponentCreated()
    {
        return entt::sink{ m_registry.on_construct<Type>() };
    }

    template <ComponentType Type>
    auto EntityRegistry::OnComponentDestroyed()
    {
        return entt::sink{ m_registry.on_destroy<Type>() };
    }

    template <ComponentType Type, typename... Args>
    Type& Entity::AddComponent(Args&&... args)
    {
        NES_ASSERT(m_pRegistry != nullptr);
        return m_pRegistry->AddComponent<Type>(m_handle, std::forward<Args>(args)...);
    }
    
    template <ComponentType Type>
    void Entity::RemoveComponent()
    {
        NES_ASSERT(m_pRegistry != nullptr);
        m_pRegistry->RemoveComponent<Type>(m_handle);
    }
    
    template <ComponentType Type>
    Type& Entity::GetComponent()
    {
        NES_ASSERT(m_pRegistry != nullptr);
        return m_pRegistry->GetComponent<Type>(m_handle);
    }
    
    template <ComponentType Type>
    const Type& Entity::GetComponent() const
    {
        NES_ASSERT(m_pRegistry != nullptr);
        return m_pRegistry->GetComponent<Type>(m_handle);
    }
    
    template <ComponentType Type>
    Type* Entity::TryGetComponent()
    {
        NES_ASSERT(m_pRegistry != nullptr);
        return m_pRegistry->TryGetComponent<Type>(m_handle);
    }
    
    template <ComponentType Type>
    const Type* Entity::TryGetComponent() const
    {
        NES_ASSERT(m_pRegistry != nullptr);
        return m_pRegistry->TryGetComponent<Type>(m_handle);
    }
    
    template <ComponentType Type>
    bool Entity::HasComponent() const
    {
        return TryGetComponent<Type>() != nullptr;   
    }
    
    inline EntityID Entity::GetID() const
    {
        return GetComponent<nes::IDComponent>().GetID();
    }
    
    inline bool Entity::IsValid() const
    {
        return m_pRegistry != nullptr && m_handle != kInvalidEntityHandle && m_pRegistry->IsValidEntity(m_handle);   
    }
    
    inline void Entity::Destroy()
    {
        if (!IsValid())
            return;
    
        m_pRegistry->DestroyEntity(m_handle);
    }
    
    inline bool Entity::IsMarkedForDestruction() const
    {
        return HasComponent<nes::PendingDestruction>();
    }
    
    inline IDComponent& Entity::GetIDComponent()
    {
        return GetComponent<nes::IDComponent>();
    }
    
    inline const IDComponent& Entity::GetIDComponent() const
    {
        return GetComponent<nes::IDComponent>();
    }
    
    inline void Entity::SetName(const std::string& name)
    {
        GetComponent<nes::IDComponent>().SetName(name);
    }
    
    inline const std::string& Entity::GetName() const
    {
        return GetComponent<nes::IDComponent>().GetName();
    }
}