// ContactConstraintManager.cpp
#include "ContactConstraintManager.h"

#include "Physics/Constraints/CalculateSolverSteps.h"
#include "Physics/Body/Body.h"
#include "Physics/PhysicsUpdateContext.h"
#include "Physics/PhysicsSettings.h"
#include "Physics/PhysicsScene.h"
#include "Physics/IslandBuilder.h"
#include "Core/Memory/StackAllocator.h"
#include "Core/QuickSort.h"

namespace nes
{
    using namespace literals;
    
    //----------------------------------------------------------------------------------------------------------------------------
    // ContactConstraintManager::WorldContactPoint
    //----------------------------------------------------------------------------------------------------------------------------

    void ContactConstraintManager::WorldContactPoint::CalculateNonPenetrationConstraintProperties(const Body& body1, const float invMass1, const float invInertiaScale1, const Body& body2, const float invMass2, const float invInertiaScale2, const RVec3 worldSpacePosition1, const RVec3 worldSpacePosition2, const Vec3 worldSpaceNormal)
    {
        // Calculate the collision points relative to the bodies.
        const RVec3 p = 0.5_r * (worldSpacePosition1 + worldSpacePosition2);
        const Vec3 r1 = Vec3(p - body1.GetCenterOfMassPosition());
        const Vec3 r2 = Vec3(p - body2.GetCenterOfMassPosition());

        m_nonPenetrationConstraint.CalculateConstraintPropertiesWithMassOverride(body1, invMass1, invInertiaScale1, r1, body2, invMass2, invInertiaScale2, r2, worldSpaceNormal);
    }

    template <EBodyMotionType Type1, EBodyMotionType Type2>
    void ContactConstraintManager::WorldContactPoint::TemplatedCalculateFrictionAndNonPenetrationConstraintProperties(float deltaTime, float inGravityDeltaTimeDotNormal, const Body& body1, const Body& body2, float invMass1, float invMass2, const Mat44& invI1, const Mat44& invI2, const RVec3 worldSpacePosition1, const RVec3 worldSpacePosition2, const Vec3 worldSpaceNormal, const Vec3 worldSpaceTangent1, const Vec3 worldSpaceTangent2, const ContactSettings& settings, float minVelocityForRestitution)
    {
        // Calculate the collision points relative to the bodies.
        const RVec3 p = 0.5_r * (worldSpacePosition1 + worldSpacePosition2);
        const Vec3 r1 = Vec3(p - body1.GetCenterOfMassPosition());
        const Vec3 r2 = Vec3(p - body2.GetCenterOfMassPosition());

        // The gravity is applied at the beginning of the time step. If we get here, there was a collision
        // at the beginning of the time step, so we've applied too much gravity. This means that our
        // calculated restitution can be too high, so when we apply restitution, we cancel the added velocity
        // due to gravity.
        float gravityDeltaDotNormal;

        // Calculate the velocity of the collision points.
        Vec3 relativeVelocity;
        if constexpr (Type1 != EBodyMotionType::Static && Type2 != EBodyMotionType::Static)
        {
            const MotionProperties* pMotionProps1 = body1.GetMotionPropertiesUnchecked();
            const MotionProperties* pMotionProps2 = body2.GetMotionPropertiesUnchecked();
            relativeVelocity = pMotionProps2->GetPointVelocityCOM(r2) - pMotionProps1->GetPointVelocityCOM(r1);
            gravityDeltaDotNormal = inGravityDeltaTimeDotNormal * (pMotionProps2->GetGravityScale() - pMotionProps1->GetGravityScale());
        }
        else if constexpr (Type1 != EBodyMotionType::Static)
        {
            const MotionProperties* pMotionProps1 = body1.GetMotionPropertiesUnchecked();
            relativeVelocity = -pMotionProps1->GetPointVelocityCOM(r1);
            gravityDeltaDotNormal = inGravityDeltaTimeDotNormal * pMotionProps1->GetGravityScale();
        }
        else if constexpr (Type2 != EBodyMotionType::Static)
        {
            const MotionProperties* pMotionProps2 = body2.GetMotionPropertiesUnchecked();
            relativeVelocity = pMotionProps2->GetPointVelocityCOM(r2);
            gravityDeltaDotNormal = inGravityDeltaTimeDotNormal * pMotionProps2->GetGravityScale();
        }
        else
        {
            NES_ASSERT(false); // Static vs. Static makes no sense.
            relativeVelocity = Vec3::Zero();
            gravityDeltaDotNormal = 0.0f;
        }

        const float normalVelocity = relativeVelocity.Dot(worldSpaceNormal);

        // How much the shapes are penetrating (> 0 if penetrating, < 0 if separated).
        float penetration = Vec3(worldSpacePosition1 - worldSpacePosition2).Dot(worldSpaceNormal);

        // If there is no penetration, this is a speculative contact, and we will apply a bias to the contact constraint
        // so that the constraint becomes (relativeVelocity . Contact normal > -penetration / delta_time)
        // instead of (relativeVelocity . Contact normal > 0)
        // See: GDC 2013: "Physics for Game Programmers; Continuous Collision" - Erin Catto
        const float speculativeContactVelocityBias = math::Max(0.f, -penetration / deltaTime);

        // Determine if the velocity is big enough for restitution.
        float normalVelocityBias;
        if (settings.m_combinedRestitution > 0.f && normalVelocity > -minVelocityForRestitution)
        {
            // We have a velocity that is big enough for restitution. This is where speculative contacts don't work
            // great as we have to decide now if we're going to apply the restitution or not. If the relative
            // velocity is big enough for a hit, we apply the restitution (in the end, due to other constraints,
            // the objects may actually not collide, and we will have applied restitution incorrectly). Another
            // artifact that occurs because of this approximation is that the object will bounce from its current
            // position rather than from a position where it is touching the other object. This causes the object
            // to appear to move faster for 1 frame (the opposite of time stealing).
            if (normalVelocity < -speculativeContactVelocityBias)
                normalVelocityBias = settings.m_combinedRestitution * (normalVelocity - gravityDeltaDotNormal);
            else
            {
                // In this case, we have predicted that we don't hit the other object, but if we do (due to other constraints changing velocities),
                // the speculative contact will prevent penetration but will not apply restitution leading to another artifact.
                normalVelocityBias = speculativeContactVelocityBias;
            }
        }
        else
        {
            // No restitution. We can safely apply our contact velocity bias.
            normalVelocityBias = speculativeContactVelocityBias;
        }

        m_nonPenetrationConstraint.TemplatedCalculateConstraintProperties<Type1, Type2>(invMass1, invI1, r1, invMass2, invI2, r2, worldSpaceNormal, normalVelocityBias);

        // Calculate the friction part
        if (settings.m_combinedFriction > 0.f)
        {
            // Get the surface velocity relative to the tangents.
            const Vec3 worldSpaceSurfaceVelocity = settings.m_relativeLinearSurfaceVelocity + settings.m_relativeAngularSurfaceVelocity.Cross(r1);
            const float surfaceVelocity1 = worldSpaceTangent1.Dot(worldSpaceSurfaceVelocity);
            const float surfaceVelocity2 = worldSpaceTangent2.Dot(worldSpaceSurfaceVelocity);

            // Implement friction as 2 AxisConstraintParts
            m_frictionConstraint1.TemplatedCalculateConstraintProperties<Type1, Type2>(invMass1, invI1, r1, invMass2, invI2, r2, worldSpaceTangent1, surfaceVelocity1);
            m_frictionConstraint2.TemplatedCalculateConstraintProperties<Type1, Type2>(invMass1, invI1, r1, invMass2, invI2, r2, worldSpaceTangent2, surfaceVelocity2);
        }
        else
        {
            // Turn off friction constraint
            m_frictionConstraint1.Deactivate();
            m_frictionConstraint2.Deactivate();
        }
    }

    //----------------------------------------------------------------------------------------------------------------------------
    // ContactConstraintManager::ManifoldCache
    //----------------------------------------------------------------------------------------------------------------------------

    void ContactConstraintManager::ManifoldCache::Init(const uint inMaxBodyPairs, const uint maxContactConstraints, const uint cachedManifoldSize)
    {
        const uint maxBodyPairs = math::Min(inMaxBodyPairs, kMaxBodyPairsLimit);
        NES_ASSERT(maxBodyPairs == inMaxBodyPairs, "Cannot support this many body pairs!");
        NES_ASSERT(maxContactConstraints <= kMaxContactConstraintsLimit); // Should have been enforced by caller

        m_allocator.Init(static_cast<uint>(math::Min(static_cast<uint64>(maxBodyPairs) * sizeof(BodyPairMap::KeyValuePair) + cachedManifoldSize, static_cast<uint64>(~static_cast<uint>(0)))));

        m_cachedManifolds.Init(math::GetNextPowerOf2(maxContactConstraints));
        m_cachedBodyPairs.Init(math::GetNextPowerOf2(maxBodyPairs));
    }

    void ContactConstraintManager::ManifoldCache::Clear()
    {
        m_cachedManifolds.Clear();
        m_cachedBodyPairs.Clear();
        m_allocator.Clear();

    #ifdef NES_ASSERTS_ENABLED
        // Mark as incomplete
        m_isFinalized = false;
    #endif
    }

    void ContactConstraintManager::ManifoldCache::Prepare(const uint expectedNumBodyPairs, const uint expectedNumManifolds)
    {
        // Minimum amount of buckets to use in the hash map.
        constexpr uint32 kMinBuckets = 1024;

        // Use the next higher power of 2 of number of objects in the cache from the last frame to determine the number of buckets to
        // use this frame.
        m_cachedManifolds.SetNumBuckets(math::Min(math::Max(kMinBuckets, math::GetNextPowerOf2(expectedNumManifolds)), m_cachedManifolds.GetMaxBuckets()));
        m_cachedBodyPairs.SetNumBuckets(math::Min(math::Max(kMinBuckets, math::GetNextPowerOf2(expectedNumBodyPairs)), m_cachedBodyPairs.GetMaxBuckets()));
    }

    const ContactConstraintManager::MKeyValue* ContactConstraintManager::ManifoldCache::Find(const SubShapeIDPair& key, uint64 keyHash) const
    {
        NES_ASSERT(m_isFinalized);
        return m_cachedManifolds.Find(key, keyHash);
    }

    ContactConstraintManager::MKeyValue* ContactConstraintManager::ManifoldCache::Create(ContactAllocator& contactAllocator, const SubShapeIDPair& key, const uint64 keyHash, const int numContactPoints)
    {
        NES_ASSERT(!m_isFinalized);
        MKeyValue* pKeyValue = m_cachedManifolds.Create(contactAllocator, key, keyHash, CachedManifold::GetRequiredExtraSize(numContactPoints));
        if (pKeyValue == nullptr)
        {
            contactAllocator.m_errors |= EPhysicsUpdateErrorCode::ManifoldCacheFull;
            return nullptr;
        }

        pKeyValue->GetValue().m_numContactPoints = static_cast<uint16>(numContactPoints);
        ++contactAllocator.m_numManifolds;
        return pKeyValue;
    }

    ContactConstraintManager::MKeyValueAndCreated ContactConstraintManager::ManifoldCache::FindOrCreate(ContactAllocator& contactAllocator, const SubShapeIDPair& key, uint64 keyHash, const int numContactPoints)
    {
        MKeyValue* pKeyValue = const_cast<MKeyValue*>(m_cachedManifolds.Find(key, keyHash));
        if (pKeyValue != nullptr)
            return { pKeyValue, false };

        return { Create(contactAllocator, key, keyHash, numContactPoints), true };
    }

    uint32 ContactConstraintManager::ManifoldCache::ToHandle(const MKeyValue* keyValue) const
    {
        NES_ASSERT(!m_isFinalized);
        return m_cachedManifolds.ToHandle(keyValue);
    }

    const ContactConstraintManager::MKeyValue* ContactConstraintManager::ManifoldCache::FromHandle(const uint32 handle) const
    {
        NES_ASSERT(m_isFinalized);
        return m_cachedManifolds.FromHandle(handle);
    }

    const ContactConstraintManager::BPKeyValue* ContactConstraintManager::ManifoldCache::Find(const BodyPair& key, const uint64 keyHash) const
    {
        NES_ASSERT(m_isFinalized);
        return m_cachedBodyPairs.Find(key, keyHash);
    }

    ContactConstraintManager::BPKeyValue* ContactConstraintManager::ManifoldCache::Create(ContactAllocator& contactAllocator, const BodyPair& key, uint64 keyHash)
    {
        NES_ASSERT(!m_isFinalized);
        BPKeyValue* pKeyValue = m_cachedBodyPairs.Create(contactAllocator, key, keyHash, 0);
        if (pKeyValue == nullptr)
        {
            contactAllocator.m_errors |= EPhysicsUpdateErrorCode::BodyPairCacheFull;
            return nullptr;
        }

        ++contactAllocator.m_numBodyPairs;
        return pKeyValue;
    }

    void ContactConstraintManager::ManifoldCache::GetAllBodyPairsSorted(std::vector<const BPKeyValue*>& outAll) const
    {
        NES_ASSERT(m_isFinalized);
        m_cachedBodyPairs.GetAllKeyValuePairs(outAll);

        // Sort by Key
        QuickSort(outAll.begin(), outAll.end(), [](const BPKeyValue* pLeft, const BPKeyValue* pRight)
        {
            return pLeft->GetKey() < pRight->GetKey();
        });
    }

    void ContactConstraintManager::ManifoldCache::GetAllManifoldsSorted(const CachedBodyPair& bodyPair, std::vector<const MKeyValue*>& outAll) const
    {
        NES_ASSERT(m_isFinalized);

        // Iterate through the attached manifolds
        for (uint32 handle = bodyPair.m_firstCachedManifold; handle != ManifoldMap::kInvalidHandle; handle = FromHandle(handle)->GetValue().m_nextWithSameBodyPair)
        {
            const MKeyValue* pKeyValue = m_cachedManifolds.FromHandle(handle);
            outAll.push_back(pKeyValue);
        }

        // Sort by key
        QuickSort(outAll.begin(), outAll.end(), [](const MKeyValue* pLeft, const MKeyValue* pRight)
        {
            return pLeft->GetKey() < pRight->GetKey();
        });
    }

    void ContactConstraintManager::ManifoldCache::GetAllCCDManifoldsSorted(std::vector<const MKeyValue*>& outAll) const
    {
        m_cachedManifolds.GetAllKeyValuePairs(outAll);

        for (int i = static_cast<int>(outAll.size() - 1); i >= 0; --i)
        {
            // Remove if not CCD Contact
            if ((outAll[i]->GetValue().m_flags & static_cast<uint16>(CachedManifold::EFlags::CCDContact)) == 0)
            {
                outAll[i] = outAll.back();
                outAll.pop_back();
            }
        }

        // Sort by Key
        QuickSort(outAll.begin(), outAll.end(), [](const MKeyValue* pLeft, const MKeyValue* pRight)
        {
            return pLeft->GetKey() < pRight->GetKey();
        });
    }

    void ContactConstraintManager::ManifoldCache::ContactPointRemovedCallbacks(ContactListener* pListener)
    {
        for (MKeyValue& keyValue : m_cachedManifolds)
        {
            if ((keyValue.GetValue().m_flags & static_cast<uint16>(CachedManifold::EFlags::ContactPersisted)) == 0)
            {
                pListener->OnContactRemoved(keyValue.GetKey());
            }
        }
    }

    #ifdef NES_ASSERTS_ENABLED
    void ContactConstraintManager::ManifoldCache::Finalize()
    {
        m_isFinalized = true;

        // [TODO]: Trace Stats.
    }
    #endif

    //----------------------------------------------------------------------------------------------------------------------------
    // ContactConstraintManager
    //----------------------------------------------------------------------------------------------------------------------------
    
    ContactConstraintManager::ContactConstraintManager(const PhysicsSettings& settings)
        : m_physicsSettings(settings)
    {
    #ifdef NES_ASSERTS_ENABLED
        // For the first frame, mark this empty buffer as finalized.
        m_cache[m_cacheWriteIndex ^ 1].Finalize();
    #endif
    }

    ContactConstraintManager::~ContactConstraintManager()
    {
        NES_ASSERT(m_constraints == nullptr);
    }

    void ContactConstraintManager::Init(uint32_t maxBodyPairs, uint32_t maxContactConstraints)
    {
        // Limit the number of constraints so that the allocation size fits in an unsigned integer.
        m_maxConstraints = math::Min(maxContactConstraints, kMaxContactConstraintsLimit);
        NES_ASSERT(m_maxConstraints == maxContactConstraints, "Cannot support this many contact constraints!");

        // Calculate the worst case cache usage
        constexpr uint kMaxManifoldSizePerConstraint = sizeof(CachedManifold) + (kMaxContactPoints - 1) * sizeof(CachedContactPoint);
        static_assert(kMaxManifoldSizePerConstraint < sizeof(ContactConstraint)); // If not true, then the next line can overflow
        const uint cachedManifoldsSize = m_maxConstraints * kMaxManifoldSizePerConstraint;

        // Init the caches
        m_cache[0].Init(maxBodyPairs, m_maxConstraints, cachedManifoldsSize);
        m_cache[1].Init(maxBodyPairs, m_maxConstraints, cachedManifoldsSize);
    }

    void ContactConstraintManager::PrepareConstraintBuffer(PhysicsUpdateContext* pContext)
    {
        // Store the context
        m_pUpdateContext = pContext;

        // Allocate temporary constraint buffer
        NES_ASSERT(m_constraints == nullptr);
        m_constraints = pContext->m_pAllocator->Allocate<ContactConstraint>(m_maxConstraints);
    }

    template <EBodyMotionType Type1, EBodyMotionType Type2>
    NES_INLINE void ContactConstraintManager::TemplatedCalculateFrictionAndNonPenetrationConstraintProperties(ContactConstraint& constraint, const ContactSettings& settings, float deltaTime, const Vec3 gravityDeltaTime, const Mat44& transformBody1, const Mat44& transformBody2, const Body& body1, const Body& body2)
    {
        // Calculate scaled mass and inertia
        Mat44 invI1;
        if constexpr (Type1 == EBodyMotionType::Dynamic)
        {
            const MotionProperties* pMotionProps1 = body1.GetMotionPropertiesUnchecked();
            invI1 = settings.m_inverseInertiaScale1 * pMotionProps1->GetInverseInertiaForRotation(transformBody1.GetRotation());
        }
        else
        {
            invI1 = Mat44::Zero();
        }

        Mat44 invI2;
        if constexpr (Type2 == EBodyMotionType::Dynamic)
        {
            const MotionProperties* pMotionProps2 = body2.GetMotionPropertiesUnchecked();
            invI2 = settings.m_inverseInertiaScale2 * pMotionProps2->GetInverseInertiaForRotation(transformBody2.GetRotation());
        }
        else
        {
            invI2 = Mat44::Zero();
        }

        // Calculate tangents
        Vec3 t1, t2;
        constraint.GetTangents(t1, t2);

        const Vec3 wsNormal = constraint.GetWorldSpaceNormal();

        // Calculate value for restitution correction
        const float gravityDeltaTimeDotNormal = gravityDeltaTime.Dot(wsNormal);

        // Setup velocity constraint properties
        const float minVelocityForRestitution = m_physicsSettings.m_minVelocityForRestitution;
        for (WorldContactPoint& wcp : constraint.m_contactPoints)
        {
            RVec3 p1 = transformBody1 * Vec3::LoadFloat3Unsafe(wcp.m_pContactPoint->m_position1);
            RVec3 p2 = transformBody2 * Vec3::LoadFloat3Unsafe(wcp.m_pContactPoint->m_position2);
            wcp.TemplatedCalculateFrictionAndNonPenetrationConstraintProperties<Type1, Type2>(deltaTime, gravityDeltaTimeDotNormal, body1, body2, constraint.m_inverseMass1, constraint.m_inverseMass2, invI1, invI2, p1, p2, wsNormal, t1, t2, settings, minVelocityForRestitution);
        }
    }

    inline void ContactConstraintManager::CalculateFrictionAndNonPenetrationConstraintProperties(ContactConstraint& constraint, const ContactSettings& settings, float deltaTime, const Vec3 gravityDeltaTime, const Mat44& transformBody1, const Mat44& transformBody2, const Body& body1, const Body& body2)
    {
        // Dispatch to the correct templated form:
        switch (body1.GetMotionType())
        {
            case EBodyMotionType::Dynamic:
            {
                switch (body2.GetMotionType())
                {
                    case EBodyMotionType::Dynamic:
                    {
                        TemplatedCalculateFrictionAndNonPenetrationConstraintProperties<EBodyMotionType::Dynamic, EBodyMotionType::Dynamic>(constraint, settings, deltaTime, gravityDeltaTime, transformBody1, transformBody2, body1, body2);
                        break;
                    }

                    case EBodyMotionType::Kinematic:
                    {
                        TemplatedCalculateFrictionAndNonPenetrationConstraintProperties<EBodyMotionType::Dynamic, EBodyMotionType::Kinematic>(constraint, settings, deltaTime, gravityDeltaTime, transformBody1, transformBody2, body1, body2);
                        break;
                    }

                    case EBodyMotionType::Static:
                    {
                        TemplatedCalculateFrictionAndNonPenetrationConstraintProperties<EBodyMotionType::Dynamic, EBodyMotionType::Static>(constraint, settings, deltaTime, gravityDeltaTime, transformBody1, transformBody2, body1, body2);
                        break;
                    }

                    default:
                    {
                        NES_ASSERT(false);
                        break;
                    }
                }
                break;
            }

            case EBodyMotionType::Kinematic:
            {
                NES_ASSERT(body2.IsDynamic());
                TemplatedCalculateFrictionAndNonPenetrationConstraintProperties<EBodyMotionType::Kinematic, EBodyMotionType::Dynamic>(constraint, settings, deltaTime, gravityDeltaTime, transformBody1, transformBody2, body1, body2);
                break;
            }

            case EBodyMotionType::Static:
            {
                NES_ASSERT(body2.IsDynamic());
                TemplatedCalculateFrictionAndNonPenetrationConstraintProperties<EBodyMotionType::Static, EBodyMotionType::Dynamic>(constraint, settings, deltaTime, gravityDeltaTime, transformBody1, transformBody2, body1, body2);
                break;
            }

            default:
            {
                NES_ASSERT(false);
                break;
            }
        }
    }

    void ContactConstraintManager::GetContactsFromCache(ContactAllocator& contactAllocator, Body& body1, Body& body2, bool& outPairHandled, bool& outConstraintCreated)
    {
        // Start with nothing found and not handled.
        outConstraintCreated = false;
        outPairHandled = false;

        // Swap bodies so that body 1 id < body 2 id.
        Body* pBody1;
        Body* pBody2;
        if (body1.GetID() < body2.GetID())
        {
            pBody1 = &body1;
            pBody2 = &body2;
        }
        else
        {
            pBody1 = &body2;
            pBody2 = &body1;
        }

        // Find the Cached Body Pair
        BodyPair bodyPairKey(pBody1->GetID(), pBody2->GetID());
        const uint64 bodyPairHash = bodyPairKey.GetHash();
        const ManifoldCache& readCache = m_cache[m_cacheWriteIndex ^ 1];
        const BPKeyValue* pKeyValue = readCache.Find(bodyPairKey, bodyPairHash);
        if (pKeyValue == nullptr)
            return;
        const CachedBodyPair& inputCBP = pKeyValue->GetValue();

        // Get relative translation
        Quat invR1 = pBody1->GetRotation().Conjugate();
        Vec3 deltaPosition = invR1 * Vec3(pBody2->GetCenterOfMassPosition() - pBody1->GetCenterOfMassPosition());

        // Get the old position delta
        Vec3 oldDeltaPosition = Vec3::LoadFloat3Unsafe(inputCBP.m_deltaPosition);

        // Check if bodies are still roughly in the same relative position.
        if ((deltaPosition - oldDeltaPosition).LengthSqr() > m_physicsSettings.m_bodyPairCacheMaxDeltaPositionSqr)
            return;

        // Determine relative orientation
        Quat deltaRotation = invR1 * pBody2->GetRotation();

        // Reconstruct old quaternion delta
        Quat oldDeltaRotation = Quat::LoadFloat3Unsafe(inputCBP.m_deltaRotation);

        // Check if bodies are still roughly in the same relative orientation
        // The delta between 2 quaternions p and q is: p q^* = [rotation_axis * sin(angle / 2), cos(angle / 2)]
        // From the W component we can extract the angle: cos(angle / 2) = px * qx + py * qy + pz * qz + pw * qw = p . q
        // Since we want to abort if the rotation is smaller than -angle or bigger than angle, we can write the comparison as |p . q| < cos(angle / 2)
        if (math::Abs(deltaRotation.Dot(oldDeltaRotation)) < m_physicsSettings.m_bodyPairCacheCosMaxDeltaRotationDiv2)
            return;

        // The cache is valid; mark that we've successfully handled this body pair.
        outPairHandled = true;

        // Copy the cached body pair to this frame
        ManifoldCache& writeCache = m_cache[m_cacheWriteIndex];
        BPKeyValue* pOutputBPKeyValue = writeCache.Create(contactAllocator, bodyPairKey, bodyPairHash);
        if (pOutputBPKeyValue == nullptr)
            return; // Out of cache space
        CachedBodyPair* pOutputCBP = &pOutputBPKeyValue->GetValue();
        memcpy(pOutputCBP, &inputCBP, sizeof(CachedBodyPair));

        // If there were no contacts, we have handled the contact
        if (inputCBP.m_firstCachedManifold == ManifoldMap::kInvalidHandle)
            return;

        // Get the body transforms
        Mat44 transformBody1 = pBody1->GetCenterOfMassTransform();
        Mat44 transformBody2 = pBody2->GetCenterOfMassTransform();

        // Get the time step
        const float deltaTime = m_pUpdateContext->m_stepDeltaTime;

        // Calculate value for the restitution correction
        const Vec3 gravityDeltaTime = m_pUpdateContext->m_pPhysicsScene->GetGravity() * deltaTime;

        // Copy manifolds
        uint32 outputHandle = ManifoldMap::kInvalidHandle;
        uint32 inputHandle = inputCBP.m_firstCachedManifold;
        do
        {
            // Find the existing manifold
            const MKeyValue* pInputKV = readCache.FromHandle(inputHandle);
            const SubShapeIDPair& inputKey = pInputKV->GetKey();
            const CachedManifold& inputCM = pInputKV->GetValue();
            NES_ASSERT(inputCM.m_numContactPoints > 0); // There should be contact points in the manifold!

            // Create room for the manifold in the write buffer and copy the data.
            uint64 inputHash = inputKey.GetHash();
            MKeyValue* pOutputKV = writeCache.Create(contactAllocator, inputKey, inputHash, inputCM.m_numContactPoints);
            if (pOutputKV == nullptr)
                break; // Out of cache space
            CachedManifold* pOutputCM = &pOutputKV->GetValue();
            memcpy(pOutputCM, &inputCM, CachedManifold::GetRequiredTotalSize(inputCM.m_numContactPoints));

            // Link the object under the body pairs
            pOutputCM->m_nextWithSameBodyPair = outputHandle;
            outputHandle = writeCache.ToHandle(pOutputKV);

            // Calculate default contact settings
            ContactSettings settings;
            settings.m_combinedFriction = m_combineFriction(*pBody1, inputKey.GetSubShape1ID(), *pBody2, inputKey.GetSubShape2ID());
            settings.m_combinedRestitution = m_combineRestitution(*pBody1, inputKey.GetSubShape1ID(), *pBody2, inputKey.GetSubShape2ID());
            settings.m_isSensor = pBody1->IsSensor() || pBody2->IsSensor();

            // Calculate world space contact normal
            const Vec3 worldSpaceNormal = transformBody2.Multiply3x3(Vec3::LoadFloat3Unsafe(pOutputCM->m_contactNormal)).Normalized();

            // Call contact listener to update settings
            if (m_pContactListener != nullptr)
            {
                // Convert constraint to manifold structure for callback.
                ContactManifold manifold;
                manifold.m_worldSpaceNormal = worldSpaceNormal;
                manifold.m_subShapeID1 = inputKey.GetSubShape1ID();
                manifold.m_subShapeID2 = inputKey.GetSubShape2ID();
                manifold.m_baseOffset = transformBody1.GetTranslation();
                manifold.m_relativeContactPointsOn1.resize(pOutputCM->m_numContactPoints);
                manifold.m_relativeContactPointsOn2.resize(pOutputCM->m_numContactPoints);
                Mat44 localTransformBody2 = transformBody2.PostTranslated(-manifold.m_baseOffset);
                float penetrationDepth = -FLT_MAX;
                for (uint32 i = 0; i < pOutputCM->m_numContactPoints; ++i)
                {
                    const CachedContactPoint& ccp = pOutputCM->m_contactPoints[i];
                    manifold.m_relativeContactPointsOn1[i] = transformBody1.Multiply3x3(Vec3::LoadFloat3Unsafe(ccp.m_position1));
                    manifold.m_relativeContactPointsOn2[i] = localTransformBody2 * Vec3::LoadFloat3Unsafe(ccp.m_position2);
                    penetrationDepth = math::Max(penetrationDepth, (manifold.m_relativeContactPointsOn1[i] - manifold.m_relativeContactPointsOn2[i]).Dot(worldSpaceNormal));
                }
                manifold.m_penetrationDepth = penetrationDepth; // We don't have the penetration depth anymore, estimate it.

                // Notify callback
                m_pContactListener->OnContactPersisted(*pBody1, *pBody2, manifold, settings);
            }

            // Try to add the constraint:
            NES_ASSERT(settings.m_isSensor || !(pBody1->IsSensor() || pBody2->IsSensor()), "Sensors cannot be converted into regular bodies by a contact callback!");
            if (!settings.m_isSensor    // If one of the bodies is a sensor, don't create a constraint.
                && ((pBody1->IsDynamic() && settings.m_inverseMassScale1 != 0.f) // One of the bodies must have a mass to be able to create the contact constraint.
                || (pBody2->IsDynamic() && settings.m_inverseMassScale2 != 0.f)))
            {
                // Add the contact constraint in world space for the solver.
                const uint32 constraintIndex = m_numConstraints++;
                if (constraintIndex >= m_maxConstraints)
                {
                    contactAllocator.m_errors |= EPhysicsUpdateErrorCode::ContactConstraintsFull;
                    break;
                }

                // A constraint will be created
                outConstraintCreated = true;

                ContactConstraint& constraint = m_constraints[constraintIndex];
                new (&constraint) ContactConstraint();
                constraint.m_pBody1 = pBody1;
                constraint.m_pBody2 = pBody2;
                constraint.m_sortKey = inputHash;
                worldSpaceNormal.StoreFloat3(&constraint.m_worldSpaceNormal);
                constraint.m_combinedFriction = settings.m_combinedFriction;
                constraint.m_inverseMass1 = pBody1->GetMotionPropertiesUnchecked() != nullptr? settings.m_inverseMassScale1 * pBody1->GetMotionPropertiesUnchecked()->GetInverseMassUnchecked() : 0.f;
                constraint.m_inverseInertiaScale1 = settings.m_inverseInertiaScale1;
                constraint.m_inverseMass2 = pBody2->GetMotionPropertiesUnchecked() != nullptr? settings.m_inverseMassScale2 * pBody2->GetMotionPropertiesUnchecked()->GetInverseMassUnchecked() : 0.f;
                constraint.m_inverseInertiaScale2 = settings.m_inverseInertiaScale2;
                constraint.m_contactPoints.resize(pOutputCM->m_numContactPoints);
                for (uint32 i = 0; i < pOutputCM->m_numContactPoints; ++i)
                {
                    CachedContactPoint& ccp = pOutputCM->m_contactPoints[i];
                    WorldContactPoint& wcp = constraint.m_contactPoints[i];
                    wcp.m_nonPenetrationConstraint.SetTotalLambda(ccp.m_nonPenetrationLambda);
                    wcp.m_frictionConstraint1.SetTotalLambda(ccp.m_frictionLambda[0]);
                    wcp.m_frictionConstraint2.SetTotalLambda(ccp.m_frictionLambda[1]);
                    wcp.m_pContactPoint = &ccp;
                }

                // Calculate the friction and non-penetration constraint properties for all contact points.
                CalculateFrictionAndNonPenetrationConstraintProperties(constraint, settings, deltaTime, gravityDeltaTime, transformBody1, transformBody2, *pBody1, *pBody2);

                // Notify the island builder.
                m_pUpdateContext->m_pIslandBuilder->LinkContact(constraintIndex, pBody1->Internal_GetIndexInActiveBodies(), pBody2->Internal_GetIndexInActiveBodies());
            }

            // Mark the contact as persisted so that we won't fire OnContactRemoved callbacks.
            inputCM.m_flags |= static_cast<uint16>(CachedManifold::EFlags::ContactPersisted);

            // Fetch the next manifold.
            inputHandle = inputCM.m_nextWithSameBodyPair;
        }
        while (inputHandle != ManifoldMap::kInvalidHandle);

        pOutputCBP->m_firstCachedManifold = outputHandle;
    }

    ContactConstraintManager::BodyPairHandle ContactConstraintManager::AddBodyPair(ContactAllocator& contactAllocator, const Body& body1, const Body& body2)
    {
        // Swap bodies so that body 1 id < body 2 id.
        const Body* pBody1;
        const Body* pBody2;
        if (body1.GetID() < body2.GetID())
        {
            pBody1 = &body1;
            pBody2 = &body2;
        }
        else
        {
            pBody1 = &body2;
            pBody2 = &body1;
        }

        // Add an entry
        const BodyPair bodyPairKey(pBody1->GetID(), pBody2->GetID());
        const uint64 bodyPairHash = bodyPairKey.GetHash();
        BPKeyValue* pBodyPairKeyValue = m_cache[m_cacheWriteIndex].Create(contactAllocator, bodyPairKey, bodyPairHash);
        if (pBodyPairKeyValue == nullptr)
            return nullptr; // Out of cache space
        CachedBodyPair* pCBP = &pBodyPairKeyValue->GetValue();
        pCBP->m_firstCachedManifold = ManifoldMap::kInvalidHandle;

        // Get relative translation
        const Quat invR1 = pBody1->GetRotation().Conjugate();
        const Vec3 deltaPosition = invR1 * Vec3(pBody2->GetCenterOfMassPosition() - pBody1->GetCenterOfMassPosition());

        // Store it
        deltaPosition.StoreFloat3(&pCBP->m_deltaPosition);

        // Determine the relative orientation
        const Quat deltaRotation = invR1 * pBody2->GetRotation();

        // Store it
        deltaRotation.StoreFloat3(&pCBP->m_deltaRotation);

        return pCBP;
    }

    template <EBodyMotionType Type1, EBodyMotionType Type2>
    bool ContactConstraintManager::TemplatedAddContactConstraint(ContactAllocator& contactAllocator, BodyPairHandle bodyPairHandle, Body& body1, Body& body2, const ContactManifold& manifold)
    {
        // Calculate hash
        SubShapeIDPair key { body1.GetID(), manifold.m_subShapeID1, body2.GetID(), manifold.m_subShapeID2};
        const uint64 keyHash = key.GetHash();

        // Determine the number of contact points
        const int numContactPoints = static_cast<int>(manifold.m_relativeContactPointsOn1.size());
        NES_ASSERT(numContactPoints <= kMaxContactPoints);
        NES_ASSERT(numContactPoints == static_cast<int>(manifold.m_relativeContactPointsOn2.size()));

        // Reserve space for new contact cache entry
        // Note that for dynamic vs. dynamic we always require the first body to have a lower body id to get a consistent key
        // under which to look up the contact.
        ManifoldCache& writeCache = m_cache[m_cacheWriteIndex];
        MKeyValue* pNewManifoldKeyValue = writeCache.Create(contactAllocator, key, keyHash, numContactPoints);
        if (pNewManifoldKeyValue == nullptr)
            return false; // Out of cache space.
        CachedManifold* pNewManifold = &pNewManifoldKeyValue->GetValue();

        // Transform the world space normal to the space of body 2 (this is usually the static body).
        const Mat44 invTransformBody2 = body2.GetInverseCenterOfMassTransform();
        invTransformBody2.Multiply3x3(manifold.m_worldSpaceNormal).Normalize().StoreFloat3(&pNewManifold->m_contactNormal);

        // Settings object that gets passed to the callback.
        ContactSettings settings;
        settings.m_combinedFriction = m_combineFriction(body1, manifold.m_subShapeID1, body2, manifold.m_subShapeID2);
        settings.m_combinedRestitution = m_combineRestitution(body1, manifold.m_subShapeID1, body2, manifold.m_subShapeID2);
        settings.m_isSensor = body1.IsSensor() || body2.IsSensor();

        // Get the contact points for the old cache entry.
        const ManifoldCache& readCache = m_cache[m_cacheWriteIndex ^ 1];
        const MKeyValue* pOldManifoldKeyValue = readCache.Find(key, keyHash);
        const CachedContactPoint* pCCPStart;
        const CachedContactPoint* pCCPEnd;
        if (pOldManifoldKeyValue != nullptr)
        {
            // Call point persisted listener
            if (m_pContactListener != nullptr)
                m_pContactListener->OnContactPersisted(body1, body2, manifold, settings);

            // Fetch the contact points for the old manifold.
            const CachedManifold* pOldManifold = &pOldManifoldKeyValue->GetValue();
            pCCPStart = pOldManifold->m_contactPoints;
            pCCPEnd = pCCPStart + pOldManifold->m_numContactPoints;
        }
        else
        {
            // Call the contact added listener
            if (m_pContactListener != nullptr)
                m_pContactListener->OnContactAdded(body1, body2, manifold, settings);

            // No contact points are available from the old manifold.
            pCCPStart = nullptr;
            pCCPEnd = nullptr;
        }

        // Get the inverse transform for body 1
        Mat44 invTransformBody1 = body1.GetInverseCenterOfMassTransform();

        bool contactConstraintCreated = false;
        
        // Try to add the constraint:
        NES_ASSERT(settings.m_isSensor || !(body1.IsSensor() || body2.IsSensor()), "Sensors cannot be converted into regular bodies by a contact callback!");
        if (!settings.m_isSensor    // If one of the bodies is a sensor, don't create a constraint.
            && ((body1.IsDynamic() && settings.m_inverseMassScale1 != 0.f) // One of the bodies must have a mass to be able to create the contact constraint.
            || (body2.IsDynamic() && settings.m_inverseMassScale2 != 0.f)))
        {
            // Add the contact constraint
            const uint32 constraintIndex = m_numConstraints++;
            if (constraintIndex >= m_maxConstraints)
            {
                contactAllocator.m_errors |= EPhysicsUpdateErrorCode::ContactConstraintsFull;

                // Manifold has been created already, we're not filling it in, so we need to reset the contact number of points.
                // Note that we don't hook it up to the body pair cache so that it won't be used as a cache during the next simulation.
                pNewManifold->m_numContactPoints = 0;
                return false;
            }

            // We will create a contact constraint.
            contactConstraintCreated = true;

            ContactConstraint& constraint = m_constraints[constraintIndex];
            new (&constraint) ContactConstraint();
            constraint.m_pBody1 = &body1;
            constraint.m_pBody2 = &body2;
            constraint.m_sortKey = keyHash;
            manifold.m_worldSpaceNormal.StoreFloat3(&constraint.m_worldSpaceNormal);
            constraint.m_combinedFriction = settings.m_combinedFriction;
            constraint.m_inverseMass1 = body1.GetMotionPropertiesUnchecked() != nullptr? settings.m_inverseMassScale1 * body1.GetMotionPropertiesUnchecked()->GetInverseMassUnchecked() : 0.f;
            constraint.m_inverseMass2 = body2.GetMotionPropertiesUnchecked() != nullptr? settings.m_inverseMassScale2 * body2.GetMotionPropertiesUnchecked()->GetInverseMassUnchecked() : 0.f;
            constraint.m_inverseInertiaScale1 = settings.m_inverseInertiaScale1;
            constraint.m_inverseInertiaScale2 = settings.m_inverseInertiaScale2;

            // Notify the island builder
            m_pUpdateContext->m_pIslandBuilder->LinkContact(constraintIndex, body1.Internal_GetIndexInActiveBodies(), body2.Internal_GetIndexInActiveBodies());

            // Get the timestep
            const float deltaTime = m_pUpdateContext->m_stepDeltaTime;

            // Calculate value for restitution correction
            const float gravityDTDotNormal = manifold.m_worldSpaceNormal.Dot(m_pUpdateContext->m_pPhysicsScene->GetGravity() * deltaTime);

            // Calculate scaled mass and inertia
            float invMass1;
            Mat44 invI1;
            if constexpr (Type1 == EBodyMotionType::Dynamic)
            {
                const MotionProperties* pMotionProps1 = body1.GetMotionPropertiesUnchecked();
                invMass1 = settings.m_inverseMassScale1 * pMotionProps1->GetInverseMassUnchecked();
                invI1 = settings.m_inverseInertiaScale1 * pMotionProps1->GetInverseInertiaForRotation(invTransformBody1.Transposed3x3());
            }
            else
            {
                invMass1 = 0.f;
                invI1 = Mat44::Zero();
            }

            float invMass2;
            Mat44 invI2;
            if constexpr (Type2 == EBodyMotionType::Dynamic)
            {
                const MotionProperties* pMotionProps2 = body2.GetMotionPropertiesUnchecked();
                invMass2 = settings.m_inverseMassScale2 * pMotionProps2->GetInverseMassUnchecked();
                invI2 = settings.m_inverseInertiaScale2 * pMotionProps2->GetInverseInertiaForRotation(invTransformBody2.Transposed3x3());
            }
            else
            {
                invMass2 = 0.f;
                invI2 = Mat44::Zero();
            }

            // Calculate the tangents
            Vec3 t1, t2;
            constraint.GetTangents(t1, t2);

            constraint.m_contactPoints.resize(numContactPoints);
            for (int i = 0; i < numContactPoints; i++)
            {
                // Convert to world space and set positions:
                WorldContactPoint& wcp = constraint.m_contactPoints[i];
                const RVec3 p1WorldSpace = manifold.m_baseOffset + manifold.m_relativeContactPointsOn1[i];
                const RVec3 p2WorldSpace = manifold.m_baseOffset + manifold.m_relativeContactPointsOn2[i];

                // Convert to local space to the body
                const Vec3 p1LocalSpace = Vec3(invTransformBody1 * p1WorldSpace);
                const Vec3 p2LocalSpace = Vec3(invTransformBody2 * p2WorldSpace);

                // Check if we have a close contact point from the last update.
                bool lambdaSet = false;
                for (const CachedContactPoint* pCCP = pCCPStart; pCCP < pCCPEnd; ++pCCP)
                {
                    if (Vec3::LoadFloat3Unsafe(pCCP->m_position1).IsClose(p1LocalSpace, m_physicsSettings.m_contactNormalPreserveLambdaMaxDistSqr)
                        && Vec3::LoadFloat3Unsafe(pCCP->m_position2).IsClose(p2LocalSpace, m_physicsSettings.m_contactNormalPreserveLambdaMaxDistSqr))
                    {
                        // Get lambdas from the previous frame
                        wcp.m_nonPenetrationConstraint.SetTotalLambda(pCCP->m_nonPenetrationLambda);
                        wcp.m_frictionConstraint1.SetTotalLambda(pCCP->m_frictionLambda[0]);
                        wcp.m_frictionConstraint2.SetTotalLambda(pCCP->m_frictionLambda[1]);
                        lambdaSet = true;
                        break;
                    }
                }

                if (!lambdaSet)
                {
                    wcp.m_nonPenetrationConstraint.SetTotalLambda(0.f);
                    wcp.m_frictionConstraint1.SetTotalLambda(0.f);
                    wcp.m_frictionConstraint2.SetTotalLambda(0.f);
                }

                // Create the new contact point
                CachedContactPoint& cp = pNewManifold->m_contactPoints[i];
                p1LocalSpace.StoreFloat3(&cp.m_position1);
                p2LocalSpace.StoreFloat3(&cp.m_position2);
                wcp.m_pContactPoint = &cp;

                // Setup velocity constraint
                wcp.TemplatedCalculateFrictionAndNonPenetrationConstraintProperties<Type1, Type2>(deltaTime, gravityDTDotNormal, body1, body2, invMass1, invMass2, invI1, invI2, p1WorldSpace, p2WorldSpace, manifold.m_worldSpaceNormal, t1, t2, settings, m_physicsSettings.m_minVelocityForRestitution);
            }
        }

        else
        {
            // Store the contact manifold in the cache
            for (int i = 0; i < numContactPoints; ++i)
            {
                // Convert to local space to the body
                const Vec3 p1 = Vec3(invTransformBody1 * (manifold.m_baseOffset + manifold.m_relativeContactPointsOn1[i]));
                const Vec3 p2 = Vec3(invTransformBody2 * (manifold.m_baseOffset + manifold.m_relativeContactPointsOn2[i]));

                // Create the new contact point
                CachedContactPoint& cp = pNewManifold->m_contactPoints[i];
                p1.StoreFloat3(&cp.m_position1);
                p2.StoreFloat3(&cp.m_position2);

                // Reset contact impulses, we haven't applied any.
                cp.m_nonPenetrationLambda = 0.f;
                cp.m_frictionLambda[0] = 0.f;
                cp.m_frictionLambda[1] = 0.f;
            }
        }

        // Store the cached contact point in the body pair cache
        CachedBodyPair* pCBP = static_cast<CachedBodyPair*>(bodyPairHandle);
        pNewManifold->m_nextWithSameBodyPair = pCBP->m_firstCachedManifold;
        pCBP->m_firstCachedManifold = writeCache.ToHandle(pNewManifoldKeyValue);

        // A contact constraint was added
        return contactConstraintCreated;
    }

    bool ContactConstraintManager::AddContactConstraint(ContactAllocator& contactAllocator, BodyPairHandle bodyPairHandle, Body& body1, Body& body2, const ContactManifold& manifold)
    {
        NES_ASSERT(manifold.m_worldSpaceNormal.IsNormalized());

        // Swap bodies so that body 1 id < body 2 id.
        const ContactManifold* pManifold;
        Body* pBody1;
        Body* pBody2;
        ContactManifold temp;
        if (body1.GetID() < body2.GetID())
        {
            pBody1 = &body1;
            pBody2 = &body2;
            temp = manifold.SwapShapes();
            pManifold = &temp;
        }
        else
        {
            pBody1 = &body2;
            pBody2 = &body1;
            pManifold = &manifold;
        }

        // Dispatch to the correct templated form
        // Note: Non-dynamic vs. non-dynamic can happen in this case due to one body being a sensor, so we need to have an extended switch case here
        switch (pBody1->GetMotionType())
        {
            case EBodyMotionType::Dynamic:
            {
                switch (pBody2->GetMotionType())
                {
                    case EBodyMotionType::Dynamic:
                        return TemplatedAddContactConstraint<EBodyMotionType::Dynamic, EBodyMotionType::Dynamic>(contactAllocator, bodyPairHandle, *pBody1, *pBody2, *pManifold);

                    case EBodyMotionType::Kinematic:
                        return TemplatedAddContactConstraint<EBodyMotionType::Dynamic, EBodyMotionType::Kinematic>(contactAllocator, bodyPairHandle, *pBody1, *pBody2, *pManifold);

                    case EBodyMotionType::Static:
                        return TemplatedAddContactConstraint<EBodyMotionType::Dynamic, EBodyMotionType::Static>(contactAllocator, bodyPairHandle, *pBody1, *pBody2, *pManifold);

                    default:
                    {
                        NES_ASSERT(false);
                        break;
                    }
                }
                break;
            }

            case EBodyMotionType::Kinematic:
            {
                switch (pBody2->GetMotionType())
                {
                    case EBodyMotionType::Dynamic:
                        return TemplatedAddContactConstraint<EBodyMotionType::Kinematic, EBodyMotionType::Dynamic>(contactAllocator, bodyPairHandle, *pBody1, *pBody2, *pManifold);

                    case EBodyMotionType::Kinematic:
                        return TemplatedAddContactConstraint<EBodyMotionType::Kinematic, EBodyMotionType::Kinematic>(contactAllocator, bodyPairHandle, *pBody1, *pBody2, *pManifold);

                    case EBodyMotionType::Static:
                        return TemplatedAddContactConstraint<EBodyMotionType::Kinematic, EBodyMotionType::Static>(contactAllocator, bodyPairHandle, *pBody1, *pBody2, *pManifold);

                    default:
                    {
                        NES_ASSERT(false);
                        break;
                    }
                }
                break;
            }

            case EBodyMotionType::Static:
            {
                switch (pBody2->GetMotionType())
                {
                    case EBodyMotionType::Dynamic:
                        return TemplatedAddContactConstraint<EBodyMotionType::Static, EBodyMotionType::Dynamic>(contactAllocator, bodyPairHandle, *pBody1, *pBody2, *pManifold);

                    case EBodyMotionType::Kinematic:
                        return TemplatedAddContactConstraint<EBodyMotionType::Static, EBodyMotionType::Kinematic>(contactAllocator, bodyPairHandle, *pBody1, *pBody2, *pManifold);

                    case EBodyMotionType::Static: // Static vs. Static not possible.
                    default:
                    {
                        NES_ASSERT(false);
                        break;
                    }
                }
                break;
            }

            default:
            {
                NES_ASSERT(false);
                break;
            }
        }

        return false;
    }

    void ContactConstraintManager::OnCCDContactAdded(ContactAllocator& contactAllocator, const Body& body1, const Body& body2, const ContactManifold& manifold, ContactSettings& outSettings)
    {
        NES_ASSERT(manifold.m_worldSpaceNormal.IsNormalized());

        // Calculate the contact settings.
        outSettings.m_combinedFriction = m_combineFriction(body1, manifold.m_subShapeID1, body2, manifold.m_subShapeID2);
        outSettings.m_combinedRestitution = m_combineRestitution(body1, manifold.m_subShapeID1, body2, manifold.m_subShapeID2);
        outSettings.m_isSensor = false; // For now, no sensors are supported during CCD.
        
        // The remainder of this function only deals with calling contact callbacks. If there's no contact callback we also don't do this work.
        if (m_pContactListener != nullptr)
        {
            // Swap bodies so that body 1 id < body 2 id.
            const ContactManifold* pManifold;
            const Body* pBody1;
            const Body* pBody2;
            ContactManifold temp;
            if (body1.GetID() < body2.GetID())
            {
                pBody1 = &body1;
                pBody2 = &body2;
                temp = manifold.SwapShapes();
                pManifold = &temp;
            }
            else
            {
                pBody1 = &body2;
                pBody2 = &body1;
                pManifold = &manifold;
            }

            // Calculate hash
            SubShapeIDPair key { pBody1->GetID(), pManifold->m_subShapeID1, pBody2->GetID(), pManifold->m_subShapeID2};
            const uint64 keyHash = key.GetHash();

            // Check if we already created this contact this physics update
            ManifoldCache& writeCache = m_cache[m_cacheWriteIndex];
            MKeyValueAndCreated newManifoldKeyValue = writeCache.FindOrCreate(contactAllocator, key, keyHash, 0);
            if (newManifoldKeyValue.second)
            {
                // This contact is new for this physics update, check if prevous update if we already had this contact.
                const ManifoldCache& readCache = m_cache[m_cacheWriteIndex ^ 1];
                const MKeyValue* pOldManifoldKeyValue = readCache.Find(key, keyHash);
                if (pOldManifoldKeyValue == nullptr)
                {
                    // New contact
                    m_pContactListener->OnContactAdded(*pBody1, *pBody2, *pManifold, outSettings);
                }
                else
                {
                    // Existing contact
                    m_pContactListener->OnContactPersisted(*pBody1, *pBody2, *pManifold, outSettings);

                    // Mark contact as persisted so that we don't fire OnContactRemoved callbacks;
                    pOldManifoldKeyValue->GetValue().m_flags |= static_cast<uint16>(CachedManifold::EFlags::ContactPersisted);
                }

                // Check if the cache is full
                if (newManifoldKeyValue.first != nullptr)
                {
                    // We don't store any contact points in this manifold as it is not for caching impulses,
                    // we only need to know that the contact was created.
                    CachedManifold& newManifold = newManifoldKeyValue.first->GetValue();
                    newManifold.m_contactNormal = { 0, 0, 0};
                    newManifold.m_flags |= static_cast<uint16>(CachedManifold::EFlags::CCDContact);
                }
            }
            else
            {
                // Already found this contact in the physics update.
                // Note that we can trigger OnContactPersisted multiple times per physics update, but otherwise we have no way of getting the settings.
                m_pContactListener->OnContactPersisted(*pBody1, *pBody2, *pManifold, outSettings);
            }

            // If we swapped body1 and body2, we need to swap the mass scales back
            if (pManifold == &temp)
            {
                std::swap(outSettings.m_inverseMassScale1, outSettings.m_inverseMassScale2);
                std::swap(outSettings.m_inverseInertiaScale1, outSettings.m_inverseInertiaScale2);
                // Note we do not need to negate the relative surface velocity as it is not applied by the CCD collision constraint.
            }
        }

        NES_ASSERT(outSettings.m_isSensor || !(body1.IsSensor() || body2.IsSensor()), "Sensors cannot be converted into regular bodies by a contact callback!");
    }

    void ContactConstraintManager::SortContacts(uint32* constraintIndexBegin, uint32* constraintIndexEnd) const
    {
        QuickSort(constraintIndexBegin, constraintIndexEnd, [this](const uint32 leftIndex, const uint32 rightIndex)
        {
            const ContactConstraint& left = m_constraints[leftIndex];
            const ContactConstraint& right = m_constraints[rightIndex];

            // Most of the time, the sort key will be different, so we sort on that.
            if (left.m_sortKey != right.m_sortKey)
                return left.m_sortKey < right.m_sortKey;

            // If they're equal, we use the IDs of the body 1 to order
            if (left.m_pBody1 != right.m_pBody1)
                return left.m_pBody1->GetID() < right.m_pBody1->GetID();

            // If they're still equal, use the IDs of the body2 to order.
            if (left.m_pBody2 != right.m_pBody2)
                return left.m_pBody2->GetID() < right.m_pBody2->GetID();

            NES_ASSERT(leftIndex == rightIndex, "Hash collision, ordering will be inconsistent");
            return false;
        });
    }

    void ContactConstraintManager::FinalizeContactCacheAndCallContactPointRemovedCallback(const uint expectedNumBodyPairs, const uint expectedNumManifolds)
    {
    #ifdef NES_ASSERTS_ENABLED
        // Mark the cache as finalized
        ManifoldCache& oldWriteCache = m_cache[m_cacheWriteIndex];
        oldWriteCache.Finalize();

        // Check that the count of body pairs and manifolds that we tracked outside the cache (to avoid contention of an atomic) is correct.
        NES_ASSERT(oldWriteCache.GetNumBodyPairs() == expectedNumBodyPairs);
        NES_ASSERT(oldWriteCache.GetNumManifolds() == expectedNumManifolds);
    #endif

        // Buffers are now complete. Swap the buffers.
        m_cacheWriteIndex ^= 1;

        // Get the old read cache / new write cache.
        ManifoldCache& oldReadCache = m_cache[m_cacheWriteIndex];

        // Call the contact point removal callbacks.
        if (m_pContactListener != nullptr)
            oldReadCache.ContactPointRemovedCallbacks(m_pContactListener);

        // We're done with the old read cache for now.
        oldReadCache.Clear();

        // Use the number of contacts from the last iteration to determine the number of buckets to use in the hash map for the next iteration.
        oldReadCache.Prepare(expectedNumBodyPairs, expectedNumManifolds);
    }

    bool ContactConstraintManager::WereBodiesInContact(const BodyID& bodyID1, const BodyID& bodyID2) const
    {
        // The body pair needs to be in the cache, and it needs to have a manifold (otherwise it's just a record indicating that there are no collisions).
        const ManifoldCache& readCache = m_cache[m_cacheWriteIndex ^ 1];
        BodyPair key;
        if (bodyID1 < bodyID2)
            key = BodyPair(bodyID1, bodyID2);
        else
            key = BodyPair(bodyID2, bodyID1);

        const uint64 keyHash = key.GetHash();
        const BPKeyValue* pKeyValue = readCache.Find(key, keyHash);
        return pKeyValue != nullptr && pKeyValue->GetValue().m_firstCachedManifold != ManifoldMap::kInvalidHandle;
    }

    template <EBodyMotionType Type1, EBodyMotionType Type2>
    NES_INLINE void ContactConstraintManager::WarmStartConstraint(ContactConstraint& constraint, MotionProperties* pMotionProps1, MotionProperties* pMotionProps2, const float warmStartImpulseRatio)
    {
        Vec3 t1, t2;
        constraint.GetTangents(t1, t2);

        const Vec3 worldSpaceNormal = constraint.GetWorldSpaceNormal();

        for (WorldContactPoint& wcp : constraint.m_contactPoints)
        {
            // Warm starting: Apply impulse from last frame
            if (wcp.m_frictionConstraint1.IsActive() || wcp.m_frictionConstraint2.IsActive())
            {
                wcp.m_frictionConstraint1.TemplatedWarmStart<Type1, Type2>(pMotionProps1, constraint.m_inverseMass1, pMotionProps2, constraint.m_inverseMass2, t1, warmStartImpulseRatio);
                wcp.m_frictionConstraint2.TemplatedWarmStart<Type1, Type2>(pMotionProps1, constraint.m_inverseMass1, pMotionProps2, constraint.m_inverseMass2, t2, warmStartImpulseRatio);
            }
            wcp.m_nonPenetrationConstraint.TemplatedWarmStart<Type1, Type2>(pMotionProps1, constraint.m_inverseMass1, pMotionProps2, constraint.m_inverseMass2, worldSpaceNormal, warmStartImpulseRatio);
        }
    }

    template <typename MotionPropertiesCallback>
    void ContactConstraintManager::WarmStartVelocityConstraints(const uint32* constraintIndexBegin, const uint32* constraintIndexEnd, const float warmStartImpulseRatio, const MotionPropertiesCallback& motionPropertiesCallback)
    {
        for (const uint32* pConstraintIndex = constraintIndexBegin; pConstraintIndex < constraintIndexEnd; ++pConstraintIndex)
        {
            ContactConstraint& constraint = m_constraints[*pConstraintIndex];

            // Fetch the bodies.
            Body& body1 = *constraint.m_pBody1;
            const EBodyMotionType motionType1 = body1.GetMotionType();
            MotionProperties* pMotionProps1 = body1.GetMotionPropertiesUnchecked();
            
            Body& body2 = *constraint.m_pBody2;
            const EBodyMotionType motionType2 = body2.GetMotionType();
            MotionProperties* pMotionProps2 = body2.GetMotionPropertiesUnchecked();

            // Dispatch to the correct templated form.
            // Note: Warm starting doesn't differentiate between kinematic/static bodies, so we handle both as static bodies.
            if (motionType1 == EBodyMotionType::Dynamic)
            {
                if (motionType2 == EBodyMotionType::Dynamic)
                {
                    WarmStartConstraint<EBodyMotionType::Dynamic, EBodyMotionType::Dynamic>(constraint, pMotionProps1, pMotionProps2, warmStartImpulseRatio);
                    motionPropertiesCallback(pMotionProps2);
                }
                else
                {
                    WarmStartConstraint<EBodyMotionType::Dynamic, EBodyMotionType::Static>(constraint, pMotionProps1, pMotionProps2, warmStartImpulseRatio);
                }

                motionPropertiesCallback(pMotionProps1);
            }

            else
            {
                NES_ASSERT(motionType2 == EBodyMotionType::Dynamic);
                WarmStartConstraint<EBodyMotionType::Static, EBodyMotionType::Dynamic>(constraint, pMotionProps1, pMotionProps2, warmStartImpulseRatio);
                motionPropertiesCallback(pMotionProps2);
            }
        }
    }

    // Specialize for the two body callback types.
    template<> void ContactConstraintManager::WarmStartVelocityConstraints<CalculateSolverSteps>(const uint32* constraintIndexBegin, const uint32* constraintIndexEnd, const float warmStartImpulseRatio, const CalculateSolverSteps& motionPropertiesCallback);
    template<> void ContactConstraintManager::WarmStartVelocityConstraints<DummyCalculateSolverSteps>(const uint32* constraintIndexBegin, const uint32* constraintIndexEnd, const float warmStartImpulseRatio, const DummyCalculateSolverSteps& motionPropertiesCallback);

    template <EBodyMotionType Type1, EBodyMotionType Type2>
    NES_INLINE bool ContactConstraintManager::SolveVelocityConstraint(ContactConstraint& constraint, MotionProperties* pMotionProps1, MotionProperties* pMotionProps2)
    {
        bool anyImpulseApplied = false;

        // Calculate tangents
        Vec3 t1, t2;
        constraint.GetTangents(t1, t2);

        // First, apply all friction constraints (non-penetration is more important than friction).
        for (WorldContactPoint& wcp : constraint.m_contactPoints)
        {
            // Check if friction is enabled
            if (wcp.m_frictionConstraint1.IsActive() || wcp.m_frictionConstraint2.IsActive())
            {
                // Calculate impulse to stop motion in tangential direction.
                float lambda1 = wcp.m_frictionConstraint1.TemplatedSolveVelocityConstraintGetTotalLambda<Type1, Type2>(pMotionProps1, pMotionProps2, t1);
                float lambda2 = wcp.m_frictionConstraint2.TemplatedSolveVelocityConstraintGetTotalLambda<Type1, Type2>(pMotionProps1, pMotionProps2, t2);
                const float totalLambdaSqr = math::Squared(lambda1) + math::Squared(lambda2);

                // Calculate max impulse that can be applied. Note that we're using the non-penetration impulse from the previous iteration here.
                // We do this because non-penetration is more important so is solved last (the last things that are solved in an iterative solver
                // contribute the most).
                const float maxLambdaF = constraint.m_combinedFriction * wcp.m_nonPenetrationConstraint.GetTotalLambda();

                // If the total lambda that we will apply is too large, scale it back.
                if (totalLambdaSqr > math::Squared(maxLambdaF))
                {
                    const float scale = maxLambdaF / std::sqrt(totalLambdaSqr);
                    lambda1 *= scale;
                    lambda2 *= scale;
                }

                // Apply the friction impulse.
                if (wcp.m_frictionConstraint1.TemplatedSolveVelocityConstraintApplyLambda<Type1, Type2>(pMotionProps1, constraint.m_inverseMass1, pMotionProps2, constraint.m_inverseMass2, t1, lambda1))
                    anyImpulseApplied = true;
                if (wcp.m_frictionConstraint2.TemplatedSolveVelocityConstraintApplyLambda<Type1, Type2>(pMotionProps1, constraint.m_inverseMass1, pMotionProps2, constraint.m_inverseMass2, t2, lambda2))
                    anyImpulseApplied = true;
            }
        }

        const Vec3 worldSpaceNormal = constraint.GetWorldSpaceNormal();

        // Then apply all non-penetration constraints
        for (WorldContactPoint& wcp : constraint.m_contactPoints)
        {
            // Solve non-penetration velocities
            if (wcp.m_nonPenetrationConstraint.TemplatedSolveVelocityConstraint<Type1, Type2>(pMotionProps1, constraint.m_inverseMass1, pMotionProps2, constraint.m_inverseMass2, worldSpaceNormal, 0.f, FLT_MAX))
                anyImpulseApplied = true;
        }

        return anyImpulseApplied;
    }

    bool ContactConstraintManager::SolveVelocityConstraints(const uint32* constraintIndexBegin, const uint32* constraintIndexEnd)
    {
        bool anyImpulseApplied = false;

        for (const uint32* pConstraintIndex = constraintIndexBegin; pConstraintIndex < constraintIndexEnd; ++pConstraintIndex)
        {
            ContactConstraint& constraint = m_constraints[*pConstraintIndex];
            
            // Fetch the bodies.
            Body& body1 = *constraint.m_pBody1;
            const EBodyMotionType motionType1 = body1.GetMotionType();
            MotionProperties* pMotionProps1 = body1.GetMotionPropertiesUnchecked();
            
            Body& body2 = *constraint.m_pBody2;
            const EBodyMotionType motionType2 = body2.GetMotionType();
            MotionProperties* pMotionProps2 = body2.GetMotionPropertiesUnchecked();

            // Dispatch to the correct templated form.
            switch (motionType1)
            {
                case EBodyMotionType::Dynamic:
                {
                    switch (motionType2)
                    {
                        case EBodyMotionType::Dynamic:
                        {
                            anyImpulseApplied |= SolveVelocityConstraint<EBodyMotionType::Dynamic, EBodyMotionType::Dynamic>(constraint, pMotionProps1, pMotionProps2);
                            break;
                        }
                            
                        case EBodyMotionType::Kinematic:
                        {
                            anyImpulseApplied |= SolveVelocityConstraint<EBodyMotionType::Dynamic, EBodyMotionType::Kinematic>(constraint, pMotionProps1, pMotionProps2);
                            break;
                        }

                        case EBodyMotionType::Static:
                        {
                            anyImpulseApplied |= SolveVelocityConstraint<EBodyMotionType::Dynamic, EBodyMotionType::Static>(constraint, pMotionProps1, pMotionProps2);
                            break;
                        }

                        default:
                        {
                            NES_ASSERT(false);
                            break;
                        }
                    }
                }

                case EBodyMotionType::Kinematic:
                {
                    NES_ASSERT(motionType2 == EBodyMotionType::Dynamic);
                    anyImpulseApplied |= SolveVelocityConstraint<EBodyMotionType::Kinematic, EBodyMotionType::Dynamic>(constraint, pMotionProps1, pMotionProps2);
                    break;
                }

                case EBodyMotionType::Static:
                {
                    NES_ASSERT(motionType2 == EBodyMotionType::Dynamic);
                    anyImpulseApplied |= SolveVelocityConstraint<EBodyMotionType::Static, EBodyMotionType::Dynamic>(constraint, pMotionProps1, pMotionProps2);
                    break;
                }
                default:
                {
                    NES_ASSERT(false);
                    break;
                }
            }
        }

        return anyImpulseApplied;
    }

    void ContactConstraintManager::StoreAppliedImpulses(const uint32* constraintIndexBegin, const uint32* constraintIndexEnd) const
    {
        // Copy back total applied impulse to the cache for the next frame.
        for (const uint32* pConstraintIndex = constraintIndexBegin; pConstraintIndex < constraintIndexEnd; ++pConstraintIndex)
        {
            const ContactConstraint& constraint = m_constraints[*pConstraintIndex];

            for (const WorldContactPoint& wcp : constraint.m_contactPoints)
            {
                wcp.m_pContactPoint->m_nonPenetrationLambda = wcp.m_nonPenetrationConstraint.GetTotalLambda();
                wcp.m_pContactPoint->m_frictionLambda[0] = wcp.m_frictionConstraint1.GetTotalLambda();
                wcp.m_pContactPoint->m_frictionLambda[1] = wcp.m_frictionConstraint2.GetTotalLambda();
            }
        }
    }

    bool ContactConstraintManager::SolvePositionConstraints(const uint32* constraintIndexBegin, const uint32* constraintIndexEnd)
    {
        bool anyImpulseApplied = false;

        for (const uint32* pConstraintIndex = constraintIndexBegin; pConstraintIndex < constraintIndexEnd; ++pConstraintIndex)
        {
            ContactConstraint& constraint = m_constraints[*pConstraintIndex];

            // Fetch the bodies
            Body& body1 = *constraint.m_pBody1;
            Body& body2 = *constraint.m_pBody2;

            // Get the transforms
            Mat44 transform1 = body1.GetCenterOfMassTransform();
            Mat44 transform2 = body2.GetCenterOfMassTransform();

            const Vec3 worldSpaceNormal = constraint.GetWorldSpaceNormal();

            for (WorldContactPoint& wcp : constraint.m_contactPoints)
            {
                // Calculate new contact point positions in world space (the bodies may have moved).
                RVec3 p1 = transform1 * Vec3::LoadFloat3Unsafe(wcp.m_pContactPoint->m_position1);
                RVec3 p2 = transform2 * Vec3::LoadFloat3Unsafe(wcp.m_pContactPoint->m_position2);

                // Calculate separation along the normal (negative if interpenetrating).
                // Allow a little penetration by default (PhysicsSettings::m_penetrationSlop) to avoid jittering between contact/no-contact that wipes out
                // the contact cache and warm start impulses.
                // Clamp penetration to a max (PhysicsSettings::m_maxPenetrationDistance) so that we don't apply a huge impulse if we're penetrating a lot.
                const float separation = math::Max(Vec3(p2 - p1).Dot(worldSpaceNormal) + m_physicsSettings.m_penetrationSlop, -m_physicsSettings.m_maxPenetrationDistance);

                // Only enforce constraint when separation is < 0 (otherwise we're apart).
                if (separation < 0.f)
                {
                    // Update constraint properties (bodies may have moved).
                    wcp.CalculateNonPenetrationConstraintProperties(body1, constraint.m_inverseMass1, constraint.m_inverseInertiaScale1, body2, constraint.m_inverseMass2, constraint.m_inverseInertiaScale2, p1, p2, worldSpaceNormal);

                    // Solve position errors
                    if (wcp.m_nonPenetrationConstraint.SolvePositionContraintWithMassOverride(body1, constraint.m_inverseMass1, body2, constraint.m_inverseMass2, worldSpaceNormal, separation, m_physicsSettings.m_baumgarte))
                        anyImpulseApplied = true;
                }
            }
        }

        return anyImpulseApplied;
    }

    void ContactConstraintManager::RecycleConstraintBuffer()
    {
        // Reset constraint array.
        m_numConstraints = 0;
    }

    void ContactConstraintManager::FinishConstraintBuffer()
    {
        // Free constraints buffer
        m_pUpdateContext->m_pAllocator->Free(m_constraints, m_maxConstraints * sizeof(ContactConstraint));
        m_constraints = nullptr;
        m_numConstraints = 0;

        // Reset the update context.
        m_pUpdateContext = nullptr;
    }
}
