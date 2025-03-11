// Entity.h
#pragma once
#include "BleachNew.h"
#include "Component.h"
#include "Core/Generic/GenerationalID.h"
#include "Debug/Assert.h"

namespace nes
{
    //----------------------------------------------------------------------------------------------------
    //      [Consider] It would be good if this Handle contained the Domain and the Generational Index.
    ///		@brief : An Entity's World Handle defines where the Entity is in the World structure.
    ///         IMPORTANT: This is a runtime identifier-it is not consistent between executions of the program.
    //----------------------------------------------------------------------------------------------------
    using LayerHandle = nes::GenerationalID<uint64_t>;

    //----------------------------------------------------------------------------------------------------
    ///		@brief : Get the unique identifier for this Entity. This is what is saved to disk. 
    //----------------------------------------------------------------------------------------------------
    using EntityID = uint64_t;

    class Entity;
    
    template <typename Type>
    concept EntityType = TypeIsSameOrDerived<Type, Entity> && requires(Type entity)
    {
        std::is_default_constructible_v<Type>;
    };
    
    //----------------------------------------------------------------------------------------------------
    //      [TODO]: RemoveComponent(), Checking that the Components are valid for the Entity's Layer.
    //		
    ///		@brief : 
    //----------------------------------------------------------------------------------------------------
    class Entity
    {
        friend class Scene;
        friend class EntityLayer;
        friend class EntityPoolBase;
        
        std::vector<Component*> m_components{};
        std::vector<Entity*> m_children{};
        Entity* m_pParent               = nullptr;
        EntityLayer* m_pLayer           = nullptr;
        LayerHandle m_handle{};
        EntityID m_id{};
        StringID m_name{};      // Could be an Editor Only construct
        bool m_isEnabled                = true;
        bool m_isMarkedForDestruction   = false;
        bool m_isInitialized            = false;

    protected:
        Entity() = default;
        
    public:
        Entity(const Entity& other) = delete;
        Entity& operator=(const Entity& other) = delete;
        Entity(Entity&& other) noexcept = default;
        Entity& operator=(Entity&& other) noexcept = default;
        virtual ~Entity() = default;

        bool Init(); // [TODO]: Init should be private:
        void Destroy();
        void DestroyAndAllChildren();
        void SetEnabled(bool isEnabled);

        void SetParent(Entity* pParent);
        void AddChild(Entity* pChild);
        void RemoveChild(Entity* pChild);
        [[nodiscard]] const std::vector<Entity*>& GetChildren() const   { return m_children; }
        [[nodiscard]] Entity* GetParent() const                         { return m_pParent; }
        [[nodiscard]] size_t GetNumChildren() const                     { return m_children.size(); }

        template <ComponentType Type>
        Type* AddComponent(const StringID& componentName);

        template <ComponentType Type>
        [[nodiscard]] Type*                             GetComponent() const;
        [[nodiscard]] const std::vector<Component*>&    GetAllComponents() const;
        [[nodiscard]] const EntityID&                   GetID() const;
        [[nodiscard]] const LayerHandle&                GetHandle() const;
        [[nodiscard]] Scene*                            GetScene() const;
        [[nodiscard]] EntityLayer*                      GetLayer() const;
        [[nodiscard]] virtual EntityDomain              GetDomain() const;
        [[nodiscard]] bool                              IsEnabled() const;
        [[nodiscard]] bool                              IsMarkedForDestruction() const;
        [[nodiscard]] bool                              IsInitialized() const;
        [[nodiscard]] bool                              IsValid() const;
        
        void SetName(const StringID& name) { m_name = name; }
        [[nodiscard]] StringID GetName() const { return m_name; }

    protected:
        void NotifyComponentsOnDestroy() const;
        void NotifyComponentsOnEnabled() const;
        void NotifyComponentsOnDisabled() const;
        void NotifyComponentsOnParentSet() const;
        void NotifyComponentsOnChildAdded(Entity* pChild);
        void NotifyComponentsOnChildRemoved(Entity* pChild);

    private:
        void DestroyEntity(bool shouldNotify);
        void RemoveFromHierarchy();
    };

    //----------------------------------------------------------------------------------------------------
    ///		@brief : Add a Component to this Entity.
    ///		@tparam Type : Type of Component to Add.
    ///     @param componentName : User-defined name of the Component to identify it.
    ///		@returns : Reference to the created Component.
    //----------------------------------------------------------------------------------------------------
    template <ComponentType Type>
    Type* Entity::AddComponent(const StringID& componentName)
    {
        Type* pComponent = BLEACH_NEW(Type());
        pComponent->m_pOwner = this;
        pComponent->m_name = componentName;
        
        // Ensure that the Component is valid for this Entity's Domain
        // - It would be nice if this was a compile time check.
        NES_ASSERT(DomainsAreCompatible(GetDomain(), pComponent->GetDomain()));
        
        // If the Entity has already been initialized, run through the initialization
        // of the Component.
        if (IsInitialized())
        {
            if (!pComponent->Init())
            {
                NES_ERRORV("Entity", "AddComponent() failed for type: ", Type::GetStaticTypename(), "!");
                BLEACH_DELETE(pComponent);
                return nullptr;
            }
        }
        
        m_components.push_back(pComponent);
        return pComponent;
    }

    //----------------------------------------------------------------------------------------------------
    //		NOTES:
    //		
    ///		@brief : Returns the first component of the given type. If none are found, returns nullptr. 
    ///		@tparam Type : Type of Component you are looking for.
    //----------------------------------------------------------------------------------------------------
    template <ComponentType Type>
    Type* Entity::GetComponent() const
    {
        for (auto& pComponent : m_components)
        {
            if (pComponent->GetTypeID() == Type::GetStaticTypeID())
            {
                return pComponent;
            }
        }

        return nullptr;
    }
}