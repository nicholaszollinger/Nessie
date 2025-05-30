// CollideShape.h
#pragma once
#include "BackFaceMode.h"
#include "ActiveEdgeMode.h"
#include "CollectFacesMode.h"
#include "Core/StaticArray.h"
#include "Math/Vector3.h"
#include "Physics/PhysicsSettings.h"
#include "Physics/Body/BodyID.h"
#include "Shapes/SubShapeID.h"

namespace nes
{
    class SubShapeID;
}

namespace nes
{
    //----------------------------------------------------------------------------------------------------
    /// @brief : Class that contains all information of two colliding shapes. 
    //----------------------------------------------------------------------------------------------------
    struct CollideShapeResult
    {
        using Face = StaticArray<Vector3, 32>;
        
        Vector3     m_contactPointOn1;
        Vector3     m_contactPointOn2;
        Vector3     m_penetrationAxis;
        float       m_penetrationDepth{};
        SubShapeID  m_subShapeID1;
        SubShapeID  m_subShapeID2;
        BodyID      m_bodyID2;
        Face        m_shape1Face{};
        Face        m_shape2Face{};
        
        CollideShapeResult() = default;
        
        CollideShapeResult(const Vector3& contactPoint1, const Vector3& contactPoint2, const Vector3& penetrationAxis, const float penetrationDepth, const SubShapeID& subShapeID1, const SubShapeID& subShapeID2, const BodyID& bodyID2)
            : m_contactPointOn1(contactPoint1)
            , m_contactPointOn2(contactPoint2)
            , m_penetrationAxis(penetrationAxis)
            , m_penetrationDepth(penetrationDepth)
            , m_subShapeID1(subShapeID1)
            , m_subShapeID2(subShapeID2)
            , m_bodyID2(bodyID2)
        {
            //
        }

        //----------------------------------------------------------------------------------------------------
        /// @brief : Function required by Collision Collector. A smaller fraction is considered to be a 'better hit'.
        ///     We use -penetration depth to get the hit with the biggest penetration depth
        //----------------------------------------------------------------------------------------------------
        inline float GetEarlyOutFraction() const { return -m_penetrationDepth; }

        //----------------------------------------------------------------------------------------------------
        /// @brief : Reverses the hit result, swapping the contact point 1 with contact point 2, etc. 
        //----------------------------------------------------------------------------------------------------
        inline CollideShapeResult Reversed() const
        {
            CollideShapeResult result;
            result.m_contactPointOn1 = m_contactPointOn2;
            result.m_contactPointOn2 = m_contactPointOn1;
            result.m_penetrationAxis = -m_penetrationAxis;
            result.m_penetrationDepth = m_penetrationDepth;
            result.m_subShapeID1 = m_subShapeID2;
            result.m_subShapeID2 = m_subShapeID1;
            result.m_bodyID2 = m_bodyID2;
            result.m_shape1Face = m_shape2Face;
            result.m_shape2Face = m_shape1Face;
            return result;
        }
    };

    //----------------------------------------------------------------------------------------------------
    /// @brief : Base Settings to be passed with a collision query. 
    //----------------------------------------------------------------------------------------------------
    struct CollideShapeSettingsBase
    {
        /// How active edges (edges that a moving object should bump into) are handled
        EActiveEdgeMode				m_activeEdgeMode				= EActiveEdgeMode::CollideOnlyWithActive;

        /// If colliding faces should be collected or only the collision point
        ECollectFacesMode			m_collectFacesMode			    = ECollectFacesMode::NoFaces;

        /// If objects are closer than this distance, they are considered to be colliding. Used for GJK. (unit: m)
        float                       m_collisionTolerance			= physics::kDefaultCollisionTolerance;

        /// A factor that determines the accuracy of the penetration depth calculation. IF the change of the squared
        /// distance is less than tolerance * m_currentPenetrationDepth^2 the algorithm will terminate. (unit: dimensionless)
        float                       m_penetrationTolerance          = physics::kDefaultPenetrationTolerance;

        /// When m_activeEdgeMode is CollideOnlyWithActive a movement direction can be provided. When hitting an inactive edge,
        /// the system will select the triangle normal as penetration depth only if it impedes the movement less than with the
        /// calculated penetration depth.
        Vector3                     m_activeEdgeMovementDirection   = Vector3::Zero();
    };

    //----------------------------------------------------------------------------------------------------
    /// @brief : Settings to be passed with a collision query. 
    //----------------------------------------------------------------------------------------------------
    struct CollideShapeSettings : public CollideShapeSettingsBase
    {
        /// When > 0, contacts in the vicinity of the query shape can be found. All nearest contacts that are
        /// not further away than this distance will be found. Note that in this case CollideShapeResult::m_penetrationDepth
        /// can become negative to indicate that objects are not overlapping. (unit: meter)
        float                       m_maxSeparationDistance = 0.f;

        /// How backfacing triangles should be treated
        EBackFaceMode                m_backFaceMode = EBackFaceMode::IgnoreBackFaces;
    };
}
