// Actor.h
#pragma once
#include "Core/Events/MulticastDelegate.h"
#include "Math/Rotation.h"
#include "Math/Mat4.h"
#include "Physics/Body/BodyID.h"
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

    public:
        //----------------------------------------------------------------------------------------------------
        ///	@brief : Rotate this Entity by a delta angle around an axis. The angle must be in radians.
        //----------------------------------------------------------------------------------------------------
        void                        Rotate(const float angle, const Vec3& axis);

        //----------------------------------------------------------------------------------------------------
        ///	@brief : Rotate this Entity by a delta rotation.
        //----------------------------------------------------------------------------------------------------
        void                        Rotate(const Rotation& rotation);

        //----------------------------------------------------------------------------------------------------
        ///	@brief : Move this Entity's local location based on the translation. 
        //----------------------------------------------------------------------------------------------------
        void                        Translate(const Vec3& translation);

        //----------------------------------------------------------------------------------------------------
        ///	@brief : Increase the local scale by a delta scalar amount. This multiplies the current scale of each
        ///     axis by the scalar values of deltaScale.
        //----------------------------------------------------------------------------------------------------
        void                        Scale(const float uniformScale);
        
        //----------------------------------------------------------------------------------------------------
        ///	@brief : Increase the local scale by an amount on each axis. This multiplies the current local
        ///     scale by the scale vector.
        //----------------------------------------------------------------------------------------------------
        void                        Scale(const Vec3& scale);

        //----------------------------------------------------------------------------------------------------
        ///	@brief : Sets this Entity's location relative to its parent. 
        //----------------------------------------------------------------------------------------------------
        void                        SetLocalLocation(const Vec3& location);

        //----------------------------------------------------------------------------------------------------
        /// @brief : Set this Entity's orientation relative to its parent. 
        //----------------------------------------------------------------------------------------------------
        void                        SetLocalRotation(const Rotation& rotation);

        //----------------------------------------------------------------------------------------------------
        ///	@brief : Set this Entity's local orientation, from euler angles. The euler angles are expected
        ///     to be in degrees.
        //----------------------------------------------------------------------------------------------------
        void                        SetLocalRotation(const Vec3& eulerAngles);

        //----------------------------------------------------------------------------------------------------
        ///	@brief : Set this Entity's scale relative to its parent. 
        //----------------------------------------------------------------------------------------------------
        void                        SetLocalScale(const Vec3& scale);
        
        //----------------------------------------------------------------------------------------------------
        ///	@brief : Set the Entity's location, orientation and scale relative to its parent. 
        //----------------------------------------------------------------------------------------------------
        void                        SetLocalTransform(const Vec3& location, const Rotation& rotation, const Vec3& scale);

        //----------------------------------------------------------------------------------------------------
        ///	@brief : Set the Entity's location, in world space. 
        //----------------------------------------------------------------------------------------------------
        void                        SetWorldLocation(const Vec3& location);

        //----------------------------------------------------------------------------------------------------
        ///	@brief : Set the Entity's orientation, in world space. 
        //----------------------------------------------------------------------------------------------------
        void                        SetWorldRotation(const Rotation& rotation);

        //----------------------------------------------------------------------------------------------------
        ///	@brief : Set the Entity's scale, in world space. 
        //----------------------------------------------------------------------------------------------------
        void                        SetWorldScale(const Vec3& scale);

        //----------------------------------------------------------------------------------------------------
        ///	@brief : Set the Entity3Ds transform, in world space. 
        //----------------------------------------------------------------------------------------------------
        void                        SetWorldTransform(const Mat44& transform);

        //----------------------------------------------------------------------------------------------------
        ///	@brief : Set the Entity3Ds transform, in world space. 
        //----------------------------------------------------------------------------------------------------
        void                        SetWorldTransform(const Vec3& worldLocation, const Rotation& worldRotation, const Vec3& worldScale);

        //----------------------------------------------------------------------------------------------------
        ///	@brief : Get the Entity's location, in world space. 
        //----------------------------------------------------------------------------------------------------
        Vec3                        GetLocation() const;

        //----------------------------------------------------------------------------------------------------
        ///	@brief : Get the Entity's world orientation. 
        //----------------------------------------------------------------------------------------------------
        Rotation                    GetRotation() const;

        //----------------------------------------------------------------------------------------------------
        ///	@brief : Get the Entity's total world scale. 
        //----------------------------------------------------------------------------------------------------
        Vec3                        GetScale() const;

        //----------------------------------------------------------------------------------------------------
        ///	@brief : Get the Entity's location relative to its parent. 
        //----------------------------------------------------------------------------------------------------
        const Vec3&                 GetLocalLocation() const;
        
        //----------------------------------------------------------------------------------------------------
        ///	@brief : Get the Entity's rotation relative to its parent. 
        //----------------------------------------------------------------------------------------------------
        const Rotation&             GetLocalRotation() const;

        //----------------------------------------------------------------------------------------------------
        ///	@brief : Get the Entity's scale relative to its parent. 
        //----------------------------------------------------------------------------------------------------
        const Vec3&                 GetLocalScale() const;

        //----------------------------------------------------------------------------------------------------
        ///	@brief : Get the Local Transform in its Matrix representation. 
        //----------------------------------------------------------------------------------------------------
        Mat44                       LocalTransformMatrix() const;

        //----------------------------------------------------------------------------------------------------
        /// @brief : Get the World Transformation Matrix of this Entity. 
        //----------------------------------------------------------------------------------------------------
        const Mat44&                GetWorldTransformMatrix() const;

        //----------------------------------------------------------------------------------------------------
        /// @brief : Get the event that is broadcast when this Entity's transform is updated. 
        //----------------------------------------------------------------------------------------------------
        WorldTransformUpdatedEvent& OnWorldTransformUpdated() { return m_onWorldTransformUpdated; }

        //----------------------------------------------------------------------------------------------------
        /// @brief : Get the World that this Entity is in. 
        //----------------------------------------------------------------------------------------------------
        [[nodiscard]] World*        GetWorld() const;

        //----------------------------------------------------------------------------------------------------
        /// @brief : Get the Scene that this Entity is in. 
        //----------------------------------------------------------------------------------------------------
        [[nodiscard]] Scene*        GetScene() const;
        
        //----------------------------------------------------------------------------------------------------
        ///	@brief : If true, then the current World Transform is out of date. This can be caused when the
        ///     Entity hierarchy is changed or this Entity or a parent has moved.
        //----------------------------------------------------------------------------------------------------
        [[nodiscard]] bool          WorldTransformNeedsUpdate() const;

    protected:
        virtual bool                Init() override;
        virtual void                OnParentSet(Entity3D* pParent) override;

        //----------------------------------------------------------------------------------------------------
        ///	@brief : Sets the "WorldTransformNeedsUpdate" flag to true, which will ensure that the next
        ///     call to get or set will update the Transform appropriately, including Children.
        //----------------------------------------------------------------------------------------------------
        void                        MarkWorldTransformDirty();

        //----------------------------------------------------------------------------------------------------
        ///	@brief : Updates our calculated world transformation matrix.
        ///     Called when a Parent's transform has changed or the Parent itself has changed.
        //----------------------------------------------------------------------------------------------------
        void                        UpdateWorldTransform(Entity3D* pParent, const Mat44& localTransform);

        //----------------------------------------------------------------------------------------------------
        ///	@brief : Walk down the tree, updating the world Transforms of all children.
        //----------------------------------------------------------------------------------------------------
        void                        PropagateTransformUpdateToChildren();
        void                        RebuildPhysicsBody();

    private:
        Vec3                        m_location;
        Rotation                    m_rotation;
        Vec3                        m_scale;
        WorldTransformUpdatedEvent  m_onWorldTransformUpdated{};
        Mat44                       m_worldTransformMatrix{};
        bool                        m_worldTransformNeedsUpdate = false;
        BodyID                      m_bodyID{}; // The ID of the Physics Body of this Entity.
    };
}
