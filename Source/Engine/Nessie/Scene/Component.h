// Component.h
#pragma once
#include "Core/Generic/Concepts.h"
#include "Core/Generic/TypeInfo.h"
#include "Core/String/StringID.h"
#include "Debug/Assert.h"

//----------------------------------------------------------------------------------------------------
//		NOTES:
///		@brief : Defines the TypeID and Typename properties for a Component. This must be used at the 
//             top of the body of the derived Component class.
///		@param componentType : Typename of the Component, not in quotes.
//----------------------------------------------------------------------------------------------------
#define NES_DEFINE_COMPONENT_TYPE(componentType) \
    NES_DEFINE_TYPE_INFO(componentType)

namespace nes
{
    class Scene;
    template <typename Type> class TEntity;

    //----------------------------------------------------------------------------------------------------
    ///		@brief : A Component is defined by the SceneNode Type that it can attach to. It is Components that
    ///         give SceneNodes their functionality. Components can be enabled and disabled.
    ///		@tparam NodeType : Type of Node that this class of Components can attach to.
    //----------------------------------------------------------------------------------------------------
    template <typename NodeType>
    class TComponent
    {
    public:
        using OwnerType = NodeType;
        using OwnerNodeType = TEntity<NodeType>;
        
    private:
        friend OwnerType;
        friend OwnerNodeType;
        
        OwnerType* m_pOwner = nullptr;
        StringID m_name;

    protected:
        bool m_isEnabled = true;
        bool m_isQueuedForDestruction = false;
        
    public:
        TComponent() = default;
        virtual ~TComponent() = default;
        TComponent(const TComponent&) = delete;
        TComponent& operator=(const TComponent&) = delete;
        TComponent(TComponent&&) noexcept = default;
        TComponent& operator=(TComponent&&) noexcept = default;
        
        virtual void SetEnabled(const bool enabled);
        void SetName(const StringID& name) { m_name = name; }
        
        [[nodiscard]] virtual const char*   GetTypename() const = 0;
        [[nodiscard]] virtual TypeID        GetTypeID() const = 0;
        [[nodiscard]] virtual bool          IsEnabled() const;
        [[nodiscard]] StringID              GetName() const     { return m_name; }
        [[nodiscard]] OwnerType*            GetOwner() const    { return m_pOwner; }

    protected:
        virtual bool Init();
        virtual void OnOwnerParentSet([[maybe_unused]] OwnerType* pParent)    {}
        virtual void OnOwnerChildAdded([[maybe_unused]] OwnerType* pChild)    {}
        virtual void OnOwnerChildRemoved([[maybe_unused]] OwnerType* pChild)  {}
        virtual void OnDestroy()  {}
        virtual void OnEnabled()  {}
        virtual void OnDisabled() {}
    };

    template <typename Type>
    concept ComponentType = HasValidTypeInfo<Type> && requires()
    {
        TypeIsDerivedFrom<Type, TComponent<typename Type::OwnerType>>;
        // Must be default constructible.
        std::is_default_constructible_v<Type>;
    };

    //----------------------------------------------------------------------------------------------------
    ///		@brief : Set the Component's enabled state. 
    //----------------------------------------------------------------------------------------------------
    template <typename NodeType>
    void TComponent<NodeType>::SetEnabled(const bool enabled)
    {
        if (m_isEnabled == enabled)
            return;

        m_isEnabled = enabled;
        
        if (m_isEnabled)
            OnEnabled();
        else
            OnDisabled();
    }

    //----------------------------------------------------------------------------------------------------
    ///		@brief : Return whether this Component is enabled or not. This will return false if
    ///         the Owner is disabled, regardless of the Component's internal enabled state.
    //----------------------------------------------------------------------------------------------------
    template <typename NodeType>
    bool TComponent<NodeType>::IsEnabled() const
    {
        NES_ASSERT(m_pOwner);
        if (!m_pOwner->IsEnabled())
            return false;
        
        return m_isEnabled;
    }

    //----------------------------------------------------------------------------------------------------
    ///		@brief : Called during its Node Owner's Init(). NOTE: Other Components are not guaranteed to
    ///           be initialized yet. Init() is meant for internal initialization.
    //----------------------------------------------------------------------------------------------------
    template <typename NodeType>
    bool TComponent<NodeType>::Init()
    {
        if (IsEnabled())
            OnEnabled();
        
        return true;
    }
}
