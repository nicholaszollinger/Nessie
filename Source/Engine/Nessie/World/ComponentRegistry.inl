﻿// ComponentRegistry.inl
#pragma once
namespace nes
{
    inline ComponentRegistry& ComponentRegistry::Get()
    {
        static ComponentRegistry registry;
        return registry;
    }

    template <ComponentType Type>
    ComponentTypeDesc& ComponentRegistry::GetTypeDesc()
    {
        std::shared_lock lock(m_mutex);
        return GetTypeDescUnlocked<Type>();
    }

    template <ComponentType Type>
    ComponentTypeDesc& ComponentRegistry::GetTypeDescUnlocked()
    {
        const entt::id_type id = entt::type_hash<Type>::value();
        return m_componentTypes[id];
    }

    template <ComponentType Type>
    void ComponentRegistry::RegisterComponent(const std::string& name)
    {
        std::unique_lock lock(m_mutex);
        ComponentTypeDesc& typeDesc = GetTypeDescUnlocked<Type>();
        if (typeDesc.m_isRegistered)
            return;

        const entt::id_type id = entt::type_hash<Type>::value();
        
        if constexpr (SerializableComponent<Type>)
        {
            typeDesc.m_serializeYAML = [name](YAML::Emitter& emitter, EntityRegistry& registry, EntityHandle entity) 
            { 
                if (const Type* comp = registry.TryGetComponent<Type>(entity))
                {
                    emitter << YAML::BeginMap << YAML::Key << name << YAML::Value;
                    Type::Serialize(emitter, *comp);
                    emitter << YAML::EndMap;
                }
            };

            typeDesc.m_deserializeYAML = [](const YAML::Node& node, EntityRegistry& registry, EntityHandle entity)
            {
                Type& comp = registry.AddComponent<Type>(entity);
                Type::Deserialize(node, comp);
            };
        }

        typeDesc.m_copyFunction = [](EntityRegistry& srcRegistry, EntityRegistry& dstRegistry, EntityHandle srcEntity, EntityHandle dstEntity)
        {
            if (const Type* pComp = srcRegistry.TryGetComponent<Type>(srcEntity))
            {
                [[maybe_unused]] auto& newComp = dstRegistry.AddComponent<Type>(dstEntity, *pComp);
            }
        };
        
        typeDesc.m_name = name;
        m_nameToTypeID.emplace(name, id);
        typeDesc.m_isRegistered = true;

        NES_LOG("ComponentRegistry: Registered Component: '{}'", name);
    }

    inline const ComponentTypeDesc* ComponentRegistry::GetComponentDescByName(const std::string& name)
    {
        std::shared_lock lock(m_mutex);
        
        if (auto it = m_nameToTypeID.find(name); it != m_nameToTypeID.end())
        {
            entt::id_type id = it->second;
            NES_ASSERT(m_componentTypes.contains(id));
            return &m_componentTypes.at(id);
        }

        NES_WARN("Failed to find registered ComponentType: '{}'! Make sure you registered the Type with NES_REGISTER_COMPONENT(Type)", name);
        return nullptr;
    }

    template <ComponentType Type>
    bool ComponentRegistry::IsRegistered()
    {
        auto& typeDesc = GetTypeDesc<Type>();
        return typeDesc.m_isRegistered;
    }
}