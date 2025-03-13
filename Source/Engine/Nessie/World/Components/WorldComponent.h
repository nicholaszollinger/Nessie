// WorldComponent.h
#pragma once
#include "ActorComponent.h"
#include "Core/Memory/StrongPtr.h"
#include "Math/Transform.h"

namespace nes
{
    class Actor;
    //----------------------------------------------------------------------------------------------------
    ///		@brief : A World Component defines the 3D Transform for Actors in the Scene. WorldComponents
    ///         can be parented to one another - useful as a 'dummy' component in the hierarchy to offset child
    ///         components.
    //----------------------------------------------------------------------------------------------------
    class WorldComponent : public ActorComponent
    {
        NES_DEFINE_COMPONENT_TYPE(WorldComponent)

        WorldComponent* m_pParent = nullptr;
        std::vector<WorldComponent*> m_children{};
        Transform m_localTransform{};
        Mat4 m_worldTransformMatrix{};
        bool m_worldTransformNeedsUpdate = false;
        
    public:
        WorldComponent() = default;
        
        void SetParent(WorldComponent* pParent);
        void AddChild(WorldComponent* pChild);
        void RemoveChild(WorldComponent* pChild);
        [[nodiscard]] WorldComponent*                       GetParent() const;
        [[nodiscard]] std::vector<WorldComponent*>          GetChildren() const;
        [[nodiscard]] const std::vector<WorldComponent*>&   GetAllChildren() const;

        virtual void SetEnabled(const bool enabled) override;
        [[nodiscard]] virtual bool IsEnabled() const override;

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

    private:
        virtual void OnParentChanged(WorldComponent* pParent);
        virtual void OnChildAdded([[maybe_unused]] WorldComponent* pChild) {}
        virtual void OnChildRemoved([[maybe_unused]] WorldComponent* pChild) {}
        virtual void OnWorldTransformUpdated() {}
        
        void MarkWorldTransformDirty();
        void UpdateWorldTransform(WorldComponent* pParent, const Matrix4x4& localTransform);
        void PropagateTransformUpdateToChildren();
        [[nodiscard]] bool WorldTransformNeedsUpdate() const;
    };
}