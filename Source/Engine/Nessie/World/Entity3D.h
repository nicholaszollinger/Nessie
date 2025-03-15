// Actor.h
#pragma once
#include "Core/Events/MulticastDelegate.h"
#include "Math/Transform.h"
#include "Scene/Entity.h"

namespace nes
{
    //----------------------------------------------------------------------------------------------------
    ///		@brief : An Actor is an Entity that exists in 3D space. 
    //----------------------------------------------------------------------------------------------------
    class Entity3D final : public TEntity<Entity3D>
    {
        friend class World;

    public:
        using WorldTransformUpdatedEvent = MulticastDelegate<>;
        
        Transform m_localTransform{};
        WorldTransformUpdatedEvent m_onWorldTransformUpdated{};
        Mat4 m_worldTransformMatrix{};
        bool m_worldTransformNeedsUpdate = false;

    public:
        void Rotate(const float angle, const Vector3& axis);
        void Rotate(const Quat& rotation);
        void Translate(const Vector3& translation);
        void Scale(const float uniformScale);
        void Scale(const Vector3& scale);

        void SetLocalLocation(const Vector3& location);
        void SetLocalOrientation(const Quat& orientation);
        void SetLocalOrientation(const Vector3& eulerAngles);
        void SetLocalScale(const Vector3& scale);
        void SetLocalTransform(const Transform& transform);
        void SetLocalTransform(const Vector3& location, const Quat& orientation, const Vector3& scale);

        void SetWorldLocation(const Vector3& location);
        void SetWorldOrientation(const Quat& orientation);
        void SetWorldScale(const Vector3& scale);
        void SetWorldTransform(const Mat4& transform);
        void SetWorldTransform(const Vector3& worldLocation, const Quat& worldOrientation, const Vector3& worldScale);

        [[nodiscard]] Vector3           GetLocation() const;
        [[nodiscard]] Quat              GetOrientation() const;
        [[nodiscard]] Vector3           GetScale() const;
        [[nodiscard]] const Vector3&    GetLocalLocation() const; 
        [[nodiscard]] const Quat&       GetLocalOrientation() const;
        [[nodiscard]] const Vector3&    GetLocalScale() const;
        [[nodiscard]] Mat4              GetLocalTransformMatrix() const;
        [[nodiscard]] const Mat4&       GetWorldTransformMatrix() const;
        [[nodiscard]] WorldTransformUpdatedEvent& OnWorldTransformUpdated() { return m_onWorldTransformUpdated; }

        [[nodiscard]] World* GetWorld() const;
        [[nodiscard]] Scene* GetScene() const;
        [[nodiscard]] bool WorldTransformNeedsUpdate() const;

    protected:
        virtual bool Init() override;
        virtual void OnParentSet(Entity3D* pParent) override;
        
        void MarkWorldTransformDirty();
        void UpdateWorldTransform(Entity3D* pParent, const Matrix4x4& localTransform);
        void PropagateTransformUpdateToChildren();
    };
}
