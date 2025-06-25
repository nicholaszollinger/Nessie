// BodyManager.cpp
#include "BodyManager.h"

#include <functional>
#include "BodyLock.h"
#include "BodyActivationListener.h"

namespace nes
{
#if NES_ASSERTS_ENABLED
    static thread_local bool s_overrideAllowActivation = false;
    static thread_local bool s_overrideAllowDeactivation = false;
    
    bool BodyManager::GetOverrideAllowActivation()
    {
        return s_overrideAllowActivation;
    }

    void BodyManager::SetOverrideAllowActivation(const bool allowActivation)
    {
        s_overrideAllowActivation = allowActivation;
    }

    bool BodyManager::GetOverrideAllowDeactivation()
    {
        return s_overrideAllowDeactivation;
    }

    void BodyManager::SetOverrideAllowDeactivation(const bool allowDeactivation)
    {
        s_overrideAllowDeactivation = allowDeactivation;
    }
#endif
    
    //----------------------------------------------------------------------------------------------------
    /// @brief : Helper class that combines a Body with its motion properties. 
    //----------------------------------------------------------------------------------------------------
    class BodyWithMotionProperties final : public Body
    {
    public:
        NES_OVERRIDE_NEW_DELETE
        
        MotionProperties m_motionProperties{};  
        BodyWithMotionProperties() = default;
    };

    BodyManager::~BodyManager()
    {
        UniqueLock lock(m_bodiesMutex NES_IF_ASSERTS_ENABLED(, this, EPhysicsLockTypes::BodiesArray));

        // Destroy any bodies that are still alive
        for (Body* pBody : m_bodies)
        {
            if (IsValidBodyPointer(pBody))
                DeleteBody(pBody);
        }

        // [TODO]: Delete Soft Body Type array as well
        NES_DELETE_ARRAY(m_pActiveBodies);
    }

    void BodyManager::Init(const uint32_t maxBodies, const uint32_t numBodyMutexes, const BroadPhaseLayerInterface& layerInterface)
    {
        UniqueLock lock(m_bodiesMutex NES_IF_ASSERTS_ENABLED(, this, EPhysicsLockTypes::BodiesArray));

        uint32_t finalNumBodyMutexes = math::Clamp<uint32_t>(math::GetNextPowerOf2(numBodyMutexes == 0? 2 * std::thread::hardware_concurrency() : numBodyMutexes), 1, sizeof(MutexMask) * 8);

        // Allocate the body mutexes
        m_bodyMutexes.Init(finalNumBodyMutexes);

        // Allocate space for bodies
        m_bodies.reserve(maxBodies);

        // Allocate space for active bodies
        NES_ASSERT(m_pActiveBodies == nullptr);
        m_pActiveBodies = NES_NEW_ARRAY(BodyID, maxBodies);

        // Allocate space for sequence numbers
        m_bodySequenceNumbers.resize(maxBodies, 0);

        // Keep layer interface
        m_pBroadPhaseLayer = &layerInterface;
    }

    uint32_t BodyManager::GetNumBodies() const
    {
        UniqueLock lock(m_bodiesMutex NES_IF_ASSERTS_ENABLED(, this, EPhysicsLockTypes::BodiesArray));
        
        return m_numBodies;
    }

    BodyManager::BodyStats BodyManager::GetStats() const
    {
        UniqueLock lock(m_bodiesMutex NES_IF_ASSERTS_ENABLED(, this, EPhysicsLockTypes::BodiesArray));

        BodyStats stats;
        stats.m_numBodies = m_numBodies;
        stats.m_maxNumBodies = static_cast<uint32_t>(m_bodies.capacity());

        for (const Body* pBody : m_bodies)
        {
            if (!IsValidBodyPointer(pBody))
                continue;
            
            if (pBody->IsSoftBody())
            {
                // [TODO]: 
                // stats.m_numSoftBodies++;
                // if (pBody->IsActive())
                //     stats.m_numActiveSoftBodies++;
            }
            else
            {
                switch (pBody->GetMotionType())
                {
                    case EBodyMotionType::Static:
                    {
                        stats.m_numStaticBodies++;
                        break;
                    }

                    case EBodyMotionType::Dynamic:
                    {
                        stats.m_numDynamicBodies++;
                        if (pBody->IsActive())
                            stats.m_numActiveDynamicBodies++;
                        break;
                    }

                    case EBodyMotionType::Kinematic:
                    {
                        stats.m_numKinematicBodies++;
                        if (pBody->IsActive())
                            stats.m_numActiveKinematicBodies++;
                        break;
                    }
                }
            }
        }

        return stats;
    }

    Body* BodyManager::AllocateBody(const BodyCreateInfo& createInfo) const
    {
        Body* pBody = nullptr;
        if (createInfo.HasMassProperties())
        {
            BodyWithMotionProperties* pBodyWithMotionProperties = new BodyWithMotionProperties();
            pBody = pBodyWithMotionProperties;
            pBody->m_pMotionProperties = &pBodyWithMotionProperties->m_motionProperties;
        }
        else
        {
            pBody = new Body();
        }

        //pBody->m_bodyType = BodyType::Rigid;
        pBody->m_pShape = createInfo.GetShape();
        pBody->m_userData = createInfo.m_userData;
        pBody->SetFriction(createInfo.m_friction);
        pBody->SetRestitution(createInfo.m_restitution);
        pBody->m_motionType = createInfo.m_motionType;
        
        if (createInfo.m_isSensor)
            pBody->SetIsSensor(true);

        if (createInfo.m_collideKinematicVsNonDynamic)
            pBody->SetCollideKinematicVsNonDynamic(true);

        if (createInfo.m_useManifoldReduction)
            pBody->SetUseManifoldReduction(true);

        if (createInfo.m_applyGyroscopicForce)
            pBody->SetApplyGyroscopicForce(true);

        if (createInfo.m_enhancedInternalEdgeRemoval)
            pBody->SetEnhancedInternalEdgeRemoval(true);

        Internal_SetBodyCollisionLayer(*pBody, createInfo.m_collisionLayer);
        pBody->m_collisionLayer = createInfo.m_collisionLayer;
        pBody->m_collisionGroup = createInfo.m_collisionGroup;

        if (createInfo.HasMassProperties())
        {
            MotionProperties* pProps = pBody->m_pMotionProperties;
            pProps->SetLinearDamping(createInfo.m_linearDamping);
            pProps->SetAngularDamping(createInfo.m_angularDamping);
            pProps->SetMaxLinearVelocity(createInfo.m_maxLinearVelocity);
            pProps->SetMaxAngularVelocity(createInfo.m_maxAngularVelocity);
            pProps->SetMassProperties(createInfo.m_allowedDOFs, createInfo.GetMassProperties());
            pProps->SetGravityScale(createInfo.m_gravityScale);
            pProps->SetNumVelocityStepsOverride(createInfo.m_numVelocityStepsOverride);
            pProps->SetNumPositionStepsOverride(createInfo.m_numPositionStepsOverride);
            pProps->m_motionQuality = createInfo.m_motionQuality;
            pProps->m_canSleep = createInfo.m_allowSleeping;
            
            //NES_IF_LOGGING_ENABLED(pProps->m_cachedBodyType = pBody->m_bodyType);
            NES_IF_LOGGING_ENABLED(pProps->m_cachedMotionType = pBody->m_motionType);
        }

        // Set the initial position of the body:
        pBody->Internal_SetPositionAndRotation(createInfo.m_position, createInfo.m_rotation);

        return pBody;
    }

    void BodyManager::FreeBody(Body* pBody) const
    {
        NES_ASSERT(!pBody->GetID().IsValid(), "This function should only e called on a body that doesn't have an ID yet! Use DestroyBody() otherwise.");
        DeleteBody(pBody);
    }

    bool BodyManager::AddBody(Body* pBody)
    {
        // Return an error when the body was already added
        if (pBody->GetID().IsValid())
            return false;

        // Determine the next free index:
        uint32_t index;
        {
            UniqueLock lock(m_bodiesMutex NES_IF_ASSERTS_ENABLED(, this, EPhysicsLockTypes::BodiesArray));

            if (m_bodyIDFreeListStart != kBodyIDFreeListEnd)
            {
                // Pop an item from the free list:
                NES_ASSERT(m_bodyIDFreeListStart & kIsFreedBody);
                index = static_cast<uint32_t>(m_bodyIDFreeListStart >> kFreedBodyIndexShift);
                NES_ASSERT(!IsValidBodyPointer(m_bodies[index]));
                m_bodyIDFreeListStart = reinterpret_cast<uintptr_t>(m_bodies[index]);
                m_bodies[index] = pBody;
            }

            else
            {
                if (m_bodies.size() < m_bodies.capacity())
                {
                    index = static_cast<uint32_t>(m_bodies.size());
                    m_bodies.push_back(pBody);
                }
                else
                {
                    // Out of bodies.
                    return false;
                }
            }

            // Update cached number of bodies.
            ++m_numBodies;
        }

        // Get the next sequence number and assign the new ID.
        const uint8_t sequenceNumber = GetNextSequenceNumber(static_cast<int>(index));
        pBody->m_id = BodyID(index, sequenceNumber);

        return true;
    }

    bool BodyManager::AddBodyWithCustomID(Body* pBody, const BodyID& bodyID)
    {
        // Return an error when the body was already added
        if (pBody->GetID().IsValid())
            return false;

        {
            UniqueLock lock(m_bodiesMutex NES_IF_ASSERTS_ENABLED(, this, EPhysicsLockTypes::BodiesArray));

            const uint32_t index = bodyID.GetIndex();
            if (index >= m_bodies.capacity())
                return false;

            if (index < m_bodies.size())
            {
                // Body array entry has already been allocated, ensure that there's a free body at the location:
                if (IsValidBodyPointer(m_bodies[index]))
                    return false;

                // Remove the entry from the free list
                uintptr_t indexStart = m_bodyIDFreeListStart >> kFreedBodyIndexShift;
                if (index == indexStart)
                {
                    // First entry, easy to remove, the start of the list is our next
                    m_bodyIDFreeListStart = reinterpret_cast<uintptr_t>(m_bodies[index]);
                }
                else
                {
                    // Loop over the freelist and find the entry in the freelist pointing to our index.
                    // [TODO]: This is O(N), see if this becomes a performance problem (don't want to put the freed bodies in a double linked list.
                    uintptr_t next;
                    for (uintptr_t current = indexStart; current != kBodyIDFreeListEnd; current = next)
                    {
                        next = reinterpret_cast<uintptr_t>(m_bodies[current]) >> kFreedBodyIndexShift;
                        if (next == index)
                        {
                            m_bodies[current] = m_bodies[index];
                            break;
                        }
                        NES_ASSERT(current != (kBodyIDFreeListEnd >> kFreedBodyIndexShift));
                    }
                }

                // Put the body in the slot.
                m_bodies[index] = pBody;
            }

            else
            {
                // Ensure that all body IDs up to this body ID have been allocated and added to our free list.
                while (index > m_bodies.size())
                {
                    m_bodies.push_back(reinterpret_cast<Body*>(m_bodyIDFreeListStart));
                    m_bodyIDFreeListStart = (static_cast<uintptr_t>(m_bodies.size() - 1) << kFreedBodyIndexShift) | kIsFreedBody;
                }

                // Add the element to the list
                m_bodies.push_back(pBody);
            }

            // Update the cached number of bodies.
            ++m_numBodies;
        }

        // Assign the ID.
        pBody->m_id = bodyID;
        return true;
    }

    void BodyManager::RemoveBodies(const BodyID* pBodyIDs, const int count, Body** pOutBodies)
    {
        // Don't take a lock if no bodies are to be destroyed
        if (count <= 0)
            return;

        UniqueLock lock(m_bodiesMutex NES_IF_ASSERTS_ENABLED(, this, EPhysicsLockTypes::BodiesArray));

        // Update cached number of bodies
        NES_ASSERT(m_numBodies >= static_cast<uint32_t>(count));
        m_numBodies -= count;

        for (const BodyID* pID = pBodyIDs; pID < pBodyIDs + count; ++pID)
        {
            // Remove the Body
            Body* pBody = RemoveBody(*pID);

            // Clear the ID
            pBody->m_id = BodyID();

            // Return the body to the caller
            if (pOutBodies != nullptr)
            {
                *pOutBodies = pBody;
                ++pOutBodies;
            }
        }

    #if defined (NES_DEBUG) && defined(NES_IF_LOGGING_ENABLED)
        ValidateFreeList();
    #endif
    }

    void BodyManager::DestroyBodies(const BodyID* pBodyIDs, const int count)
    {
        // Don't take a lock if no bodies are to be destroyed.
        if (count <= 0)
            return;

        UniqueLock lock(m_bodiesMutex NES_IF_ASSERTS_ENABLED(, this, EPhysicsLockTypes::BodiesArray));

        // Update the cached number of bodies.
        NES_ASSERT(m_numBodies >= static_cast<uint32_t>(count));
        m_numBodies -= count;

        for (const BodyID* pID = pBodyIDs; pID < pBodyIDs + count; ++pID)
        {
            // Remove the Body
            Body* pBody = RemoveBody(*pID);

            // Free the Body
            DeleteBody(pBody);
        }

    #if defined (NES_DEBUG) && defined(NES_IF_LOGGING_ENABLED)
        ValidateFreeList();
    #endif
    }

    void BodyManager::ActivateBodies(const BodyID* pBodyIDs, const int count)
    {
        // Don't take lock if no bodies are to be activated
        if (count <= 0)
            return;

        UniqueLock lock(m_activeBodiesMutex NES_IF_ASSERTS_ENABLED(, this, EPhysicsLockTypes::ActiveBodiesArray));
        
        NES_ASSERT(!m_activeBodiesLocked || s_overrideAllowActivation);

        for (const BodyID* pID = pBodyIDs; pID < pBodyIDs + count; ++pID)
        {
            if (pID->IsValid())
            {
                BodyID id = *pID;
                Body& body = *m_bodies[id.GetIndex()];

                NES_ASSERT(body.GetID() == id);
                NES_ASSERT(body.IsInBroadPhase(), "Use BodyInterface::AddBody to add the body first!");

                if (!body.IsStatic())
                {
                    // Reset the sleeping timer so that we don't immediately go to sleep again
                    body.ResetSleepTimer();

                    // Check if we are sleeping:
                    if (body.m_pMotionProperties->m_indexInActiveBodies == Body::kInactiveIndex)
                    {
                        AddBodyToActiveBodies(body);

                        // Call the activation listener
                        if (m_pActivationListener != nullptr)
                            m_pActivationListener->OnBodyActivated(id, body.GetUserData());
                    }
                }
            }
        }
    }

    void BodyManager::DeactivateBodies(const BodyID* pBodyIDs, const int count)
    {
        // Don't take lock if no bodies are to be activated
        if (count <= 0)
            return;

        UniqueLock lock(m_activeBodiesMutex NES_IF_ASSERTS_ENABLED(, this, EPhysicsLockTypes::ActiveBodiesArray));
        
        NES_ASSERT(!m_activeBodiesLocked || s_overrideAllowDeactivation);

        for (const BodyID* pID = pBodyIDs; pID < pBodyIDs + count; ++pID)
        {
            if (pID->IsValid())
            {
                BodyID id = *pID;
                Body& body = *m_bodies[id.GetIndex()];

                NES_ASSERT(body.GetID() == id);
                NES_ASSERT(body.IsInBroadPhase(), "Use BodyInterface::AddBody to add the body first!");

                if (body.m_pMotionProperties != nullptr && body.m_pMotionProperties->m_indexInActiveBodies != Body::kInactiveIndex)
                {
                    // Remove from the active bodies list
                    RemoveBodyFromActiveBodies(body);

                    // Mark this body as no longer active
                    body.m_pMotionProperties->m_islandIndex = Body::kInactiveIndex;

                    // Reset the velocity
                    body.m_pMotionProperties->m_linearVelocity = Vec3::Zero();
                    body.m_pMotionProperties->m_angularVelocity = Vec3::Zero();
                    
                    // Call the activation listener
                    if (m_pActivationListener != nullptr)
                        m_pActivationListener->OnBodyDeactivated(id, body.GetUserData());
                }
            }
        }
    }

    void BodyManager::SetMotionQuality(Body& body, const EBodyMotionQuality motionQuality)
    {
        MotionProperties* pMotion = body.GetMotionPropertiesUnchecked();
        if (pMotion != nullptr && pMotion->GetMotionQuality() != motionQuality)
        {
            UniqueLock lock(m_activeBodiesMutex NES_IF_ASSERTS_ENABLED(, this, EPhysicsLockTypes::ActiveBodiesArray));

            NES_ASSERT(!m_activeBodiesLocked);

            bool isActive = body.IsActive();
            if (isActive && pMotion->GetMotionQuality() == EBodyMotionQuality::LinearCast)
                --m_numActiveCCDBodies;

            pMotion->m_motionQuality = motionQuality;

            if (isActive && pMotion->GetMotionQuality() == EBodyMotionQuality::LinearCast)
                ++m_numActiveCCDBodies;
        }
    }

    void BodyManager::GetActiveBodies(/*BodyType type, */BodyIDVector& outBodies) const
    {
        // [TODO]: Profile
        // [TODO]: Delineate between rigid and soft bodies.
        UniqueLock lock(m_activeBodiesMutex NES_IF_ASSERTS_ENABLED(, this, EPhysicsLockTypes::ActiveBodiesArray));
        outBodies.assign(m_pActiveBodies, m_pActiveBodies + m_numActiveBodies.load(std::memory_order_relaxed));
    }

    void BodyManager::SetBodyActivationListener(BodyActivationListener* pListener)
    {
        UniqueLock lock(m_activeBodiesMutex NES_IF_ASSERTS_ENABLED(, this, EPhysicsLockTypes::ActiveBodiesArray));
        m_pActivationListener = pListener;
    }

    void BodyManager::GetBodyIDs(BodyIDVector& outBodies) const
    {
        // [TODO]: Profile
        UniqueLock lock(m_bodiesMutex NES_IF_ASSERTS_ENABLED(, this, EPhysicsLockTypes::BodiesArray));

        // Reserve space for all bodies
        outBodies.clear();
        outBodies.reserve(m_numBodies);

        // Iterate the list and find the bodies that are not null
        for (const Body* pBody : m_bodies)
        {
            if (IsValidBodyPointer(pBody))
                outBodies.push_back(pBody->GetID());
        }

        // Validate that the reserve was correct.
        NES_ASSERT(outBodies.size() == m_numBodies);
    }

    const Body* BodyManager::TryGetBody(const BodyID& id) const
    {
        const uint32_t index = id.GetIndex();
        if (index >= m_bodies.size())
            return nullptr;

        const Body* pBody = m_bodies[index];
        if (IsValidBodyPointer(pBody) && pBody->GetID() == id)
            return pBody;

        return nullptr;
    }

    Body* BodyManager::TryGetBody(const BodyID& id)
    {
        const uint32_t index = id.GetIndex();
        if (index >= m_bodies.size())
            return nullptr;

        Body* pBody = m_bodies[index];
        if (IsValidBodyPointer(pBody) && pBody->GetID() == id)
            return pBody;

        return nullptr;
    }

    void BodyManager::LockAllBodies() const
    {
        NES_IF_ASSERTS_ENABLED(PhysicsLock::CheckLock(this, EPhysicsLockTypes::PerBody));
        m_bodyMutexes.LockAll();

        PhysicsLock::Lock(m_bodiesMutex NES_IF_ASSERTS_ENABLED(, this, EPhysicsLockTypes::BodiesArray));
    }

    void BodyManager::UnlockAllBodies() const
    {
        PhysicsLock::Unlock(m_bodiesMutex NES_IF_ASSERTS_ENABLED(, this, EPhysicsLockTypes::BodiesArray));
        
        NES_IF_ASSERTS_ENABLED(PhysicsLock::CheckUnlock(this, EPhysicsLockTypes::PerBody));
        m_bodyMutexes.UnlockAll();
    }

    void BodyManager::InvalidateContactCacheForBody(Body& body)
    {
        // If this is the first time we flip the collision cache invalid flag, we need to add it to an internal list to ensure that we reset the flag at the
        // end of the physics update.
        if (body.Internal_InvalidateContactCache())
        {
            std::lock_guard lock(m_bodiesCacheInvalidMutex);
            m_bodiesCacheInvalid.push_back(body.GetID());
        }
    }

    void BodyManager::ValidateContactCacheForAllBodies()
    {
        std::lock_guard lock(m_bodiesCacheInvalidMutex);

        for (const BodyID& id : m_bodiesCacheInvalid)
        {
            // The body may have been removed between the call to InvalidateContactCacheForBody() and this call, so check if
            // it still exists.
            Body* pBody = TryGetBody(id);
            if (pBody != nullptr)
                pBody->Internal_ValidateContactCache();
        }
        m_bodiesCacheInvalid.clear();
    }

    BodyManager::MutexMask BodyManager::Internal_GetAllBodiesMutexMask() const
    {
        return m_bodyMutexes.GetNumMutexes() == sizeof(MutexMask) * 8 ?
            ~static_cast<MutexMask>(0)
            : (static_cast<MutexMask>(1) << m_bodyMutexes.GetNumMutexes()) - 1;
    }

    BodyManager::MutexMask BodyManager::Internal_GetMutexMask(const BodyID* pBodyIDs, int number) const
    {
        NES_ASSERT(sizeof(MutexMask) * 8 >= m_bodyMutexes.GetNumMutexes(), "MutexMask must have enough bits");

        if (number >= static_cast<int>(m_bodyMutexes.GetNumMutexes()))
        {
            // Just lock everything if there are too many bodies.
            return Internal_GetAllBodiesMutexMask();
        }

        MutexMask mask = 0;
        for (const BodyID* pID = pBodyIDs; pID != pBodyIDs + number; ++pID)
        {
            if (pID->IsValid())
            {
                uint32_t index = m_bodyMutexes.GetMutexIndex(pBodyIDs->GetIndex());
                mask |= (static_cast<MutexMask>(1) << index);
            }
        }
        return mask;
    }

    void BodyManager::Internal_LockRead(const MutexMask mask) const
    {
        NES_IF_ASSERTS_ENABLED(PhysicsLock::CheckLock(this, EPhysicsLockTypes::PerBody));

        int index = 0;
        for (MutexMask current = mask; current != 0; current >>= 1, ++index)
        {
            if (current & 1)
                m_bodyMutexes.GetMutexByIndex(index).lock_shared();
        }
    }

    void BodyManager::Internal_UnlockRead(const MutexMask mask) const
    {
        NES_IF_ASSERTS_ENABLED(PhysicsLock::CheckUnlock(this, EPhysicsLockTypes::PerBody));
        
        int index = 0;
        for (MutexMask current = mask; current != 0; current >>= 1, ++index)
        {
            if (current & 1)
                m_bodyMutexes.GetMutexByIndex(index).unlock_shared();
        }
    }

    void BodyManager::Internal_LockWrite(const MutexMask mask) const
    {
        NES_IF_ASSERTS_ENABLED(PhysicsLock::CheckLock(this, EPhysicsLockTypes::PerBody));
        
        int index = 0;
        for (MutexMask current = mask; current != 0; current >>= 1, ++index)
        {
            if (current & 1)
                m_bodyMutexes.GetMutexByIndex(index).lock();
        }
    }

    void BodyManager::Internal_UnlockWrite(const MutexMask mask) const
    {
        NES_IF_ASSERTS_ENABLED(PhysicsLock::CheckUnlock(this, EPhysicsLockTypes::PerBody));
        
        int index = 0;
        for (MutexMask current = mask; current != 0; current >>= 1, ++index)
        {
            if (current & 1)
                m_bodyMutexes.GetMutexByIndex(index).unlock();
        }
    }

    void BodyManager::Internal_SetBodyCollisionLayer(Body& body, const CollisionLayer layer) const
    {
        body.m_collisionLayer = layer;
        body.m_broadPhaseLayer = m_pBroadPhaseLayer->GetBroadPhaseLayer(layer);
    }

    void BodyManager::AddBodyToActiveBodies(Body& body)
    {
        // [TODO]: Delineate between Rigid and Soft body types.
        //int type = static_cast<int>(body.GetBodyType());

        MotionProperties* pMotion = body.m_pMotionProperties;
        uint32_t numActiveBodiesVal = m_numActiveBodies.load(std::memory_order_relaxed);
        pMotion->m_indexInActiveBodies = numActiveBodiesVal;
        NES_ASSERT(numActiveBodiesVal < GetMaxNumBodies());

        m_pActiveBodies[numActiveBodiesVal] = body.GetID();
        // Increment atomic after setting the bodyID so that the PhysicsSystem::JobFindCollisions (which doesn't lock m_activeBodiesMutex) will
        // only read valid IDs.
        m_numActiveBodies.fetch_add(1, std::memory_order_release);

        // Update CCD bodies if applicable.
        if (pMotion->GetMotionQuality() == EBodyMotionQuality::LinearCast)
            ++m_numActiveCCDBodies;
    }

    void BodyManager::RemoveBodyFromActiveBodies(Body& body)
    {
        // [TODO]: Delineate between Rigid and Soft body types.
        //int type = static_cast<int>(body.GetBodyType());

        uint32_t lastBodyIndex = m_numActiveBodies.load(std::memory_order_relaxed) - 1;
        MotionProperties* pMotion = body.m_pMotionProperties;
        if (pMotion->m_indexInActiveBodies == lastBodyIndex)
        {
            // This is not the last body, use the last body to fill the hole.
            BodyID lastBodyID = m_pActiveBodies[lastBodyIndex];
            m_pActiveBodies[pMotion->m_indexInActiveBodies] = lastBodyID;

            // Update that body's index in the active list.
            Body& lastBody = *m_bodies[lastBodyID.GetIndex()];
            NES_ASSERT(lastBody.m_pMotionProperties->m_indexInActiveBodies == lastBodyIndex);
            lastBody.m_pMotionProperties->m_indexInActiveBodies = pMotion->m_indexInActiveBodies;
        }

        // Mark this body as no longer active.
        pMotion->m_indexInActiveBodies = Body::kInactiveIndex;

        // Remove the unused element from the active bodies list
        m_numActiveBodies.fetch_sub(1, std::memory_order_release);

        // Update CCD bodies if applicable
        if (pMotion->GetMotionQuality() == EBodyMotionQuality::LinearCast)
            --m_numActiveCCDBodies;
    }

    Body* BodyManager::RemoveBody(const BodyID& id)
    {
        // Get the body
        const uint32_t index = id.GetIndex();
        Body* pBody = m_bodies[index];

        NES_ASSERT(pBody->GetID() == id);
        NES_ASSERT(!pBody->IsActive());
        NES_ASSERT(!pBody->IsInBroadPhase(), "Use BodyInterface::RemoveBody to remove this body first");

        // Push the ID onto the free list.
        m_bodies[index] = reinterpret_cast<Body*>(m_bodyIDFreeListStart);
        m_bodyIDFreeListStart = (static_cast<uintptr_t>(index) << kFreedBodyIndexShift) | kIsFreedBody;

        return pBody;
    }

    inline void BodyManager::DeleteBody(Body* pBody)
    {
        if (pBody->m_pMotionProperties != nullptr)
        {
            NES_IF_LOGGING_ENABLED(pBody->m_pMotionProperties = nullptr);

            // [TODO]: Soft Body version.
            NES_DELETE(checked_cast<BodyWithMotionProperties*>(pBody));
        }
    }

#if NES_LOGGING_ENABLED
    void BodyManager::ValidateFreeList() const
    {
        // Check that the free list is correct.
        [[maybe_unused]] size_t numFreed = 0;
        for (uintptr_t start = m_bodyIDFreeListStart; start != kBodyIDFreeListEnd; start = reinterpret_cast<uintptr_t>(m_bodies[start >> kFreedBodyIndexShift]))
        {
            NES_ASSERT(start & kIsFreedBody);
            ++numFreed;
        }

        NES_ASSERT(m_numBodies == static_cast<uint32_t>(m_bodies.size() - numFreed));
    }
#endif
    
}
