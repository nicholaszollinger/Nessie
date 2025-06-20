// ContactConstraintManager.h
#pragma once

#include "Core/StaticArray.h"
#include "Physics/PhysicsUpdateErrorCodes.h"
#include "Physics/Body/BodyPair.h"
#include "Physics/Collision/ManifoldBetweenTwoFaces.h"
#include "Physics/Collision/Shapes/SubShapeIDPair.h"
#include "Physics/Constraints/ConstraintPart/AxisConstraintPart.h"

// [TODO]:
//#include "Core/LockFreeHashMap.h"
//#include "Physics/Constraints/ConstraintPart/DualAxisConstraintPart.h"

namespace nes
{
    struct PhysicsSettings;
    struct PhysicsUpdateContext;

    //----------------------------------------------------------------------------------------------------
    /// @brief : Class that manages constraints between two bodies in contact with each other. 
    //----------------------------------------------------------------------------------------------------
    class ContactConstraintManager
    {
    public:
        /// Max 4 contact points are needed for a stable manifold.
        static constexpr int kMaxContactPoints = 4;

        /// Callback function to combine the restitution or friction of two bodies
        /// Note that when merging manifolds (when PhysicsSettings::m_useManifoldReduction is true) you will only get a callback for the merged manifold.
        /// It is not possible in that case to get all sub shape ID pairs that were colliding, you'll get the first encountered pair.
        using CombineFunction = float (*)(const Body& body1, const SubShapeID& subShapeID1, const Body& body2, const SubShapeID& subShapeID2);
        
    public:
        explicit ContactConstraintManager(const PhysicsSettings& settings);
        ContactConstraintManager(const ContactConstraintManager&) = delete;
        ContactConstraintManager(ContactConstraintManager&&) noexcept = delete;
        ContactConstraintManager& operator=(const ContactConstraintManager&) = delete;
        ContactConstraintManager& operator=(ContactConstraintManager&&) noexcept = delete;
        ~ContactConstraintManager();

        //----------------------------------------------------------------------------------------------------
        /// @brief : Initialize the system. 
        ///	@param maxBodyPairs : Maximum amount of body pairs to process (anything else will fall through the world).
        ///     This number should generally be much higher than the max amount of contact points as there will
        ///     be lots of bodies close that are not actually touching
        ///	@param maxContactConstraints : Maximum amount of contact constraints to process (anything else will fall through the world).
        //----------------------------------------------------------------------------------------------------
        void                        Init(uint32_t maxBodyPairs, uint32_t maxContactConstraints);

        //----------------------------------------------------------------------------------------------------
        /// @brief : Get the two affected bodies for a given constraint.
        //----------------------------------------------------------------------------------------------------
        inline void                 GetAffectedBodies(const uint32 constraintIndex, const Body*& outBody1, const Body*& outBody2) const
        {
            const ContactConstraint& constraint = m_constraints[constraintIndex];
            outBody1 = constraint.m_pBody1;
            outBody2 = constraint.m_pBody2;
        }

    private:
        // [TODO]: 
        //----------------------------------------------------------------------------------------------------
        /// @brief : Contact constraints are used for solving penetrations between bodies. 
        //----------------------------------------------------------------------------------------------------
        struct ContactConstraint
        {
            Body*                   m_pBody1;
            Body*                   m_pBody2;
            uint64                  m_sortKey;
            Float3                  m_worldSpaceNormal;
            float                   m_combinedFriction;
            float                   m_inverseMass1;
            float                   m_inverseInertiaScale1;
            float                   m_inverseMass2;
            float                   m_inverseInertiaScale2;
            // WorldContactPoints
            
            //----------------------------------------------------------------------------------------------------
            /// @brief : Convert world space normal to a Vec3 
            //----------------------------------------------------------------------------------------------------
            NES_INLINE Vec3         GetWorldSpaceNormal() const { return Vec3::LoadFloat3Unsafe(m_worldSpaceNormal); }
        };


    private:
        /// The main physics settings instance.
        const PhysicsSettings&      m_physicsSettings;

        /// The constraints that were added this frame.
        ContactConstraint*          m_constraints = nullptr;
        uint32                      m_maxConstraints = 0;
        std::atomic<uint32>         m_numConstraints { 0 };

        /// Context used for this physics update
        PhysicsUpdateContext*       m_pUpdateContext = nullptr;
    };
}
