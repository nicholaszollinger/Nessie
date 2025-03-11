// Component.h
#pragma once
#include "EntityDomain.h"
#include "Core/Generic/Concepts.h"
#include "Core/Generic/TypeInfo.h"
#include "Core/String/StringID.h"

//----------------------------------------------------------------------------------------------------
//		NOTES:
//		
///		@brief : Defines the TypeID and Typename propeties for a Component. This must be used at the 
//             top of the body of the derived Component class.
///		@param componentType : Typename of the Component, not in quotes.
//----------------------------------------------------------------------------------------------------
#define NES_DEFINE_COMPONENT_TYPE(componentType)                                                             \
private:                                                                                                     \
    static_assert(std::is_class_v<componentType>, "NES_DEFINE_NODE_TYPE_INFO(): componentType must be a class or a struct");  \
    using UnderlyingType = componentType;                                                                    \
    static constexpr nes::TypeID kTypeID = HashString64(#componentType);                                     \
    public:                                                                                                      \
    [[nodiscard]] static nes::TypeID GetStaticTypeID() { return kTypeID; }                                   \
    [[nodiscard]] virtual nes::TypeID GetTypeID() const override { return kTypeID; }                         \
    [[nodiscard]] static const char* GetStaticTypename() { return #componentType; }                          \
    [[nodiscard]] virtual const char* GetTypename() const override { return #componentType; }                \
private:

namespace nes
{
    class Entity;
    class Scene;
    
    class Component
    {
        friend class Entity;
        Entity* m_pOwner;
        StringID m_name; // Might only be necessary for Editor versions of the app.

    protected:
        bool m_isEnabled = true;
        bool m_isQueuedForDestruction = false;

    public:
        Component() = default;
        virtual ~Component() = default;
        Component(const Component&) = delete;
        Component& operator=(const Component&) = delete;
        Component(Component&&) = delete;
        Component& operator=(Component&&) = delete;

    public:
        virtual void SetEnabled(const bool enabled);
        void SetName(const StringID& name) { m_name = name; }
        
        [[nodiscard]] virtual TypeID        GetTypeID() const = 0;
        [[nodiscard]] virtual const char*   GetTypename() const = 0;
        [[nodiscard]] virtual bool          IsEnabled() const;
        [[nodiscard]] StringID              GetName() const { return m_name; }
        [[nodiscard]] Entity*               GetOwner() const { return m_pOwner; }
        [[nodiscard]] virtual EntityDomain  GetDomain() const;

    protected:
        virtual bool Init();
        virtual void OnEntityParentSet([[maybe_unused]] Entity* pParent)    {}
        virtual void OnEntityChildAdded([[maybe_unused]] Entity* pChild)    {}
        virtual void OnEntityChildRemoved([[maybe_unused]] Entity* pChild)  {}
        virtual void OnDestroy()  {}
        virtual void OnEnabled()  {}
        virtual void OnDisabled() {}
        [[nodiscard]] Scene* GetScene() const;
    };

    template <typename Type>
    concept ComponentType = HasValidTypeInfo<Type> && requires()
    {
        // "UnderlyingType" member must be the same as the Type passed in.
        // This is a check to make sure the above macro was used for the derived
        // class.
        std::same_as<typename Type::UnderlyingType, Type>;

        // Must be default constructible.
        std::is_default_constructible_v<Type>;
    };
}
