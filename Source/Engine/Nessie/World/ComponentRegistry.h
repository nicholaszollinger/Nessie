// ComponentRegistry.h
#pragma once
#include "entt/entity/registry.hpp"
#include "Nessie/Core/Thread/Mutex.h"
#include "Nessie/Debug/Assert.h"
#include "Nessie/Core/String/FormatString.h"
#include "Component.h"
#include "Entity.h"

namespace nes
{
    class EntityRegistry;

    struct ComponentTypeDesc
    {
        using SerializeYAML = std::function<void(YAML::Emitter& emitter, EntityRegistry& registry, EntityHandle entity)>;
        using DeserializeYAML = std::function<void(const YAML::Node& node, EntityRegistry& registry, EntityHandle entity)>;
        using CopyFunction = std::function<void(EntityRegistry& srcRegistry, EntityRegistry& dstRegistry, EntityHandle srcEntity, EntityHandle dstEntity)>;

        // Component Functors generated on Registration.
        SerializeYAML           m_serializeYAML{};
        DeserializeYAML         m_deserializeYAML{};
        CopyFunction            m_copyFunction{};  
        
        // Meta Data
        std::string             m_name{};
        bool                    m_isRegistered = false;
    };

    class ComponentRegistry
    {
    public:
        //----------------------------------------------------------------------------------------------------
        /// @brief : Get the Component Registry instance. 
        //----------------------------------------------------------------------------------------------------
        static ComponentRegistry&   Get();

        //----------------------------------------------------------------------------------------------------
        /// @brief : Component Types must be registered in order to have OnConstruct/OnDestroy events invoked,
        ///     as well as being able to serialize/deserialize.
        //----------------------------------------------------------------------------------------------------
        template <ComponentType Type>
        void                        RegisterComponent(const std::string& name);
        
        //----------------------------------------------------------------------------------------------------
        /// @brief : Checks if a Component has been registered properly to the Registry.
        //----------------------------------------------------------------------------------------------------
        template <ComponentType Type>
        bool                        IsRegistered();

        //----------------------------------------------------------------------------------------------------
        /// @brief : Attempt to get a ComponentTypeDesc by name. If not found, this will return nullptr.
        //----------------------------------------------------------------------------------------------------
        const ComponentTypeDesc*    GetComponentDescByName(const std::string& name);

        //----------------------------------------------------------------------------------------------------
        /// @brief : Get the array of Component Type Descriptions. 
        //----------------------------------------------------------------------------------------------------
        std::vector<ComponentTypeDesc> GetAllComponentTypes() const;
    
    private:
        //----------------------------------------------------------------------------------------------------
        /// @brief : Get the information about a Component Type, including serialization methods. 
        //----------------------------------------------------------------------------------------------------
        template <ComponentType Type>
        ComponentTypeDesc&          GetTypeDesc();

        template <ComponentType Type>
        ComponentTypeDesc&          GetTypeDescUnlocked();

    private:
        std::unordered_map<entt::id_type, ComponentTypeDesc>    m_componentTypes;
        std::unordered_map<std::string, entt::id_type>          m_nameToTypeID;
        std::shared_mutex                                       m_mutex{};
    };
}

#define NES_REGISTER_COMPONENT(Type) nes::ComponentRegistry::Get().RegisterComponent<Type>(nes::StripNamespaceFromTypename(#Type))