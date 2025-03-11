// Actor.h
#pragma once
#include "Components/WorldComponent.h"
#include "Scene/Entity.h"

namespace nes
{
    class World;
    
    //----------------------------------------------------------------------------------------------------
    ///		@brief : An Actor is an Entity that exists in 3D space. 
    //----------------------------------------------------------------------------------------------------
    class Actor final : public Entity
    {
        WorldComponent* m_pRootComponent = nullptr;
        World* m_pWorld = nullptr;

    public:
        void AddTranslation(const Vector3& deltaLocation);
        void AddRotation(const Quat& deltaRotation);
        void AddScale(const Vector3& deltaScale);
        void AddScale(const float deltaUniformScale);

        void SetActorTransform(const Mat4& transform);
        void SetActorLocation(const Vector3& location);
        void SetActorOrientation(const Quat& orientation);
        void SetActorScale(const Vector3& scale);

        void SetActorLocalTransform(const Transform& localTransform);
        void SetActorLocalLocation(const Vector3& localLocation);
        void SetActorLocalOrientation(const Quat& localOrientation);
        void SetActorLocalScale(const Vector3& localScale);

        [[nodiscard]] Mat4      GetActorTransformMatrix() const;
        [[nodiscard]] Mat4      GetActorLocalTransformMatrix() const;
        [[nodiscard]] Vector3   GetActorLocation() const;
        [[nodiscard]] Vector3   GetActorScale() const;
        [[nodiscard]] Quat      GetActorOrientation() const;
        [[nodiscard]] Vector3   GetActorLocalLocation() const;
        [[nodiscard]] Vector3   GetActorLocalScale() const;
        [[nodiscard]] Quat      GetActorLocalOrientation() const;
        
        [[nodiscard]] virtual EntityDomain GetDomain() const override final { return EntityDomain::Physical3D; }
        
        void SetRootComponent(WorldComponent* pRoot);
        [[nodiscard]] WorldComponent* GetRootComponent() const { return m_pRootComponent; }
        [[nodiscard]] World* GetWorld() const { return m_pWorld; }
    
    protected:
        //virtual void OnParentSet() override;
    };
}
