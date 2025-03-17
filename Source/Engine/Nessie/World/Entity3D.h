// Actor.h
#pragma once
#include "Core/Events/MulticastDelegate.h"
#include "Math/Rotation.h"
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
        
    private:
        Vector3 m_location;
        Rotation m_rotation;
        Vector3 m_scale;
        
        WorldTransformUpdatedEvent m_onWorldTransformUpdated{};
        Mat4 m_worldTransformMatrix{};
        bool m_worldTransformNeedsUpdate = false;

    public:
        void Rotate(const float angle, const Vector3& axis);
        void Rotate(const Rotation& rotation);
        void Translate(const Vector3& translation);
        void Scale(const float uniformScale);
        void Scale(const Vector3& scale);

        void SetLocalLocation(const Vector3& location);
        void SetLocalRotation(const Rotation& rotation);
        void SetLocalRotation(const Vector3& eulerAngles);
        void SetLocalScale(const Vector3& scale);
        void SetLocalTransform(const Vector3& location, const Rotation& rotation, const Vector3& scale);

        void SetWorldLocation(const Vector3& location);
        void SetWorldRotation(const Rotation& rotation);
        void SetWorldScale(const Vector3& scale);
        void SetWorldTransform(const Mat4& transform);
        void SetWorldTransform(const Vector3& worldLocation, const Rotation& worldRotation, const Vector3& worldScale);

        [[nodiscard]] Vector3           GetLocation() const;
        [[nodiscard]] Rotation          GetRotation() const;
        [[nodiscard]] Vector3           GetScale() const;
        [[nodiscard]] const Vector3&    GetLocalLocation() const; 
        [[nodiscard]] const Rotation&   GetLocalRotation() const;
        [[nodiscard]] const Vector3&    GetLocalScale() const;
        [[nodiscard]] Mat4              LocalTransformMatrix() const;
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
