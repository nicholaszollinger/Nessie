// ContactListener.h
#pragma once
#include "Core/StaticArray.h"
#include "Math/Real.h"
#include "Physics/Collision/Shapes/SubShapeIDPair.h"

namespace nes
{
    class Body;
    struct CollideShapeResult;

    /// Array of Contact Points
    using ContactPoints = StaticArray<Vec3, 64>;

    //----------------------------------------------------------------------------------------------------
    /// @brief : A Contact Manifold describes the contact surface between two bodies.  
    //----------------------------------------------------------------------------------------------------
    struct ContactManifold
    {
        RVec3               m_baseOffset;                       /// Offset to which all the contact points are relative to.
        Vec3                m_worldSpaceNormal;                 /// Normal for this manifold. It is the direction along which to move Body 2 out of collision along the shortest path.
        float               m_penetrationDepth;                 /// Penetration depth (move shape 2 by this distance to resolve the collision). If the value is negative, this is a speculative contact point and may not result in a velocity change as during solving the bodies may not collide.
        SubShapeID          m_subShapeID1;                      /// First of the 2 sub shapes that formed this manifold (note that when multiple manifolds are combined because they're coplanar, we lose some information here because we only keep track of one sub shape pair that we encounter, see description at Body::SetUseManifoldReduction)
        SubShapeID          m_subShapeID2;                      /// First of the 2 sub shapes that formed this manifold (note that when multiple manifolds are combined because they're coplanar, we lose some information here because we only keep track of one sub shape pair that we encounter, see description at Body::SetUseManifoldReduction)
        ContactPoints       m_relativeContactPointsOn1;         /// Contact points on the surface of sub shape 1 relative to the m_baseOffset.
        ContactPoints       m_relativeContactPointsOn2;         /// Contact points on the surface of sub shape 2 relative to the m_baseOffset.
                
        //----------------------------------------------------------------------------------------------------
        /// @brief : Returns a contact manifold with the two sub shapes swapped. 
        //----------------------------------------------------------------------------------------------------
        ContactManifold     SwapShapes() const      { return { m_baseOffset, -m_worldSpaceNormal, m_penetrationDepth, m_subShapeID2, m_subShapeID1, m_relativeContactPointsOn2, m_relativeContactPointsOn1}; }

        //----------------------------------------------------------------------------------------------------
        /// @brief : Access the world space contact positions on sub shape 1. 
        //----------------------------------------------------------------------------------------------------
        inline RVec3        GetWorldSpaceContactPointOn1(const uint index) const { return m_baseOffset + m_relativeContactPointsOn1[index]; }

        //----------------------------------------------------------------------------------------------------
        /// @brief : Access the world space contact positions on sub shape 2. 
        //----------------------------------------------------------------------------------------------------
        inline RVec3        GetWorldSpaceContactPointOn2(const uint index) const { return m_baseOffset + m_relativeContactPointsOn2[index]; }
    };

    //----------------------------------------------------------------------------------------------------
    /// @brief : When a contact point is added or persisted, the callback gets a chance to override certain
    ///     properties of the contact constraint. The values are filled in with their defaults by the system,
    ///     so the callback doesn't need to modify anything, but it can if it wants to.
    //----------------------------------------------------------------------------------------------------
    struct ContactSettings
    {
        float               m_combinedFriction;                 /// Combined friction for the body pair (see: PhysicsScene::SetCombinedFriction).
        float               m_combinedRestitution;              /// Combined resitution for the body pair (see: PhysicsScene::SetCombinedRestitution).
        float               m_inverseMassScale1 = 1.f;          /// Scale factor for the inverse mass of body 1 (0 = infinite mass, 1 = use original mass, 2 = body has half the mass). For the same contact pair, you should strive to keep the value the same over time.
        float               m_inverseInertiaScale1 = 1.f;       /// Scale factor for the inverse inertia of body 1 (usually the same as m_inverseMass1).
        float               m_inverseMassScale2 = 1.f;          /// Scale factor for the inverse mass of body 2 (0 = infinite mass, 1 = use original mass, 2 = body has half the mass). For the same contact pair, you should strive to keep the value the same over time.
        float               m_inverseInertiaScale2 = 1.f;       /// Scale factor for the inverse inertia of body 2 (usually the same as m_inverseMass2).
        bool                m_isSensor;                         /// If the contact point should be treated as a sensor vs. body contact (no collision response). 
        Vec3                m_relativeLinearSurfaceVelocity = Vec3::Zero();  /// Relative linear surface velocity between the bodies (world space velocity of body 2 - world space surface velocity of body 1). This can be used to create a conveyor belt effect.
        Vec3                m_relativeAngularSurfaceVelocity = Vec3::Zero(); /// Relative angular surface velocity between the bodies (world space angular surface velocity of body 2 - world space angular surface velocity of body 1). Note that this angular velocity is relative to the center of mass of body 1, so if you want it relative to body 2's center of mass you need to add body 2 angular velocity x (body 1 world space center of mass - body 2 world space center of mass) to m_relativeLinearSurfaceVelocity.
    };

    //----------------------------------------------------------------------------------------------------
    /// @brief : Return value for the OnContactValidate callback. Determines if the contact is being processed
    ///     or not. Results are ordered so that the strongest contact acceptance has the lowest value and the strongest
    ///     reject has the highest number (which allows for easy combining of results.
    //----------------------------------------------------------------------------------------------------
    enum class EValidateContactResult : uint8
    {
        AcceptAllContactsForThisBodyPair,                       /// Accept this and any further contact points for this body pair.
        AcceptContact,                                          /// Accept this contact only (and continue calling this callback for every contact manifold for the same body pair). 
        RejectContact,                                          /// Reject this contact only (and continue calling this callback for every contact manifold for the same body pair). 
        RejectAllContactsForThisBodyPair,                       /// Reject this and any further contact points for this body pair.
    };

    //----------------------------------------------------------------------------------------------------
    /// @brief : A listener class that receives collision contact events. It can be registered through
    ///     PhysicsScene::SetContactListener. Only a single contact listener can be registered. A common
    ///     pattern is to create a contact listener that casts Body::GetUserData() to a game object and then
    ///     forwards the call to a handler specific for that game object.
    ///
    /// @note : Contact listener callbacks are called from multiple threads at the same time when all bodies
    ///     are locked. This means that you cannot use PhysicsScene::GetBodyInterface / PhysicsScene::GetBodyLockInterface
    ///     but must use PhysicsScene::GetBodyInterfaceNoLock / PhysicsScene::GetBodyLockInterfaceNoLock.
    ///     If you are using a locking interface, the simulation will deadlock! You're only allowed to read from
    ///     the bodies, and you can't change the physics state.
    /// @note : During OnContactRemoved you cannot access the bodies at all, see the comments at that function.
    //----------------------------------------------------------------------------------------------------
    class ContactListener
    {
    public:
        virtual                         ~ContactListener() = default;

        //----------------------------------------------------------------------------------------------------
        /// @brief : Called after detecting a collision between a body pair, but before calling OnContactAdded and
        /// before adding the contact constraint. If the function rejects the contact, the contact will not be
        /// processed by the simulation.
        ///
        /// This is a rather expensive time to reject a contact point since a lot of the collision detection
        /// has happened already. Make sure you filter out the majority of undesired body pairs through the
        /// CollisionLayerPairFilter that is registered to the PhysicsScene.
        ///
        /// This function may not be called again on the next update if a contact persists, and no new contact
        /// pairs between sub shapes are found.
        ///
        /// @note : This function is called when all bodies are locked, so don't use any locking functions!
        ///     See ContactListener class description.
        ///
        ///	@param body1 : Body 1 will have a motion type that is larget or equal to body 2's motion type.
        ///     (Order from larget to small: Dynamic -> Kinematic -> Static). When motion types are equal,
        ///     they are ordered by BodyID.
        ///	@param body2 : Second body involved in the contact. See Body1 description for ordering info.
        ///	@param baseOffset : Offset to relative space of the collision result.
        ///	@param collisionResult : The collision result is reported relative to the 'baseOffset'.
        //----------------------------------------------------------------------------------------------------
        virtual EValidateContactResult  OnContactValidate([[maybe_unused]] const Body& body1, [[maybe_unused]] const Body& body2, [[maybe_unused]] const RVec3 baseOffset, [[maybe_unused]] const CollideShapeResult& collisionResult) { return EValidateContactResult::AcceptAllContactsForThisBodyPair; }

        //----------------------------------------------------------------------------------------------------
        /// @brief : Called whenever a new contact point is detected.
        ///
        /// Only active bodies will report contacts; as soon as a body goes to sleep, the contacts between
        /// that body and all other bodies will receive an OnContactRemoved callback. When this happens,
        /// Body::IsActive() will return false during the callback.
        ///
        /// When contacts are added, the constraint solver has not run yet, so the collision impulse is
        /// unknown at that point. The velocities of body1 and body2 are the velocities before the contact
        /// has been resolved, so you can use this to estimate the collision impulse to determine the volume
        /// of an impact sound to play, for example.
        ///
        /// @note : This function is called when all bodies are locked, so don't use any locking functions!
        /// See ContactListener class description.
        ///
        ///	@param body1 : Body 1 and 2 will be sorted such that body 1 ID < body 2 ID, so body 1 may not be dynamic.
        ///	@param body2 : Second body involved in the contact. See Body1 description for ordering info.
        ///	@param manifold : Description of the contact surface between the bodies.
        ///	@param ioSettings : Settings of the contact constraint that you can modify if you want to. 
        //----------------------------------------------------------------------------------------------------
        virtual void                    OnContactAdded([[maybe_unused]] const Body& body1, [[maybe_unused]] const Body& body2, [[maybe_unused]] const ContactManifold& manifold, [[maybe_unused]] ContactSettings& ioSettings) { /* Do nothing. */ }

        //----------------------------------------------------------------------------------------------------
        /// @brief : Called whenever a contact is detected that was also detected last update.
        ///
        /// If the shape structure of a body changes between simulation steps (e.g. by adding/removing
        /// a child shape of a compound shape), it is possible that the same sub shape ID used to identify
        /// the removed child shape is now reused for a different child shape. The physics scene cannot detect
        /// this, so you may send a 'contact persisted' callback even though the contact is now on a different
        /// child shape. You can detect this by keeping the old shape (before adding/removing a part) around until
        /// the next PhysicsScene::Update (when the OnContactPersisted callbacks are triggered). Then, you can
        /// check if the sub shape ID against both the old and new shape to see if they still refer to the same
        /// child shape.
        /// 
        /// @note : This function is called when all bodies are locked, so don't use any locking functions!
        /// See ContactListener class description.
        ///
        ///	@param body1 : Body 1 and 2 will be sorted such that body 1 ID < body 2 ID, so body 1 may not be dynamic.
        ///	@param body2 : Second body involved in the contact. See Body1 description for ordering info.
        ///	@param manifold : Description of the contact surface between the bodies.
        ///	@param ioSettings : Settings of the contact constraint that you can modify if you want to. 
        //----------------------------------------------------------------------------------------------------
        virtual void                    OnContactPersisted([[maybe_unused]] const Body& body1, [[maybe_unused]] const Body& body2, [[maybe_unused]] const ContactManifold& manifold, [[maybe_unused]] ContactSettings& ioSettings) { /* Do nothing. */}

        //----------------------------------------------------------------------------------------------------
        /// @brief : Called whenever a contact was detected last update but not detected anymore.
        ///
        /// You cannot access the bodies at the time of this callback because:
        /// - All bodies are locked at the time of this callback.
        /// - Some properties of the bodies are being modified from another thread at the same time.
        /// - The body may have been removed and destroyed (you'll receive an OnContactRemoved callback in
        ///   the PhysicsScene::Update after the body has been removed).
        ///
        /// Cache what you need in the OnContactAdded and OnContactPersisted callbacks and store it in a
        /// separate structure to use during this callback. Alternatively, you could record that the contact
        /// was removed and process it after PhysicsSystem::Update.
        ///
        /// Body 1 and 2 will be sorted such that body 1 ID < body 2 ID, so body 1 may not be dynamic.
        ///
        ///	@param subShapePair : The sub shape IDs where created in the previous simulation step; so if the structure of a
        /// shape changes (by adding/removing a child shape of a compound shape), the sub shape ID may not be valid
        /// or may not point to the same sub shape anymore.
        /// If you want to know if this is the last contact between the two bodies, use PhysicsScene::WereBodiesInContact.
        //----------------------------------------------------------------------------------------------------
        virtual void                    OnContactRemoved([[maybe_unused]] const SubShapeIDPair& subShapePair) { /* Do nothing. */ }
    };
}