// ManifoldBetweenTwoFaces.h
#pragma once
#include "Nessie/Physics/Collision/ContactListener.h"
#include "Shapes/ConvexShape.h"

namespace nes
{
    //----------------------------------------------------------------------------------------------------
    /// @brief : Remove contact points if there are > 4 (no more than 4 are needed for a stable solution)
    ///     On return, contactPointsOn1/2 are reduced to 4 or less points.
    ///	@param penetrationAxis : World space penetration axis. Must be normalized.
    ///	@param contactPointsOn1 : The contact points on shape 1 relative to the centerOfMass.
    ///	@param contactPointsOn2 : The contact points on shape 2 relative to the centerOfMass.
    //----------------------------------------------------------------------------------------------------
    void PruneContactPoints(const Vec3& penetrationAxis, ContactPoints& contactPointsOn1, ContactPoints& contactPointsOn2);
    
    //----------------------------------------------------------------------------------------------------
    /// @brief : Determine contact points between 2 faces of 2 shapes and return them in outContactPoints 1 & 2.
    ///	@param contactPoint1 : The contact point on shape 1 relative to centerOfMass.
    ///	@param contactPoint2 : The contact point on shape 2 relative to centerOfMass.
    ///	@param penetrationAxis : The local space penetration axis in world space.
    ///	@param maxContactDistance : After face 2 is clipped against face 1, each remaining point on face 2
    ///     is tested against the plane of face 1. If the distance on the positive side of the plane is larger
    ///     than this distance, the point will be discarded as a contact point.
    ///	@param shape1Face : The supporting faces on shape 1 relative to centerOfMass.
    ///	@param shape2Face : The supporting faces on shape 1 relative to centerOfMass.
    ///	@param outContactPoints1 : Returns the contact points between the two shapes for shape 1 relative to centerOfMass (any existing points in the output array are left as is).
    ///	@param outContactPoints2 : Returns the contact points between the two shapes for shape 2 relative to centerOfMass (any existing points in the output array are left as is).
    //----------------------------------------------------------------------------------------------------
    void ManifoldBetweenTwoFaces(const Vec3& contactPoint1, const Vec3& contactPoint2, const Vec3& penetrationAxis, float maxContactDistance, const ConvexShape::SupportingFace& shape1Face, const ConvexShape::SupportingFace& shape2Face, ContactPoints& outContactPoints1, ContactPoints& outContactPoints2);
}