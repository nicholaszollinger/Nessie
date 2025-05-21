// BodyInterface.h
#pragma once

#include "BodyID.h"
#include "BodyActivationMode.h"
#include "MotionQuality.h"
#include "MotionType.h"
#include "Core/Memory/StrongPtr.h"
#include "Physics/Collision/CollisionLayer.h"
#include "Physics/Collision/BroadPhase/BroadPhase.h"

namespace nes
{
    class Body;
    struct BodyCreateInfo;
    class BodyLockInterface;
    class BroadPhase;
    class BodyManager;
    class TransformedShape;
    class SubShapeID;
    class Shape;
    class CollisionGroup;
    
    //----------------------------------------------------------------------------------------------------
    //	NOTES:
    //  This is meant to be the public-facing interface with the Application, as to not clog up the
    //  Physics Scene itself. The class also takes in a specific BodyLockInterface, so there can be a
    //  non-locking vs locking interface entirely.
    //
    // [TODO]: Constraints, Physics Materials.
    //
    /// @brief : Interface for performing operations on bodies using BodyIDs.
    /// @note : If you need to do multiple operations on a single body, it is more efficient to lock the
    ///     body once at the beginning, then perform the operations.
    //----------------------------------------------------------------------------------------------------
    class BodyInterface
    {
    public:
        /// Add State handle, used to keep track of a batch of bodies while adding them to the Physics Scene.
        using AddState = void*;
        
    private:
        BodyLockInterface*      m_pBodyLockInterface = nullptr;
        BodyManager*            m_pBodyManager = nullptr;
        BroadPhase*             m_pBroadPhase = nullptr;

        
    public:
        BodyInterface() = default;
        BodyInterface(const BodyInterface&) = delete;
        BodyInterface& operator=(const BodyInterface&) = delete;

        //----------------------------------------------------------------------------------------------------
        /// @brief : Create a rigid body. 
        ///	@returns : Created Body or nullptr when max number of bodies already reached.
        //----------------------------------------------------------------------------------------------------
        Body*                   CreateBody(const BodyCreateInfo& createInfo);

        //----------------------------------------------------------------------------------------------------
        /// @brief : Create a rigid body with a specified ID. This function can be used if a simulation is to run
        ///     sync between clients, or if a simulation needs to be restored exactly.
        ///	@returns : Created Body or nullptr when max number of bodies already reached.
        //----------------------------------------------------------------------------------------------------
        Body*                   CreateBodyWithID(const BodyID& bodyID, const BodyCreateInfo& createInfo);

        //----------------------------------------------------------------------------------------------------
        /// @brief : Advanced use only. Creates a rigid body without specifying an ID.
        ///     This body cannot be added to the physics system until it has been assigned a body ID.
        ///     This can be used to decouple allocation from registering the body. A call to CreateBodyWithoutID()
        ///      followed by AssignBodyID() is equivalent to calling CreateBodyWithID.
        ///	@returns : Created Body.
        //----------------------------------------------------------------------------------------------------
        Body*                   CreateBodyWithoutID(const BodyCreateInfo& createInfo) const;

        //----------------------------------------------------------------------------------------------------
        /// @brief : Advanced use only. Destroy a body previously created with CreateBodyWithoutID() that hasn't
        ///     gotten an ID yet through AssignBodyID(), or a body that has had its body ID unassigned through
        ///     UnassignBodyIDs(). Bodies that have an ID should be destroyed through DestroyBody().
        //----------------------------------------------------------------------------------------------------
        void                    DestroyBodyWithoutID(Body* pBody) const;

        //----------------------------------------------------------------------------------------------------
        /// @brief : Advances use only. Assigns the next available body ID to a body that was created with
        ///     CreateBodyWithoutID(). After this call, the Body can be added to the physics system through
        ///     AddBody().
        /// @returns Returns false if the body already has an ID or there are no available IDs.
        //----------------------------------------------------------------------------------------------------
        bool                    AssignBodyID(Body* pBody);

        //----------------------------------------------------------------------------------------------------
        /// @brief : Advances use only. Assigns the body ID to a body that was created with
        ///     CreateBodyWithoutID(). After this call, the Body can be added to the physics system through
        ///     AddBody().
        /// @returns Returns false if the body already has an ID or if the ID is not valid.
        //----------------------------------------------------------------------------------------------------
        bool                    AssignBodyID(Body* pBody, const BodyID& bodyID);

        //----------------------------------------------------------------------------------------------------
        /// @brief : Advanced use only. @see UnassignBodyIDs().
        //----------------------------------------------------------------------------------------------------
        Body*                   UnassignBodyID(const BodyID& bodyID);

        //----------------------------------------------------------------------------------------------------
        /// @brief : Advanced use only. Removes a number of body IDs from their bodies and returns those body
        ///     pointers in pOutBodies. Before calling this, the bodies should have been removed from the physics
        ///     system through RemoveBody(). The returned bodies can be destroyed through DestroyBodyWithoutID().
        ///     This can be used to decouple deallocation. A call to UnassignBodyIDs() followed by calls to
        ///     DestroyBodyWithoutID() is equivalent to calling DestroyBodies().
        ///	@param pBodyIDs : IDs of the bodies to unassign.
        ///	@param count : Number of ids in the array.
        ///	@param pOutBodies : If not null on input, this will contain a list of body pointers corresponding
        ///     to the pBodyIDs that can be destroyed afterward.
        /// @note : On return, the caller assumes ownership of the pOutBodies data.
        //----------------------------------------------------------------------------------------------------
        void                    UnassignBodyIDs(const BodyID* pBodyIDs, const int count, Body** pOutBodies);

        //----------------------------------------------------------------------------------------------------
        /// @brief : Destroy a body.
        /// @note : Make sure that you removed the body from the physics scene, using RemoveBody(), before
        ///     calling this function.
        //----------------------------------------------------------------------------------------------------
        void                    DestroyBody(const BodyID& bodyID);

        //----------------------------------------------------------------------------------------------------
        /// @brief : Destroy multiple bodies.
        /// @note : Make sure that you removed each body from the physics scene, using RemoveBody(), before
        ///     calling this function.
        //----------------------------------------------------------------------------------------------------
        void                    DestroyBodies(const BodyID* pBodyIDs, const int count);

        //----------------------------------------------------------------------------------------------------
        /// @brief : Add a body to the physics scene.
        ///
        /// If you need to add multiple bodies, use the AddBodiesPrepare()/AddBodiesFinalize() functions.
        /// Adding many bodies, one at a time, results in a really inefficient broadphase until
        /// PhysicsScene::OptimizeBroadphase() is called or when PhysicsScene::Update() rebuilds the tree!
        ///
        /// @note : After adding, to get a body by ID use the BodyLockRead or BodyLockWrite interface!
        //----------------------------------------------------------------------------------------------------
        void                    AddBody(const BodyID& bodyID, const BodyActivationMode activationMode);

        //----------------------------------------------------------------------------------------------------
        /// @brief : Remove a body from the physics scene.
        //----------------------------------------------------------------------------------------------------
        void                    RemoveBody(const BodyID& bodyID);

        //----------------------------------------------------------------------------------------------------
        /// @brief : Check if a body has been added to the physics scene.
        //----------------------------------------------------------------------------------------------------
        bool                    IsAdded(const BodyID& bodyID) const;

        //----------------------------------------------------------------------------------------------------
        /// @brief : Combines CreateBody() and AddBody(). 
        ///	@returns : The created body's ID, or an invalid ID when the max number of bodies have been reached.
        //----------------------------------------------------------------------------------------------------
        BodyID                  CreateAndAddBody(const BodyCreateInfo& createInfo, BodyActivationMode activationMode);

        //----------------------------------------------------------------------------------------------------
        /// @brief : Prepare adding a number of bodies to the Physics Scene. Returns a handle that should be
        ///     used in AddBodiesFinalize/Abort(). This can be done on a background thread without influencing
        ///     the physics scene.
        /// @note : pBodies may be shuffled around by this function and should be kept in that order until
        ///     AddBodiesFinalize/Abort() is called.
        //----------------------------------------------------------------------------------------------------
        AddState                AddBodiesPrepare(BodyID* pBodies, const int count);

        //----------------------------------------------------------------------------------------------------
        /// @brief : Finalizes adding multiple bodies to the Physics Scene - must be supplied with the return value
        ///     of AddBodiesPrepare().
        ///     Please ensure that the pBodies array passed into AddBodiesPrepare() is unmodified and passed
        ///     into this function.
        ///	@param pBodies : Array of bodies that is being added. Must be the same, unmodified array passed into
        ///     AddBodiesPrepare().
        ///	@param count : Number of bodies in the array.
        ///	@param addState : The return value from the AddBodiesPrepare() function.
        ///	@param activationMode : The activation behavior applied to all added bodies.
        //----------------------------------------------------------------------------------------------------
        void                    AddBodiesFinalize(BodyID* pBodies, const int count, AddState addState, BodyActivationMode activationMode);

        //----------------------------------------------------------------------------------------------------
        /// @brief : Abort adding multiple bodies to the Physics Scene - must be supplied with the return result
        ///     of AddBodiesPrepare().
        ///     This can be done on a background thread without influencing the Physics Scene.
        ///     Please ensure that the pBodies array passed into AddBodiesPrepare() is unmodified and passed
        ///     into this function.
        ///	@param pBodies : Array of bodies that is being added. Must be the same, unmodified array passed into
        ///     AddBodiesPrepare().
        ///	@param count : Number of bodies in the array.
        ///	@param addState : The return value from the AddBodiesPrepare() function.
        //----------------------------------------------------------------------------------------------------
        void                    AddBodiesAbort(BodyID* pBodies, const int count, AddState addState);

        //----------------------------------------------------------------------------------------------------
        /// @brief : Remove a number of bodies from the Physics Scene. The order of the pBodies is *not*
        ///     preserved.
        //----------------------------------------------------------------------------------------------------
        void                    RemoveBodies(BodyID* pBodies, const int count);

        //----------------------------------------------------------------------------------------------------
        /// @brief : Activate a body. Active bodies are simulated and detect collisions, inactive bodies are
        ///     asleep and are not checked. Only Dynamic/Kinematic bodies need to be activated.
        //----------------------------------------------------------------------------------------------------
        void                    ActivateBody(const BodyID& bodyID);

        //----------------------------------------------------------------------------------------------------
        /// @brief : Activate multiple bodies. Active bodies are simulated and detect collisions, inactive bodies are
        ///     asleep and are not checked. Only Dynamic/Kinematic bodies need to be activated.
        //----------------------------------------------------------------------------------------------------
        void                    ActivateBodies(const BodyID* pBodies, const int count);

        //----------------------------------------------------------------------------------------------------
        /// @brief : Activate all bodies that intersect the bounding box, and pass the two filters.  
        //----------------------------------------------------------------------------------------------------
        void                    ActivateBodiesInAABox(const AABox& box, const BroadPhaseLayerFilter& broadPhaseFilter, const CollisionLayerFilter& layerFilter);

        //----------------------------------------------------------------------------------------------------
        /// @brief : Deactivate an active body. Active bodies are simulated and detect collisions, inactive bodies are
        ///     asleep and are not checked. Only Dynamic/Kinematic bodies need to be activated.
        //----------------------------------------------------------------------------------------------------
        void                    DeactivateBody(const BodyID& bodyID);

        //----------------------------------------------------------------------------------------------------
        /// @brief : Deactivate multiple active bodies. Active bodies are simulated and detect collisions, inactive bodies are
        ///     asleep and are not checked. Only Dynamic/Kinematic bodies need to be activated.
        //----------------------------------------------------------------------------------------------------
        void                    DeactivateBodies(const BodyID* pBodies, const int count);

        //----------------------------------------------------------------------------------------------------
        /// @brief : Check whether this body is currently simulating (true) or sleeping (false).  
        //----------------------------------------------------------------------------------------------------
        bool                    IsBodyActive(const BodyID& bodyID) const;

        //----------------------------------------------------------------------------------------------------
        /// @brief : Resets the sleep timer for a body. This does not wake up the body if it is sleeping, but allows
        ///     resetting the system that detects when a body is sleeping. 
        //----------------------------------------------------------------------------------------------------
        void                    ResetSleepTimer(const BodyID& bodyID);

        //----------------------------------------------------------------------------------------------------
        /// @brief : Get the shape (collision volume) of the body.
        //----------------------------------------------------------------------------------------------------
        ConstStrongPtr<Shape>   GetShape(const BodyID& bodyID) const;

        //----------------------------------------------------------------------------------------------------
        /// @brief : Set a new shape on the Body. 
        ///	@param bodyID : Body ID of the body that is having its shape changed. 
        ///	@param pShape : The new shape.
        ///	@param updateMassProperties : When true, the mass and inertia tensor is recalculated.
        ///	@param activationMode : Whether to activate the body.
        //----------------------------------------------------------------------------------------------------
        void                    SetShape(const BodyID& bodyID, const Shape* pShape, bool updateMassProperties, BodyActivationMode activationMode) const;

        //----------------------------------------------------------------------------------------------------
        /// @brief : Notify all systems to indicate that a shape has changed (usable for MutableCompoundShapes).
        ///	@param bodyID : Body ID of the body that is having its shape changed. 
        ///	@param previousCenterOfMass : Center of mass of the shape before the alterations.
        ///	@param updateMassProperties : When true, the mass and inertia tensor is recalculated.
        ///	@param activationMode : Whether to activate the body.
        //----------------------------------------------------------------------------------------------------
        void                    NotifyShapeChanged(const BodyID& bodyID, const Vector3& previousCenterOfMass, bool updateMassProperties, BodyActivationMode activationMode) const;

        //----------------------------------------------------------------------------------------------------
        /// @brief : Set the collision layer for a body. A collision layer, combined with a Broadphase Layer,
        ///     determines which bodies can collide with one another.
        //----------------------------------------------------------------------------------------------------
        void                    SetCollisionLayer(const BodyID& bodyID, const CollisionLayer& layer);

        //----------------------------------------------------------------------------------------------------
        /// @brief : Get the collision layer for this body. A collision layer, combined with a Broadphase Layer,
        ///     determines which bodies can collide with one another.
        //----------------------------------------------------------------------------------------------------
        CollisionLayer          GetCollisionLayer(const BodyID& bodyID) const;

        //----------------------------------------------------------------------------------------------------
        /// @brief : Update the position and rotation of the body, and then activate the body. 
        //----------------------------------------------------------------------------------------------------
        void                    SetPositionAndRotation(const BodyID& bodyID, const Vector3& position, const Quat& rotation, BodyActivationMode activationMode);

        //----------------------------------------------------------------------------------------------------
        /// @brief : Will only update the position/rotation and activate the body when the difference is larger
        ///     than a very small number. This avoids updating the broadphase/waking up a body when the
        ///     resulting position/orientation doesn't really change.  
        //----------------------------------------------------------------------------------------------------
        void                    SetPositionAndRotationWhenChanged(const BodyID& bodyID, const Vector3& position, const Quat& rotation, BodyActivationMode activationMode);

        //----------------------------------------------------------------------------------------------------
        /// @brief : Get the current position and rotation of a body.
        //----------------------------------------------------------------------------------------------------
        void                    GetPositionAndRotation(const BodyID& bodyID, Vector3& outPosition, Quat& outRotation) const;

        //----------------------------------------------------------------------------------------------------
        /// @brief : Update the position and activate the body. 
        //----------------------------------------------------------------------------------------------------
        void                    SetPosition(const BodyID& bodyID, const Vector3& position, BodyActivationMode activationMode);

        //----------------------------------------------------------------------------------------------------
        /// @brief : Get the current position of a body.
        //----------------------------------------------------------------------------------------------------
        Vector3                 GetPosition(const BodyID& bodyID) const;

        //----------------------------------------------------------------------------------------------------
        /// @brief : Get the current position of the body's center of mass. 
        //----------------------------------------------------------------------------------------------------
        Vector3                 GetCenterOfMassPosition(const BodyID& bodyID) const;

        //----------------------------------------------------------------------------------------------------
        /// @brief : Update the rotation and activate the body. 
        //----------------------------------------------------------------------------------------------------
        void                    SetRotation(const BodyID& bodyID, const Quat& rotation, BodyActivationMode activationMode);

        //----------------------------------------------------------------------------------------------------
        /// @brief : Get the current rotation of a body.
        //----------------------------------------------------------------------------------------------------
        Quat                    GetRotation(const BodyID& bodyID) const;

        //----------------------------------------------------------------------------------------------------
        /// @brief : Get the world transform of a body. 
        //----------------------------------------------------------------------------------------------------
        Mat4                    GetWorldTransform(const BodyID& bodyID) const;

        //----------------------------------------------------------------------------------------------------
        /// @brief : Get the world transform of a body's center of mass. 
        //----------------------------------------------------------------------------------------------------
        Mat4                    GetCenterOfMassTransform(const BodyID& bodyID) const;

        //----------------------------------------------------------------------------------------------------
        /// @brief : Set the velocity of a Body such that it will be positioned at targetPosition/Rotation in
        ///     deltaTime seconds. This will activate the body if needed.
        //----------------------------------------------------------------------------------------------------
        void                    MoveKinematic(const BodyID& bodyID, const Vector3& targetPosition, const Quat& targetRotation, float deltaTime);

        //----------------------------------------------------------------------------------------------------
        /// @brief : Set the linear and angular velocity of the body. This will activate the body if needed.
        /// @note : The linear velocity is the velocity of the center of mass, which may not coincide with the position
        ///     of your object. To correct for this: VelocityCOM = Velocity - AngularVelocity * ShapeCOM.
        //----------------------------------------------------------------------------------------------------
        void                    SetLinearAndAngularVelocity(const BodyID& bodyID, const Vector3& linearVelocity, const Vector3& angularVelocity);

        //----------------------------------------------------------------------------------------------------
        /// @brief : Get the linear and angular velocity of the body.
        /// @note : The linear velocity is the velocity of the center of mass, which may not coincide with the position
        ///     of your object. To correct for this: VelocityCOM = Velocity - AngularVelocity * ShapeCOM.
        //----------------------------------------------------------------------------------------------------
        void                    GetLinearAndAngularVelocity(const BodyID& bodyID, Vector3& outLinearVelocity, Vector3& outAngularVelocity) const;

        //----------------------------------------------------------------------------------------------------
        /// @brief : Set the linear velocity of the body.
        /// @note : The linear velocity is the velocity of the center of mass, which may not coincide with the position
        ///     of your object. To correct for this: VelocityCOM = Velocity - AngularVelocity * ShapeCOM.
        //----------------------------------------------------------------------------------------------------
        void                    SetLinearVelocity(const BodyID& bodyID, const Vector3& linearVelocity);

        //----------------------------------------------------------------------------------------------------
        /// @brief : Get the linear velocity of the body.
        /// @note : The linear velocity is the velocity of the center of mass, which may not coincide with the position
        ///     of your object. To correct for this: VelocityCOM = Velocity - AngularVelocity * ShapeCOM.
        //----------------------------------------------------------------------------------------------------
        Vector3                 GetLinearVelocity(const BodyID& bodyID) const;

        //----------------------------------------------------------------------------------------------------
        /// @brief : Set the angular velocity of the body. 
        //----------------------------------------------------------------------------------------------------
        void                    SetAngularVelocity(const BodyID& bodyID, const Vector3& angularVelocity);

        //----------------------------------------------------------------------------------------------------
        /// @brief : Add velocity to the current velocity of the body.
        //----------------------------------------------------------------------------------------------------
        void                    AddLinearVelocity(const BodyID& bodyID, const Vector3& deltaLinearVelocity);

        //----------------------------------------------------------------------------------------------------
        /// @brief : Add linear and angular velocity to the current velocities of the body.
        //----------------------------------------------------------------------------------------------------
        void                    AddLinearAndAngularVelocity(const BodyID& bodyID, const Vector3& deltaLinearVelocity, const Vector3& deltaAngularVelocity);

        //----------------------------------------------------------------------------------------------------
        /// @brief : Set the angular velocity of the body.
        //----------------------------------------------------------------------------------------------------
        Vector3                 GetAngularVelocity(const BodyID& bodyID) const;

        //----------------------------------------------------------------------------------------------------
        /// @brief : Get the velocity of a point (in world space, on the surface of the body) of the body.
        //----------------------------------------------------------------------------------------------------
        Vector3                 GetPointVelocity(const BodyID& bodyID, const Vector3& point) const;

        //----------------------------------------------------------------------------------------------------
        /// @brief : Set the complete motion state of the body. 
        /// @note : The linear velocity is the velocity of the center of mass, which may not coincide with the position
        ///     of your object. To correct for this: VelocityCOM = Velocity - AngularVelocity * ShapeCOM.
        //----------------------------------------------------------------------------------------------------
        void                    SetPositionAndRotationAndVelocity(const BodyID& bodyID, const Vector3& position, const Quat& rotation, const Vector3& linearVelocity, const Vector3& angularVelocity);

        //----------------------------------------------------------------------------------------------------
        /// @brief : Add force (unit: N) at the center of mass for the next time step. This will be reset after
        ///     the next call to PhysicsScene::Update().
        //----------------------------------------------------------------------------------------------------
        void                    AddForce(const BodyID& bodyID, const Vector3& force, BodyActivationMode activationMode = BodyActivationMode::Activate);

        //----------------------------------------------------------------------------------------------------
        /// @brief : Add Force, applied at a world space point.
        //----------------------------------------------------------------------------------------------------
        void                    AddForce(const BodyID& bodyID, const Vector3& force, const Vector3& point, BodyActivationMode activationMode = BodyActivationMode::Activate);

        //----------------------------------------------------------------------------------------------------
        /// @brief : Add torque (unit: N m) for the next time step, will be reset after the next call to
        ///     PhysicsScene::Update().
        //----------------------------------------------------------------------------------------------------
        void                    AddTorque(const BodyID& bodyID, const Vector3& torque, BodyActivationMode activationMode = BodyActivationMode::Activate);

        //----------------------------------------------------------------------------------------------------
        /// @brief : A combination of AddForce() and AddTorque().
        //----------------------------------------------------------------------------------------------------
        void                    AddForceAndTorque(const BodyID& bodyID, const Vector3& force, const Vector3& torque, BodyActivationMode activationMode = BodyActivationMode::Activate);

        //----------------------------------------------------------------------------------------------------
        /// @brief : Add an impulse to the center of mass (unit: kg m/s).
        //----------------------------------------------------------------------------------------------------
        void                    AddImpulse(const BodyID& bodyID, const Vector3& impulse);

        //----------------------------------------------------------------------------------------------------
        /// @brief : Add an impulse to a point in world space (unit: kg m/s).
        //----------------------------------------------------------------------------------------------------
        void                    AddImpulse(const BodyID& bodyID, const Vector3& impulse, const Vector3& point);

        //----------------------------------------------------------------------------------------------------
        /// @brief : Add angular impulse to this body in world space (unit: N m s).
        //----------------------------------------------------------------------------------------------------
        void                    AddAngularImpulse(const BodyID& bodyID, const Vector3& angularImpulse);

        //----------------------------------------------------------------------------------------------------
        /// @brief : Set the motion-type of a body, and activate it if desired. 
        //----------------------------------------------------------------------------------------------------
        void                    SetMotionType(const BodyID& bodyID, BodyMotionType motionType, BodyActivationMode activationMode);

        //----------------------------------------------------------------------------------------------------
        /// @brief : Get the motion-type of a body. If the ID is invalid, this will return Kinematic as a default. 
        //----------------------------------------------------------------------------------------------------
        BodyMotionType          GetMotionType(const BodyID& bodyID) const;

        //----------------------------------------------------------------------------------------------------
        /// @brief : Set the motion quality of a body. 
        //----------------------------------------------------------------------------------------------------
        void                    SetMotionQuality(const BodyID& bodyID, BodyMotionQuality quality);

        //----------------------------------------------------------------------------------------------------
        /// @brief : Get the body's motion quality.
        //----------------------------------------------------------------------------------------------------
        BodyMotionQuality       GetMotionQuality(const BodyID& bodyID) const;

        //----------------------------------------------------------------------------------------------------
        /// @brief : Get the inverse inertia tensor in world space. 
        //----------------------------------------------------------------------------------------------------
        Mat4                    GetInverseInertia(const BodyID& bodyID) const;

        //----------------------------------------------------------------------------------------------------
        /// @brief : Set a body's restitution. Restitution is usually between [0, 1], where 0 = completely non-elastic collision response
        ///     and 1 = completely elastic collision response.
        //----------------------------------------------------------------------------------------------------
        void                    SetRestitution(const BodyID& bodyID, const float restitution);

        //----------------------------------------------------------------------------------------------------
        /// @brief : Get a body's restitution. Restitution is usually between [0, 1], where 0 = completely non-elastic collision response
        ///     and 1 = completely elastic collision response.
        //----------------------------------------------------------------------------------------------------
        float                   GetRestitution(const BodyID& bodyID) const;

        //----------------------------------------------------------------------------------------------------
        /// @brief : Set a body's friction. Friction is usually between [0, 1], where 0 = no friction and 1 = friction
        ///     force equals the force that presses the two bodies together.
        //----------------------------------------------------------------------------------------------------
        void                    SetFriction(const BodyID& bodyID, const float friction);

        //----------------------------------------------------------------------------------------------------
        /// @brief : Get a body's friction. Friction is usually between [0, 1], where 0 = no friction and 1 = friction
        ///     force equals the force that presses the two bodies together. 
        //----------------------------------------------------------------------------------------------------
        float                   GetFriction(const BodyID& bodyID) const;

        //----------------------------------------------------------------------------------------------------
        /// @brief : Set the value to scale gravity by. (1 = normal gravity, 0 = no gravity).
        //----------------------------------------------------------------------------------------------------
        void                    SetGravityScale(const BodyID& bodyID, const float gravityScale);

        //----------------------------------------------------------------------------------------------------
        /// @brief : Get the value to scale gravity by. (1 = normal gravity, 0 = no gravity).
        //----------------------------------------------------------------------------------------------------
        float                   GetGravityScale(const BodyID& bodyID) const;

        //----------------------------------------------------------------------------------------------------
        /// @brief : If PhysicsSettings::m_useManifoldReduction is true, this allows turning off manifold reduction for this specific body.
        /// Manifold reduction by default will combine contacts with similar normals that come from different SubShapeIDs (e.g. different triangles in a mesh shape or different compound shapes).
        /// If the application requires tracking exactly which SubShapeIDs are in contact, you can turn off manifold reduction. Note that this comes at a performance cost.
        //----------------------------------------------------------------------------------------------------
        void                    SetUseManifoldReduction(const BodyID& bodyID, bool useManifoldReduction);

        //----------------------------------------------------------------------------------------------------
        /// @brief : If PhysicsSettings::m_useManifoldReduction is true, this allows turning off manifold reduction for this specific body.
        /// Manifold reduction by default will combine contacts with similar normals that come from different SubShapeIDs (e.g. different triangles in a mesh shape or different compound shapes).
        /// If the application requires tracking exactly which SubShapeIDs are in contact, you can turn off manifold reduction. Note that this comes at a performance cost.
        //----------------------------------------------------------------------------------------------------
        bool                    GetUseManifoldReduction(const BodyID& bodyID) const;

        //----------------------------------------------------------------------------------------------------
        /// @brief : Set the collision group for this Body. 
        //----------------------------------------------------------------------------------------------------
        void                    SetCollisionGroup(const BodyID& bodyID, const CollisionGroup& collisionGroup);

        //----------------------------------------------------------------------------------------------------
        /// @brief : Get the collision group for this Body. 
        //----------------------------------------------------------------------------------------------------
        const CollisionGroup&   GetCollisionGroup(const BodyID& bodyID) const;

        //----------------------------------------------------------------------------------------------------
        /// @brief : Get transform and shape for this body, used to perform collision detection.
        //----------------------------------------------------------------------------------------------------
        TransformedShape        GetTransformedShape(const BodyID& bodyID) const;

        //----------------------------------------------------------------------------------------------------
        /// @brief : Get the user data for this Body. The user data can be anything that you want.
        //----------------------------------------------------------------------------------------------------
        uint64_t                GetUserData(const BodyID& bodyID) const;

        //----------------------------------------------------------------------------------------------------
        /// @brief : Set the user data for this Body. The user data can be anything that you want.
        //----------------------------------------------------------------------------------------------------
        void                    SetUserData(const BodyID& bodyID, uint64_t userData);
        
        //----------------------------------------------------------------------------------------------------
        /// @brief : Initialize the interface (should only be called by the Physics System). 
        //----------------------------------------------------------------------------------------------------
        void                    Internal_Init(BodyLockInterface& lockInterface, BodyManager& bodyManager, BroadPhase& broadPhase);

        //----------------------------------------------------------------------------------------------------
        /// @brief : Set the Body::Flags::InvalidateContactCache flag for the specified body. This means that
        ///     the collision cache is invalid for any body pair involving that body until the next physics step.
        //----------------------------------------------------------------------------------------------------
        void                    Internal_InvalidateContactCache(const BodyID& bodyID);

    private:
        //----------------------------------------------------------------------------------------------------
        /// @brief : Helper function to activate a single body.
        //----------------------------------------------------------------------------------------------------
        NES_INLINE void         Internal_ActivateBody(Body& body) const;
    };
}
