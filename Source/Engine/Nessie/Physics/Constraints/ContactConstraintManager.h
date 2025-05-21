// ContactConstraintManager.h
#pragma once
#include "Core/StaticArray.h"
#include "Physics/PhysicsUpdateErrorCodes.h"
#include "Physics/Body/BodyPair.h"
#include "Physics/Collision/ManifoldBetweenTwoFaces.h"
#include "Physics/Collision/Shapes/SubShapeIDPair.h"

// [TODO]:
//#include "Core/LockFreeHashMap.h"
//#include "Physics/Constraints/ConstraintPart/AxisConstraintPart.h"
//#include "Physics/Constraints/ConstraintPart/DualAxisConstraintPart.h"

namespace nes
{
    struct PhysicsSettings;
    class PhysicsUpdateContext;

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
        ContactConstraintManager& operator=(const ContactConstraintManager&) = delete;
        ~ContactConstraintManager();

        //----------------------------------------------------------------------------------------------------
        /// @brief : Intiailize the system. 
        ///	@param maxBodyPairs : Maximum amount of body pairs to process (anything else will fall through the world).
        ///     This number should generally be much higher than the max amount of contact points as there will
        ///     be lots of bodies close that are not actually touching
        ///	@param maxContactConstraints : Maximum amount of contact constraints to process (anything else will fall through the world).
        //----------------------------------------------------------------------------------------------------
        void Init(uint32_t maxBodyPairs, uint32_t maxContactConstraints);
    };
}