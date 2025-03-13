// Actor.h
#pragma once
#include <vector>
#include "Components/ActorComponent.h"
#include "Components/WorldComponent.h"
#include "Core/Memory/StrongPtr.h"
#include "Scene/SceneNode.h"

namespace nes
{
    template <typename Type>
    concept ActorComponentType = ComponentType<Type> && requires (Type type)
    {
        std::same_as<Actor, typename Type::OwnerType>;
    };

    template <typename Type>
    concept ActorDomainComponentType = TypeIsDerivedFrom<Type, WorldComponent>;
    
    //----------------------------------------------------------------------------------------------------
    ///		@brief : An Actor is an Entity that exists in 3D space. 
    //----------------------------------------------------------------------------------------------------
    class Actor final : public TSceneNode<Actor>
    {
        friend class World;
        
        std::vector<StrongPtr<ActorComponent>>  m_components{};
        StrongPtr<WorldComponent>               m_pRootComponent = nullptr;
        World* m_pWorld = nullptr;

    public:
        template <ActorComponentType Type> 
        StrongPtr<Type> AddComponent(const StringID& componentName);

        template <ActorComponentType Type>
        StrongPtr<Type> GetComponent();
        
        virtual void SetParent(Actor* pParent) override;
        [[nodiscard]] virtual std::vector<Actor*> GetChildren() const override;
        [[nodiscard]] virtual bool    IsValid() const override;
        
        void AddTranslation(const Vector3& deltaLocation);
        void AddRotation(const Quat& deltaRotation);
        void AddScale(const Vector3& deltaScale);
        void AddScale(const float deltaUniformScale);

        void SetTransform(const Mat4& transform);
        void SetLocation(const Vector3& location);
        void SetOrientation(const Quat& orientation);
        void SetScale(const Vector3& scale);

        void SetLocalTransform(const Transform& localTransform);
        void SetLocalTransform(const Vector3& location, const Quat& orientation, const Vector3& scale);
        void SetLocalLocation(const Vector3& localLocation);
        void SetLocalOrientation(const Quat& localOrientation);
        void SetLocalScale(const Vector3& localScale);

        [[nodiscard]] Mat4      GetTransformMatrix() const;
        [[nodiscard]] Mat4      GetLocalTransformMatrix() const;
        [[nodiscard]] Vector3   GetLocation() const;
        [[nodiscard]] Vector3   GetScale() const;
        [[nodiscard]] Quat      GetOrientation() const;
        [[nodiscard]] Vector3   GetLocalLocation() const;
        [[nodiscard]] Vector3   GetLocalScale() const;
        [[nodiscard]] Quat      GetLocalOrientation() const;
        
        void SetRootComponent(StrongPtr<WorldComponent>& pRoot);
        [[nodiscard]] StrongPtr<WorldComponent> GetRootComponent() const { return m_pRootComponent; }
        [[nodiscard]] virtual Actor*            GetParent() const override final;
        [[nodiscard]] World*                    GetWorld() const { return m_pWorld; }
        [[nodiscard]] Scene*                    GetScene() const;

    protected:
        virtual bool Init() override;
        virtual void OnParentSet(Actor* pParent) override;
        virtual void OnChildAdded(Actor* pChild) override;
        virtual void OnEnabled() override;
        virtual void OnDisabled() override;
        virtual void OnBeginDestroy() override;
        virtual void OnFinishDestroy() override;
        
        void NotifyComponentsOnDestroy();
        void NotifyComponentsOnEnabled();
        void NotifyComponentsOnDisabled();
        void NotifyComponentsOnParentSet(Actor* pParent);
        void NotifyComponentsOnChildAdded(Actor* pChild);
        void NotifyComponentsOnChildRemoved(Actor* pChild);

    private:
        bool FinishAddComponent(StrongPtr<ActorComponent>&& pComponent);
    };

    //----------------------------------------------------------------------------------------------------
    ///		@brief : Add a Component to this Actor.
    ///		@tparam Type : Type of Component to Add.
    ///     @param componentName : User-defined name of the Component to identify it.
    ///		@returns : Pointer to the created Component.
    //----------------------------------------------------------------------------------------------------
    template <ActorComponentType Type>
    StrongPtr<Type> Actor::AddComponent(const StringID& componentName)
    {
        StrongPtr<Type> pComponent = MakeStrong<Type>();
        pComponent->m_pOwner = this;
        pComponent->m_name = componentName;
        
        // Handle World Component attachment:
        if constexpr (ActorDomainComponentType<Type>)
        {
            if (m_pRootComponent == nullptr)
                m_pRootComponent = pComponent;
            else
            {
                pComponent->SetParent(m_pRootComponent.Get());
            }
        }

        StrongPtr<ActorComponent> pActorComponent = Cast<ActorComponent>(pComponent);
        if (!FinishAddComponent(std::move(pActorComponent)))
        {
            pComponent.Reset();
        }
        
        return pComponent;
    }

    //----------------------------------------------------------------------------------------------------
    ///		@brief : Returns the first component of the given type. If none are found, returns nullptr. 
    ///		@tparam Type : Type of Component you are looking for.
    //----------------------------------------------------------------------------------------------------
    template <ActorComponentType Type>
    StrongPtr<Type> Actor::GetComponent()
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
