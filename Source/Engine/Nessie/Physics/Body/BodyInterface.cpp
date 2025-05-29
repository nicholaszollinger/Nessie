// BodyInterface.cpp
#include "BodyInterface.h"

#include "Body.h"
#include "BodyLock.h"
#include "BodyManager.h"

namespace nes
{
    Body* BodyInterface::CreateBody(const BodyCreateInfo& createInfo)
    {
        Body* pBody = m_pBodyManager->AllocateBody(createInfo);
        if (!m_pBodyManager->AddBody(pBody))
        {
            m_pBodyManager->FreeBody(pBody);
            return nullptr;
        }
        return pBody;
    }

    Body* BodyInterface::CreateBodyWithID(const BodyID& bodyID, const BodyCreateInfo& createInfo)
    {
        Body* pBody = m_pBodyManager->AllocateBody(createInfo);
        if (!m_pBodyManager->AddBodyWithCustomID(pBody, bodyID))
        {
            m_pBodyManager->FreeBody(pBody);
            return nullptr;
        }
        return pBody;
    }

    Body* BodyInterface::CreateBodyWithoutID(const BodyCreateInfo& createInfo) const
    {
        return m_pBodyManager->AllocateBody(createInfo);
    }

    void BodyInterface::DestroyBodyWithoutID(Body* pBody) const
    {
        m_pBodyManager->FreeBody(pBody);
    }

    bool BodyInterface::AssignBodyID(Body* pBody)
    {
        return m_pBodyManager->AddBody(pBody);
    }

    bool BodyInterface::AssignBodyID(Body* pBody, const BodyID& bodyID)
    {
        return m_pBodyManager->AddBodyWithCustomID(pBody, bodyID);
    }

    Body* BodyInterface::UnassignBodyID(const BodyID& bodyID)
    {
        Body* pBody = nullptr;
        m_pBodyManager->RemoveBodies(&bodyID, 1, &pBody);
        return pBody;
    }

    void BodyInterface::UnassignBodyIDs(const BodyID* pBodyIDs, const int count, Body** pOutBodies)
    {
        m_pBodyManager->RemoveBodies(pBodyIDs, count, pOutBodies);
    }

    void BodyInterface::DestroyBody(const BodyID& bodyID)
    {
        m_pBodyManager->DestroyBodies(&bodyID, 1);
    }

    void BodyInterface::DestroyBodies(const BodyID* pBodyIDs, const int count)
    {
        m_pBodyManager->DestroyBodies(pBodyIDs, count);
    }

    void BodyInterface::AddBody(const BodyID& bodyID, const EBodyActivationMode activationMode)
    {
        BodyLockWrite lock(*m_pBodyLockInterface, bodyID);
        if (lock.Succeeded())
        {
            const Body& body = lock.GetBody();

            // Add to the broadphase
            BodyID id = bodyID;
            const BroadPhase::AddState addState = m_pBroadPhase->AddBodiesPrepare(&id, 1);
            m_pBroadPhase->AddBodiesFinalize(&id, 1, addState);

            // Optionally activate
            if (activationMode == EBodyActivationMode::Activate && !body.IsStatic())
                m_pBodyManager->ActivateBodies(&bodyID, 1);
        }
    }

    void BodyInterface::RemoveBody(const BodyID& bodyID)
    {
        BodyLockWrite lock(*m_pBodyLockInterface, bodyID);
        if (lock.Succeeded())
        {
            const Body& body = lock.GetBody();

            // Deactivate Body
            if (body.IsActive())
                m_pBodyManager->DeactivateBodies(&bodyID, 1);

            // Remove from the Broad phase
            BodyID id = bodyID;
            m_pBroadPhase->RemoveBodies(&id, 1);
        }
    }

    bool BodyInterface::IsAdded(const BodyID& bodyID) const
    {
        BodyLockRead lock(*m_pBodyLockInterface, bodyID);
        return lock.SucceededAndIsInBroadPhase();
    }

    BodyID BodyInterface::CreateAndAddBody(const BodyCreateInfo& createInfo, EBodyActivationMode activationMode)
    {
        const Body* pBody = CreateBody(createInfo);
        if (pBody == nullptr)
            return BodyID();

        AddBody(pBody->GetID(), activationMode);
        return pBody->GetID();
    }

    BodyInterface::AddState BodyInterface::AddBodiesPrepare(BodyID* pBodies, const int count)
    {
        return m_pBroadPhase->AddBodiesPrepare(pBodies, count);
    }

    void BodyInterface::AddBodiesFinalize(BodyID* pBodies, const int count, AddState addState,
        EBodyActivationMode activationMode)
    {
        BodyLockMultiWrite lock(*m_pBodyLockInterface, pBodies, count);
        
        // Add to broadphase
        m_pBroadPhase->AddBodiesFinalize(pBodies, count, addState);

        // Optionally activate bodies
        if (activationMode == EBodyActivationMode::Activate)
            m_pBodyManager->ActivateBodies(pBodies, count);
    }

    void BodyInterface::AddBodiesAbort(BodyID* pBodies, const int count, AddState addState)
    {
        m_pBroadPhase->AddBodiesAbort(pBodies, count, addState);
    }

    void BodyInterface::RemoveBodies(BodyID* pBodies, const int count)
    {
        BodyLockMultiWrite lock(*m_pBodyLockInterface, pBodies, count);

        // Deactivate bodies
        m_pBodyManager->DeactivateBodies(pBodies, count);

        // Remove from the broad phase
        m_pBroadPhase->RemoveBodies(pBodies, count);
    }

    void BodyInterface::ActivateBody(const BodyID& bodyID)
    {
        BodyLockWrite lock(*m_pBodyLockInterface, bodyID);
        if (lock.Succeeded())
        {
            Body& body = lock.GetBody();
            Internal_ActivateBody(body);
        }
    }

    void BodyInterface::ActivateBodies(const BodyID* pBodies, const int count)
    {
        BodyLockMultiWrite lock(*m_pBodyLockInterface, pBodies, count);
        m_pBodyManager->ActivateBodies(pBodies, count);
    }

    void BodyInterface::ActivateBodiesInAABox(const AABox& box, const BroadPhaseLayerFilter& broadPhaseFilter, const CollisionLayerFilter& layerFilter)
    {
        // Collect all bodies within the box
        AllHitCollisionCollector<CollideShapeBodyCollector> collector;
        m_pBroadPhase->CollideAABox(box, collector, broadPhaseFilter, layerFilter);

        // Activate the bodies.
        ActivateBodies(collector.m_hits.data(), static_cast<int>(collector.m_hits.size()));
    }

    void BodyInterface::DeactivateBody(const BodyID& bodyID)
    {
        BodyLockWrite lock(*m_pBodyLockInterface, bodyID);
        if (lock.Succeeded())
        {
            const Body& body = lock.GetBody();
            
            if (body.IsActive())
                m_pBodyManager->DeactivateBodies(&bodyID, 1);
        }
    }

    void BodyInterface::DeactivateBodies(const BodyID* pBodies, const int count)
    {
        BodyLockMultiWrite lock(*m_pBodyLockInterface, pBodies, count);
        m_pBodyManager->DeactivateBodies(pBodies, count);
    }

    bool BodyInterface::IsBodyActive(const BodyID& bodyID) const
    {
        BodyLockRead lock(*m_pBodyLockInterface, bodyID);
        return lock.Succeeded() && lock.GetBody().IsActive();
    }

    void BodyInterface::ResetSleepTimer(const BodyID& bodyID)
    {
        BodyLockWrite lock(*m_pBodyLockInterface, bodyID);
        if (lock.Succeeded())
            lock.GetBody().ResetSleepTimer();
    }

    ConstStrongPtr<Shape> BodyInterface::GetShape(const BodyID& bodyID) const
    {
        ConstStrongPtr<Shape> pShape;
        
        BodyLockRead lock(*m_pBodyLockInterface, bodyID);
        if (lock.Succeeded())
            pShape = lock.GetBody().GetShape();
        
        return pShape;
    }

    void BodyInterface::SetShape(const BodyID& bodyID, const Shape* pShape, bool updateMassProperties,
        EBodyActivationMode activationMode) const
    {
        BodyLockWrite lock(*m_pBodyLockInterface, bodyID);

        if (lock.Succeeded())
        {
            Body& body = lock.GetBody();

            // Check if the shape actually changed
            if (body.GetShape() != pShape)
            {
                // Update the shape
                body.Internal_SetShape(pShape, updateMassProperties);

                // Flag collision cache as invalid for this body
                m_pBodyManager->InvalidateContactCacheForBody(body);

                // Notify broadphase of the change
                if (body.IsInBroadPhase())
                {
                    BodyID id = body.GetID();
                    m_pBroadPhase->NotifyBodiesAABBChanged(&id, 1);
                    
                    // Optionally activate the body
                    if (activationMode == EBodyActivationMode::Activate && !body.IsStatic())
                        Internal_ActivateBody(body);
                }
            }
        }
    }

    void BodyInterface::NotifyShapeChanged(const BodyID& bodyID, const Vector3& previousCenterOfMass,
        bool updateMassProperties, EBodyActivationMode activationMode) const
    {
        BodyLockWrite lock(*m_pBodyLockInterface, bodyID);

        if (lock.Succeeded())
        {
            Body& body = lock.GetBody();

            // Update center of mass, mass and inertia
            body.Internal_UpdateCenterOfMass(previousCenterOfMass, updateMassProperties);

            // Recalculate bounding box
            body.Internal_CalculateWorldSpaceBounds();

            // Flag collision cache invalid for this body
            m_pBodyManager->InvalidateContactCacheForBody(body);

            // Notify Broadphase of the change
            if (body.IsInBroadPhase())
            {
                BodyID id = body.GetID();
                m_pBroadPhase->NotifyBodiesAABBChanged(&id, 1);

                // Optionally activate the body
                if (activationMode == EBodyActivationMode::Activate && !body.IsStatic())
                    Internal_ActivateBody(body);
            }
        }
    }

    void BodyInterface::SetCollisionLayer(const BodyID& bodyID, const CollisionLayer& layer)
    {
        BodyLockWrite lock(*m_pBodyLockInterface, bodyID);
        if (lock.Succeeded())
        {
            Body& body = lock.GetBody();

            // Check if the layer actually changed. Updating the broadphase is expensive
            if (body.GetCollisionLayer() != layer)
            {
                // Update the layer on the body
                m_pBodyManager->Internal_SetBodyCollisionLayer(body, layer);

                // Notify broadphase
                if (body.IsInBroadPhase())
                {
                    BodyID id = body.GetID();
                    m_pBroadPhase->NotifyBodiesLayerChanged(&id, 1);
                }
            }
        }
    }

    CollisionLayer BodyInterface::GetCollisionLayer(const BodyID& bodyID) const
    {
        BodyLockRead lock(*m_pBodyLockInterface, bodyID);
        if (lock.Succeeded())
            return lock.GetBody().GetCollisionLayer();

        return kInvalidCollisionLayer;
    }

    void BodyInterface::SetPositionAndRotation(const BodyID& bodyID, const Vector3& position, const Quat& rotation,
        EBodyActivationMode activationMode)
    {
        BodyLockWrite lock(*m_pBodyLockInterface, bodyID);
        if (lock.Succeeded())
        {
            Body& body = lock.GetBody();

            // Update the position and rotation
            body.Internal_SetPositionAndRotation(position, rotation);
            
            // Notify Broadphase of the change
            if (body.IsInBroadPhase())
            {
                BodyID id = body.GetID();
                m_pBroadPhase->NotifyBodiesAABBChanged(&id, 1);

                // Optionally activate the body
                if (activationMode == EBodyActivationMode::Activate && !body.IsStatic())
                    Internal_ActivateBody(body);
            }
        }
    }

    void BodyInterface::SetPositionAndRotationWhenChanged(const BodyID& bodyID, const Vector3& position,
        const Quat& rotation, EBodyActivationMode activationMode)
    {
        BodyLockWrite lock(*m_pBodyLockInterface, bodyID);
        if (lock.Succeeded())
        {
            Body& body = lock.GetBody();

            // Update the position and rotation
            if (!body.GetPosition().IsClose(position)
                || !body.GetRotation().IsClose(rotation))
            {
                body.Internal_SetPositionAndRotation(position, rotation);
            
                // Notify Broadphase of the change
                if (body.IsInBroadPhase())
                {
                    BodyID id = body.GetID();
                    m_pBroadPhase->NotifyBodiesAABBChanged(&id, 1);

                    // Optionally activate the body
                    if (activationMode == EBodyActivationMode::Activate && !body.IsStatic())
                        Internal_ActivateBody(body);
                }
            }
        }
    }

    void BodyInterface::GetPositionAndRotation(const BodyID& bodyID, Vector3& outPosition, Quat& outRotation) const
    {
        BodyLockRead lock(*m_pBodyLockInterface, bodyID);
        if (lock.Succeeded())
        {
            const Body& body = lock.GetBody();
            outPosition = body.GetPosition();
            outRotation = body.GetRotation();
        }
        else
        {
            outPosition = Vector3::Zero();
            outRotation = Quat::Identity();
        }
    }

    void BodyInterface::SetPosition(const BodyID& bodyID, const Vector3& position, EBodyActivationMode activationMode)
    {
        BodyLockWrite lock(*m_pBodyLockInterface, bodyID);
        if (lock.Succeeded())
        {
            Body& body = lock.GetBody();

            body.Internal_SetPositionAndRotation(position, body.GetRotation());
            
            // Notify Broadphase of the change
            if (body.IsInBroadPhase())
            {
                BodyID id = body.GetID();
                m_pBroadPhase->NotifyBodiesAABBChanged(&id, 1);

                // Optionally activate the body
                if (activationMode == EBodyActivationMode::Activate && !body.IsStatic())
                    Internal_ActivateBody(body);
            }
        }
    }

    Vector3 BodyInterface::GetPosition(const BodyID& bodyID) const
    {
        BodyLockRead lock(*m_pBodyLockInterface, bodyID);
        if (lock.Succeeded())
            return lock.GetBody().GetPosition();
        
        return Vector3::Zero();
    }

    Vector3 BodyInterface::GetCenterOfMassPosition(const BodyID& bodyID) const
    {
        BodyLockRead lock(*m_pBodyLockInterface, bodyID);
        if (lock.Succeeded())
            return lock.GetBody().GetCenterOfMassPosition();
        
        return Vector3::Zero();
    }

    void BodyInterface::SetRotation(const BodyID& bodyID, const Quat& rotation, EBodyActivationMode activationMode)
    {
        BodyLockWrite lock(*m_pBodyLockInterface, bodyID);
        if (lock.Succeeded())
        {
            Body& body = lock.GetBody();

            body.Internal_SetPositionAndRotation(body.GetPosition(), rotation);
            
            // Notify Broadphase of the change
            if (body.IsInBroadPhase())
            {
                BodyID id = body.GetID();
                m_pBroadPhase->NotifyBodiesAABBChanged(&id, 1);

                // Optionally activate the body
                if (activationMode == EBodyActivationMode::Activate && !body.IsStatic())
                    Internal_ActivateBody(body);
            }
        }
    }

    Quat BodyInterface::GetRotation(const BodyID& bodyID) const
    {
        BodyLockRead lock(*m_pBodyLockInterface, bodyID);
        if (lock.Succeeded())
            return lock.GetBody().GetRotation();
        
        return Quat::Identity();
    }

    Mat4 BodyInterface::GetWorldTransform(const BodyID& bodyID) const
    {
        BodyLockRead lock(*m_pBodyLockInterface, bodyID);
        if (lock.Succeeded())
            return lock.GetBody().GetWorldTransform();
        
        return Mat4::Identity();
    }

    Mat4 BodyInterface::GetCenterOfMassTransform(const BodyID& bodyID) const
    {
        BodyLockRead lock(*m_pBodyLockInterface, bodyID);
        if (lock.Succeeded())
            return lock.GetBody().GetCenterOfMassTransform();
        
        return Mat4::Identity();
    }

    void BodyInterface::MoveKinematic(const BodyID& bodyID, const Vector3& targetPosition, const Quat& targetRotation,
        float deltaTime)
    {
        BodyLockWrite lock(*m_pBodyLockInterface, bodyID);
        if (lock.Succeeded())
        {
            Body& body = lock.GetBody();

            body.MoveKinematic(targetPosition, targetRotation, deltaTime);

            // Wake the body if the not awake and velocities are not near zero.
            if (!body.IsActive() && (!body.GetLinearVelocity().IsNearZero() || !body.GetAngularVelocity().IsNearZero()))
                m_pBodyManager->ActivateBodies(&bodyID, 1);
        }
    }

    void BodyInterface::SetLinearAndAngularVelocity(const BodyID& bodyID, const Vector3& linearVelocity,
        const Vector3& angularVelocity)
    {
        BodyLockWrite lock(*m_pBodyLockInterface, bodyID);
        if (lock.Succeeded())
        {
            Body& body = lock.GetBody();
            if (!body.IsStatic())
            {
                body.SetLinearVelocityClamped(linearVelocity);
                body.SetAngularVelocityClamped(angularVelocity);

                // Wake the body if the not awake and velocities are not near zero.
                if (!body.IsActive() && (!linearVelocity.IsNearZero() || !angularVelocity.IsNearZero()))
                    m_pBodyManager->ActivateBodies(&bodyID, 1);
            }
        }
    }

    void BodyInterface::GetLinearAndAngularVelocity(const BodyID& bodyID, Vector3& outLinearVelocity,
        Vector3& outAngularVelocity) const
    {
        BodyLockRead lock(*m_pBodyLockInterface, bodyID);
        if (lock.Succeeded())
        {
            const Body& body = lock.GetBody();
            if (!body.IsStatic())
            {
                outLinearVelocity = body.GetLinearVelocity();
                outAngularVelocity = body.GetAngularVelocity();
                return;
            }
        }
        
        outLinearVelocity = Vector3::Zero();
        outAngularVelocity = Vector3::Zero();
    }

    void BodyInterface::SetLinearVelocity(const BodyID& bodyID, const Vector3& linearVelocity)
    {
        BodyLockWrite lock(*m_pBodyLockInterface, bodyID);
        if (lock.Succeeded())
        {
            Body& body = lock.GetBody();
            if (!body.IsStatic())
            {
                body.SetLinearVelocityClamped(linearVelocity);

                // Wake the body if the not awake and velocities are not near zero.
                if (!body.IsActive() && !linearVelocity.IsNearZero())
                    m_pBodyManager->ActivateBodies(&bodyID, 1);
            }
        }
    }

    Vector3 BodyInterface::GetLinearVelocity(const BodyID& bodyID) const
    {
        BodyLockRead lock(*m_pBodyLockInterface, bodyID);
        if (lock.Succeeded())
        {
            const Body& body = lock.GetBody();
            if (!body.IsStatic())
            {
                return body.GetLinearVelocity();
            }
        }
        
        return Vector3::Zero();
    }

    void BodyInterface::SetAngularVelocity(const BodyID& bodyID, const Vector3& angularVelocity)
    {
        BodyLockWrite lock(*m_pBodyLockInterface, bodyID);
        if (lock.Succeeded())
        {
            Body& body = lock.GetBody();
            if (!body.IsStatic())
            {
                body.SetAngularVelocityClamped(angularVelocity);

                // Wake the body if the not awake and velocities are not near zero.
                if (!body.IsActive() && !angularVelocity.IsNearZero())
                    m_pBodyManager->ActivateBodies(&bodyID, 1);
            }
        }
    }

    void BodyInterface::AddLinearVelocity(const BodyID& bodyID, const Vector3& deltaLinearVelocity)
    {
        BodyLockWrite lock(*m_pBodyLockInterface, bodyID);
        if (lock.Succeeded())
        {
            Body& body = lock.GetBody();
            if (!body.IsStatic())
            {
                body.SetLinearVelocityClamped(body.GetLinearVelocity() + deltaLinearVelocity);

                if (!body.IsActive() && !body.GetLinearVelocity().IsNearZero())
                    m_pBodyManager->ActivateBodies(&bodyID, 1);
            }
        }
    }

    void BodyInterface::AddLinearAndAngularVelocity(const BodyID& bodyID, const Vector3& deltaLinearVelocity,
        const Vector3& deltaAngularVelocity)
    {
        BodyLockWrite lock(*m_pBodyLockInterface, bodyID);
        if (lock.Succeeded())
        {
            Body& body = lock.GetBody();
            if (!body.IsStatic())
            {
                body.SetLinearVelocityClamped(body.GetLinearVelocity() + deltaLinearVelocity);
                body.SetAngularVelocityClamped(body.GetAngularVelocity() + deltaAngularVelocity);

                // Wake the body if the not awake and velocities are not near zero.
                if (!body.IsActive() && (!body.GetLinearVelocity().IsNearZero() || !body.GetAngularVelocity().IsNearZero()))
                    m_pBodyManager->ActivateBodies(&bodyID, 1);
            }
        }
    }

    Vector3 BodyInterface::GetAngularVelocity(const BodyID& bodyID) const
    {
        BodyLockRead lock(*m_pBodyLockInterface, bodyID);
        if (lock.Succeeded())
        {
            const Body& body = lock.GetBody();
            if (!body.IsStatic())
            {
                return body.GetAngularVelocity();
            }
        }
        
        return Vector3::Zero();
    }

    Vector3 BodyInterface::GetPointVelocity(const BodyID& bodyID, const Vector3& point) const
    {
        BodyLockRead lock(*m_pBodyLockInterface, bodyID);
        if (lock.Succeeded())
        {
            const Body& body = lock.GetBody();
            if (!body.IsStatic())
            {
                return body.GetPointVelocity(point);
            }
        }
        
        return Vector3::Zero();
    }

    void BodyInterface::SetPositionAndRotationAndVelocity(const BodyID& bodyID, const Vector3& position,
        const Quat& rotation, const Vector3& linearVelocity, const Vector3& angularVelocity)
    {
        BodyLockWrite lock(*m_pBodyLockInterface, bodyID);
        if (lock.Succeeded())
        {
            Body& body = lock.GetBody();

            // Update position and rotation
            body.Internal_SetPositionAndRotation(position, rotation);

            // Notify the broadphase
            if (body.IsInBroadPhase())
            {
                BodyID id = body.GetID();
                m_pBroadPhase->NotifyBodiesAABBChanged(&id, 1);
            }

            if (!body.IsStatic())
            {
                body.SetLinearVelocityClamped(linearVelocity);
                body.SetAngularVelocityClamped(angularVelocity);

                // Optionally activate the body
                if (!body.IsActive() && (!body.GetLinearVelocity().IsNearZero() || !body.GetAngularVelocity().IsNearZero()))
                    m_pBodyManager->ActivateBodies(&bodyID, 1);
            }
        }
    }

    void BodyInterface::AddForce(const BodyID& bodyID, const Vector3& force, EBodyActivationMode activationMode)
    {
        BodyLockWrite lock(*m_pBodyLockInterface, bodyID);
        if (lock.Succeeded())
        {
            Body& body = lock.GetBody();
            if (body.IsDynamic() && (activationMode == EBodyActivationMode::Activate || body.IsActive()))
            {
                body.AddForce(force);

                if (activationMode == EBodyActivationMode::Activate)
                    Internal_ActivateBody(body);
            }
        }
    }

    void BodyInterface::AddForce(const BodyID& bodyID, const Vector3& force, const Vector3& point,
        EBodyActivationMode activationMode)
    {
        BodyLockWrite lock(*m_pBodyLockInterface, bodyID);
        if (lock.Succeeded())
        {
            Body& body = lock.GetBody();
            if (body.IsDynamic() && (activationMode == EBodyActivationMode::Activate || body.IsActive()))
            {
                body.AddForce(force, point);

                if (activationMode == EBodyActivationMode::Activate)
                    Internal_ActivateBody(body);
            }
        }
    }

    void BodyInterface::AddTorque(const BodyID& bodyID, const Vector3& torque, EBodyActivationMode activationMode)
    {
        BodyLockWrite lock(*m_pBodyLockInterface, bodyID);
        if (lock.Succeeded())
        {
            Body& body = lock.GetBody();
            if (body.IsDynamic() && (activationMode == EBodyActivationMode::Activate || body.IsActive()))
            {
                body.AddTorque(torque);

                if (activationMode == EBodyActivationMode::Activate)
                    Internal_ActivateBody(body);
            }
        }
    }

    void BodyInterface::AddForceAndTorque(const BodyID& bodyID, const Vector3& force, const Vector3& torque,
        EBodyActivationMode activationMode)
    {
        BodyLockWrite lock(*m_pBodyLockInterface, bodyID);
        if (lock.Succeeded())
        {
            Body& body = lock.GetBody();
            if (body.IsDynamic() && (activationMode == EBodyActivationMode::Activate || body.IsActive()))
            {
                body.AddForce(force);
                body.AddTorque(torque);

                if (activationMode == EBodyActivationMode::Activate)
                    Internal_ActivateBody(body);
            }
        }
    }

    void BodyInterface::AddImpulse(const BodyID& bodyID, const Vector3& impulse)
    {
        BodyLockWrite lock(*m_pBodyLockInterface, bodyID);
        if (lock.Succeeded())
        {
            Body& body = lock.GetBody();
            if (body.IsDynamic())
            {
                body.AddImpulse(impulse);

                if (!body.IsActive())
                    m_pBodyManager->ActivateBodies(&bodyID, 1);
            }
        }
    }

    void BodyInterface::AddImpulse(const BodyID& bodyID, const Vector3& impulse, const Vector3& point)
    {
        BodyLockWrite lock(*m_pBodyLockInterface, bodyID);
        if (lock.Succeeded())
        {
            Body& body = lock.GetBody();
            if (body.IsDynamic())
            {
                body.AddImpulse(impulse, point);

                if (!body.IsActive())
                    m_pBodyManager->ActivateBodies(&bodyID, 1);
            }
        }
    }

    void BodyInterface::AddAngularImpulse(const BodyID& bodyID, const Vector3& angularImpulse)
    {
        BodyLockWrite lock(*m_pBodyLockInterface, bodyID);
        if (lock.Succeeded())
        {
            Body& body = lock.GetBody();
            if (body.IsDynamic())
            {
                body.AddAngularImpulse(angularImpulse);

                if (!body.IsActive())
                    m_pBodyManager->ActivateBodies(&bodyID, 1);
            }
        }
    }

    void BodyInterface::SetMotionType(const BodyID& bodyID, EBodyMotionType motionType,
        EBodyActivationMode activationMode)
    {
        BodyLockWrite lock(*m_pBodyLockInterface, bodyID);
        if (lock.Succeeded())
        {
            Body& body = lock.GetBody();
 
            // Deactivate if we're making the body static
            if (body.IsActive() && motionType == EBodyMotionType::Kinematic)
                m_pBodyManager->DeactivateBodies(&bodyID, 1);

            body.SetMotionType(motionType);

            // Activate body if requested
            if (motionType != EBodyMotionType::Static && activationMode == EBodyActivationMode::Activate)
                Internal_ActivateBody(body);
        }
    }

    EBodyMotionType BodyInterface::GetMotionType(const BodyID& bodyID) const
    {
        BodyLockRead lock(*m_pBodyLockInterface, bodyID);
        if (lock.Succeeded())
            return lock.GetBody().GetMotionType();

        return EBodyMotionType::Static;
    }

    void BodyInterface::SetMotionQuality(const BodyID& bodyID, const EBodyMotionQuality quality)
    {
        BodyLockWrite lock(*m_pBodyLockInterface, bodyID);
        if (lock.Succeeded())
            m_pBodyManager->SetMotionQuality(lock.GetBody(), quality);
    }

    EBodyMotionQuality BodyInterface::GetMotionQuality(const BodyID& bodyID) const
    {
        BodyLockRead lock(*m_pBodyLockInterface, bodyID);
        if (lock.Succeeded() && !lock.GetBody().IsStatic())
            return lock.GetBody().GetMotionProperties()->GetMotionQuality();

        return EBodyMotionQuality::Discrete;
    }

    Mat4 BodyInterface::GetInverseInertia(const BodyID& bodyID) const
    {
        BodyLockRead lock(*m_pBodyLockInterface, bodyID);
        if (lock.Succeeded())
            return lock.GetBody().GetInverseInertia();
        
        return Mat4::Identity();
    }

    void BodyInterface::SetRestitution(const BodyID& bodyID, const float restitution)
    {
        BodyLockWrite lock(*m_pBodyLockInterface, bodyID);
        if (lock.Succeeded())
            lock.GetBody().SetRestitution(restitution);
    }

    float BodyInterface::GetRestitution(const BodyID& bodyID) const
    {
        BodyLockRead lock(*m_pBodyLockInterface, bodyID);
        if (lock.Succeeded())
            return lock.GetBody().GetRestitution();

        return 0.f;
    }

    void BodyInterface::SetFriction(const BodyID& bodyID, const float friction)
    {
        BodyLockWrite lock(*m_pBodyLockInterface, bodyID);
        if (lock.Succeeded())
            lock.GetBody().SetFriction(friction);
    }

    float BodyInterface::GetFriction(const BodyID& bodyID) const
    {
        BodyLockRead lock(*m_pBodyLockInterface, bodyID);
        if (lock.Succeeded())
            return lock.GetBody().GetFriction();

        return 0.f;
    }

    void BodyInterface::SetGravityScale(const BodyID& bodyID, const float gravityScale)
    {
        BodyLockWrite lock(*m_pBodyLockInterface, bodyID);
        if (lock.Succeeded() && lock.GetBody().GetMotionPropertiesUnchecked() != nullptr)
            lock.GetBody().GetMotionPropertiesUnchecked()->SetGravityScale(gravityScale);
    }

    float BodyInterface::GetGravityScale(const BodyID& bodyID) const
    {
        BodyLockRead lock(*m_pBodyLockInterface, bodyID);
        if (lock.Succeeded() && lock.GetBody().GetMotionPropertiesUnchecked() != nullptr)
            return lock.GetBody().GetMotionPropertiesUnchecked()->GetGravityScale();

        return 1.f;
    }

    void BodyInterface::SetUseManifoldReduction(const BodyID& bodyID, bool useManifoldReduction)
    {
        BodyLockWrite lock(*m_pBodyLockInterface, bodyID);
        if (lock.Succeeded())
        {
            Body& body = lock.GetBody();
            if (body.GetUseManifoldReduction() != useManifoldReduction)
            {
                body.SetUseManifoldReduction(useManifoldReduction);

                // Flag collision cache invalid for this body
                m_pBodyManager->InvalidateContactCacheForBody(body);
            }
        }
    }

    bool BodyInterface::GetUseManifoldReduction(const BodyID& bodyID) const
    {
        BodyLockRead lock(*m_pBodyLockInterface, bodyID);
        if (lock.Succeeded())
            return lock.GetBody().GetUseManifoldReduction();

        return true;
    }

    void BodyInterface::SetCollisionGroup(const BodyID& bodyID, const CollisionGroup& collisionGroup)
    {
        BodyLockWrite lock(*m_pBodyLockInterface, bodyID);
        if (lock.Succeeded())
            lock.GetBody().SetCollisionGroup(collisionGroup);
    }

    const CollisionGroup& BodyInterface::GetCollisionGroup(const BodyID& bodyID) const
    {
        BodyLockRead lock(*m_pBodyLockInterface, bodyID);
        if (lock.Succeeded())
            return lock.GetBody().GetCollisionGroup();

        return CollisionGroup::s_Invalid;
    }

    TransformedShape BodyInterface::GetTransformedShape(const BodyID& bodyID) const
    {
        BodyLockRead lock(*m_pBodyLockInterface, bodyID);
        if (lock.Succeeded())
            return lock.GetBody().GetTransformedShape();

        return TransformedShape();
    }

    uint64_t BodyInterface::GetUserData(const BodyID& bodyID) const
    {
        BodyLockRead lock(*m_pBodyLockInterface, bodyID);
        if (lock.Succeeded())
            return lock.GetBody().GetUserData();

        return 0;
    }

    void BodyInterface::SetUserData(const BodyID& bodyID, uint64_t userData)
    {
        BodyLockWrite lock(*m_pBodyLockInterface, bodyID);
        if (lock.Succeeded())
            lock.GetBody().SetUserData(userData);
    }

    void BodyInterface::Internal_Init(BodyLockInterface& lockInterface, BodyManager& bodyManager,
        BroadPhase& broadPhase)
    {
        m_pBodyLockInterface = &lockInterface;
        m_pBodyManager = &bodyManager;
        m_pBroadPhase = &broadPhase;
    }

    void BodyInterface::Internal_InvalidateContactCache(const BodyID& bodyID)
    {
        BodyLockWrite lock(*m_pBodyLockInterface, bodyID);
        if (lock.Succeeded())
            m_pBodyManager->InvalidateContactCacheForBody(lock.GetBody());
    }

    void BodyInterface::Internal_ActivateBody(Body& body) const
    {
        // Activate body or reset its sleep timer
        // Note that BodyManager::ActivateBodies also resets the sleep timer internally,
        // but we avoid a mutex lock if the body is already active by calling ResetSleepTimer directly.
        if (!body.IsActive())
            m_pBodyManager->ActivateBodies(&body.GetID(), 1);
        else
            body.ResetSleepTimer();
    }
}
