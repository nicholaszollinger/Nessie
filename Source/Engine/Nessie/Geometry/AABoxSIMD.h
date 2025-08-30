// AABoxSIMD.h
#pragma once
#include "Nessie/Geometry/OrientedBox.h"

namespace nes::math
{
    //----------------------------------------------------------------------------------------------------
    /// @brief : Test a single box against 4 boxes with dimensions split into registers.
    ///	@param box : Box to test against.
    ///	@param box4MinX : The min X values of the 4 boxes.
    ///	@param box4MinY : The min Y values of the 4 boxes. 
    ///	@param box4MinZ : The min Z values of the 4 boxes.
    ///	@param box4MaxX : The max X values of the 4 boxes.
    ///	@param box4MaxY : The max Y values of the 4 boxes.
    ///	@param box4MaxZ : The max Z values of the 4 boxes.
    ///	@returns : An integer register class where each component represents if the box collided or not.
    ///     If the X component is == 1, then the box represented as the first component of each of the
    ///     register inputs collides with the box.
    //----------------------------------------------------------------------------------------------------
    NES_INLINE UVec4Reg AABox4VsAABox(const AABox& box, const Vec4Reg& box4MinX, const Vec4Reg& box4MinY, const Vec4Reg& box4MinZ, const Vec4Reg& box4MaxX, const Vec4Reg& box4MaxY, const Vec4Reg& box4MaxZ)
    {
        // Splat the values of the single box
        // (Replicate the specific component value among all components in the register).
        const Vec4Reg boxMinX = Vec4Reg::Replicate(box.m_min.x);
        const Vec4Reg boxMinY = Vec4Reg::Replicate(box.m_min.y);
        const Vec4Reg boxMinZ = Vec4Reg::Replicate(box.m_min.z);
        const Vec4Reg boxMaxX = Vec4Reg::Replicate(box.m_max.x);
        const Vec4Reg boxMaxY = Vec4Reg::Replicate(box.m_max.y);
        const Vec4Reg boxMaxZ = Vec4Reg::Replicate(box.m_max.z);

        // Test separation over each axis:
        UVec4Reg noOverlapX = UVec4Reg::Or(Vec4Reg::Greater(boxMinX, box4MaxX), Vec4Reg::Greater(box4MinX, boxMaxX));
        UVec4Reg noOverlapY = UVec4Reg::Or(Vec4Reg::Greater(boxMinY, box4MaxY), Vec4Reg::Greater(box4MinY, boxMaxY));
        UVec4Reg noOverlapZ = UVec4Reg::Or(Vec4Reg::Greater(boxMinZ, box4MaxZ), Vec4Reg::Greater(box4MinZ, boxMaxZ));

        // Return Overlap:
        return UVec4Reg::Not(UVec4Reg::Or(UVec4Reg::Or(noOverlapX, noOverlapY), noOverlapZ));
    }

    //----------------------------------------------------------------------------------------------------
    /// @brief : Scale 4 axis aligned boxes. 
    //----------------------------------------------------------------------------------------------------
    NES_INLINE void AABox4Scale(const Vec3 scale, const Vec4Reg& box4MinX, const Vec4Reg& box4MinY, const Vec4Reg& box4MinZ, const Vec4Reg& box4MaxX, const Vec4Reg& box4MaxY, const Vec4Reg& box4MaxZ, Vec4Reg& outBoundsMinX, Vec4Reg& outBoundsMinY, Vec4Reg& outBoundsMinZ, Vec4Reg& outBoundsMaxX, Vec4Reg& outBoundsMaxY, Vec4Reg& outBoundsMaxZ)
    {
        const Vec4Reg scaleX = scale.SplatX();
        const Vec4Reg scaledMinX = scaleX * box4MinX;
        const Vec4Reg scaledMaxX = scaleX * box4MaxX;
        outBoundsMinX = Vec4Reg::Min(scaledMinX, scaledMaxX); // Negative scale can flip the min and max.
        outBoundsMaxX = Vec4Reg::Max(scaledMinX, scaledMaxX);

        const Vec4Reg scaleY = scale.SplatY();
        const Vec4Reg scaledMinY = scaleY * box4MinY;
        const Vec4Reg scaledMaxY = scaleY * box4MaxY;
        outBoundsMinY = Vec4Reg::Min(scaledMinY, scaledMaxY); // Negative scale can flip the min and max.
        outBoundsMaxY = Vec4Reg::Max(scaledMinY, scaledMaxY);

        const Vec4Reg scaleZ = scale.SplatZ();
        const Vec4Reg scaledMinZ = scaleZ * box4MinZ;
        const Vec4Reg scaledMaxZ = scaleZ * box4MaxZ;
        outBoundsMinZ = Vec4Reg::Min(scaledMinZ, scaledMaxZ); // Negative scale can flip the min and max.
        outBoundsMaxZ = Vec4Reg::Max(scaledMinZ, scaledMaxZ);
    }

    //----------------------------------------------------------------------------------------------------
    /// @brief : Enlarge 4 bounding boxes with the given extent (adds to both sides). 
    //----------------------------------------------------------------------------------------------------
    NES_INLINE void AABox4EnlargeWithExtent(const Vec3 extent, Vec4Reg& boundsMinX, Vec4Reg& boundsMinY, Vec4Reg& boundsMinZ, Vec4Reg& boundsMaxX, Vec4Reg& boundsMaxY, Vec4Reg& boundsMaxZ)
    {
        const Vec4Reg extentX = extent.SplatX();
        boundsMinX -= extentX;
        boundsMaxX += extentX;
        
        const Vec4Reg extentY = extent.SplatY();
        boundsMinY -= extentY;
        boundsMaxY += extentY;

        const Vec4Reg extentZ = extent.SplatZ();
        boundsMinZ -= extentZ;
        boundsMaxZ += extentZ;
    }
    
    //----------------------------------------------------------------------------------------------------
    /// @brief : Test 4 AABBs overlap with a point. 
    //----------------------------------------------------------------------------------------------------
    NES_INLINE UVec4Reg AABox4VsPoint(const Vec3 point, const Vec4Reg& box4MinX, const Vec4Reg& box4MinY, const Vec4Reg& box4MinZ, const Vec4Reg& box4MaxX, const Vec4Reg& box4MaxY, const Vec4Reg& box4MaxZ)
    {
        // Splat point
        const Vec4Reg pointX = point.SplatX();
        const Vec4Reg pointY = point.SplatY();
        const Vec4Reg pointZ = point.SplatZ();

        // Test if point overlaps with the boxes.
        const UVec4Reg overlapX = UVec4Reg::And(Vec4Reg::GreaterOrEqual(pointX, box4MinX), Vec4Reg::LessOrEqual(pointX, box4MaxX));
        const UVec4Reg overlapY = UVec4Reg::And(Vec4Reg::GreaterOrEqual(pointY, box4MinY), Vec4Reg::LessOrEqual(pointY, box4MaxY));
        const UVec4Reg overlapZ = UVec4Reg::And(Vec4Reg::GreaterOrEqual(pointZ, box4MinZ), Vec4Reg::LessOrEqual(pointZ, box4MaxZ));

        // Test if all overlapping.
        return UVec4Reg::And(UVec4Reg::And(overlapX, overlapY), overlapZ);
    }

    //----------------------------------------------------------------------------------------------------
    /// @brief : Test if 4 AABBs overlap oriented box. 
    //----------------------------------------------------------------------------------------------------
    NES_INLINE UVec4Reg AABox4VsBox(const Mat44& orientation, const Vec3& halfExtents, const Vec4Reg& box4MinX, const Vec4Reg& box4MinY, const Vec4Reg& box4MinZ, const Vec4Reg& box4MaxX, const Vec4Reg& box4MaxY, const Vec4Reg& box4MaxZ, const float inEpsilon = 1.0e-6f)
    {
        // Taken from: Real Time Collision Detection - Christer Ericson
        // Chapter 4.4.1, page 103-105.
        // Note that the code is swapped around: A is the aabox and B is the oriented box (this saves us from having to invert the orientation of the oriented box)

        // Compute the translation vector t (The translation of B in the space of A).
        Vec4Reg t[3]
        {
            orientation.GetTranslation().SplatX() - 0.5f * (box4MinX + box4MaxX),  
            orientation.GetTranslation().SplatY() - 0.5f * (box4MinY + box4MaxY),  
            orientation.GetTranslation().SplatZ() - 0.5f * (box4MinZ + box4MaxZ)  
        };

        // Compute common subexpressions. Add in an epsilon term to
        // counteract arithmetic errors when two edges are parallel, and
        // their cross-product is (near) null (see text for details)
        Vec3 epsilon = Vec3::Replicate(inEpsilon);
        Vec3 absR[3]
        {
            orientation.GetAxisX().Abs() + epsilon,
            orientation.GetAxisY().Abs() + epsilon,
            orientation.GetAxisZ().Abs() + epsilon
        };

        // Half-extents for A
        Vec4Reg aHalfExtents[3]
        {
            0.5f * (box4MaxX - box4MinX),
            0.5f * (box4MaxY - box4MinY),
            0.5f * (box4MaxZ - box4MinZ)
        };

        // Half-extents for B
        const Vec4Reg bHalfExtentsX = halfExtents.SplatX(); 
        const Vec4Reg bHalfExtentsY = halfExtents.SplatY(); 
        const Vec4Reg bHalfExtentsZ = halfExtents.SplatZ();

        // Each component corresponds to 1 overlapping OBB vs ABB
        UVec4Reg overlaps = UVec4Reg(0xffffffff, 0xffffffff, 0xffffffff, 0xffffffff);

        // Test axes L = A0, L = A1, L = A2
        Vec4Reg ra, rb;
        for (int i = 0; i < 3; i++)
        {
            ra = aHalfExtents[i];
            rb = bHalfExtentsX * absR[0][i] + bHalfExtentsY * absR[1][i] + bHalfExtentsZ * absR[2][i];
            overlaps = UVec4Reg::And(overlaps, Vec4Reg::LessOrEqual(t[i].Abs(), ra + rb));
        }

        // Test axes L = B0, L = B1, L = B2
		for (int i = 0; i < 3; i++)
		{
			ra = aHalfExtents[0] * absR[i][0] + aHalfExtents[1] * absR[i][1] + aHalfExtents[2] * absR[i][2];
			rb = Vec4Reg::Replicate(halfExtents[i]);
			overlaps = UVec4Reg::And(overlaps, Vec4Reg::LessOrEqual((t[0] * orientation[i][0] + t[1] * orientation[i][1] + t[2] * orientation[i][2]).Abs(), ra + rb));
		}

		// Test axis L = A0 x B0
		ra = aHalfExtents[1] * absR[0][2] + aHalfExtents[2] * absR[0][1];
		rb = bHalfExtentsY * absR[2][0] + bHalfExtentsZ * absR[1][0];
		overlaps = UVec4Reg::And(overlaps, Vec4Reg::LessOrEqual((t[2] * orientation[0][1] - t[1] * orientation[0][2]).Abs(), ra + rb));

		// Test axis L = A0 x B1
		ra = aHalfExtents[1] * absR[1][2] + aHalfExtents[2] * absR[1][1];
		rb = bHalfExtentsX * absR[2][0] + bHalfExtentsZ * absR[0][0];
		overlaps = UVec4Reg::And(overlaps, Vec4Reg::LessOrEqual((t[2] * orientation[1][1] - t[1] * orientation[1][2]).Abs(), ra + rb));

		// Test axis L = A0 x B2
		ra = aHalfExtents[1] * absR[2][2] + aHalfExtents[2] * absR[2][1];
		rb = bHalfExtentsX * absR[1][0] + bHalfExtentsY * absR[0][0];
		overlaps = UVec4Reg::And(overlaps, Vec4Reg::LessOrEqual((t[2] * orientation[2][1] - t[1] * orientation[2][2]).Abs(), ra + rb));

		// Test axis L = A1 x B0
		ra = aHalfExtents[0] * absR[0][2] + aHalfExtents[2] * absR[0][0];
		rb = bHalfExtentsY * absR[2][1] + bHalfExtentsZ * absR[1][1];
		overlaps = UVec4Reg::And(overlaps, Vec4Reg::LessOrEqual((t[0] * orientation[0][2] - t[2] * orientation[0][0]).Abs(), ra + rb));

		// Test axis L = A1 x B1
		ra = aHalfExtents[0] * absR[1][2] + aHalfExtents[2] * absR[1][0];
		rb = bHalfExtentsX * absR[2][1] + bHalfExtentsZ * absR[0][1];
		overlaps = UVec4Reg::And(overlaps, Vec4Reg::LessOrEqual((t[0] * orientation[1][2] - t[2] * orientation[1][0]).Abs(), ra + rb));

		// Test axis L = A1 x B2
		ra = aHalfExtents[0] * absR[2][2] + aHalfExtents[2] * absR[2][0];
		rb = bHalfExtentsX * absR[1][1] + bHalfExtentsY * absR[0][1];
		overlaps = UVec4Reg::And(overlaps, Vec4Reg::LessOrEqual((t[0] * orientation[2][2] - t[2] * orientation[2][0]).Abs(), ra + rb));

		// Test axis L = A2 x B0
		ra = aHalfExtents[0] * absR[0][1] + aHalfExtents[1] * absR[0][0];
		rb = bHalfExtentsY * absR[2][2] + bHalfExtentsZ * absR[1][2];
		overlaps = UVec4Reg::And(overlaps, Vec4Reg::LessOrEqual((t[1] * orientation[0][0] - t[0] * orientation[0][1]).Abs(), ra + rb));

		// Test axis L = A2 x B1
		ra = aHalfExtents[0] * absR[1][1] + aHalfExtents[1] * absR[1][0];
		rb = bHalfExtentsX * absR[2][2] + bHalfExtentsZ * absR[0][2];
		overlaps = UVec4Reg::And(overlaps, Vec4Reg::LessOrEqual((t[1] * orientation[1][0] - t[0] * orientation[1][1]).Abs(), ra + rb));

		// Test axis L = A2 x B2
		ra = aHalfExtents[0] * absR[2][1] + aHalfExtents[1] * absR[2][0];
		rb = bHalfExtentsX * absR[1][2] + bHalfExtentsY * absR[0][2];
		overlaps = UVec4Reg::And(overlaps, Vec4Reg::LessOrEqual((t[1] * orientation[2][0] - t[0] * orientation[2][1]).Abs(), ra + rb));

		// Return if the OBB vs. AABBs are intersecting
		return overlaps;
    }

    //----------------------------------------------------------------------------------------------------
    /// @brief : Test if 4 AABBs overlap an oriented box. 
    //----------------------------------------------------------------------------------------------------
    NES_INLINE UVec4Reg AABox4VsBox(const OBB& box, const Vec4Reg& box4MinX, const Vec4Reg& box4MinY, const Vec4Reg& box4MinZ, const Vec4Reg& box4MaxX, const Vec4Reg& box4MaxY, const Vec4Reg& box4MaxZ)
    {
        return AABox4VsBox(box.m_orientation, box.m_halfExtents, box4MinX, box4MinY, box4MinZ, box4MaxX, box4MaxY, box4MaxZ);
    }

    //----------------------------------------------------------------------------------------------------
    /// @brief : Get the squared distance between 4 AABoxes and a point. 
    //----------------------------------------------------------------------------------------------------
    NES_INLINE Vec4Reg AABox4DistanceSqrToPoint(const Vec4Reg& pointX, const Vec4Reg& pointY, const Vec4Reg& pointZ, const Vec4Reg& box4MinX, const Vec4Reg& box4MinY, const Vec4Reg& box4MinZ, const Vec4Reg& box4MaxX, const Vec4Reg& box4MaxY, const Vec4Reg& box4MaxZ)
    {
        // Get the closest point on the box
        Vec4Reg closestPointX = Vec4Reg::Min(Vec4Reg::Max(pointX, box4MinX), box4MaxX);
        Vec4Reg closestPointY = Vec4Reg::Min(Vec4Reg::Max(pointY, box4MinY), box4MaxY);
        Vec4Reg closestPointZ = Vec4Reg::Min(Vec4Reg::Max(pointZ, box4MinZ), box4MaxZ);

        // Return the squared distance between the box and the point.
        return math::Squared(closestPointX - pointX) + math::Squared(closestPointY - pointY) + math::Squared(closestPointZ - pointZ);
    }

    //----------------------------------------------------------------------------------------------------
    /// @brief : Get the squared distance between 4 AABoxes and a point. 
    //----------------------------------------------------------------------------------------------------
    NES_INLINE Vec4Reg AABox4DistanceSqrToPoint(const Vec3 point, const Vec4Reg& box4MinX, const Vec4Reg& box4MinY, const Vec4Reg& box4MinZ, const Vec4Reg& box4MaxX, const Vec4Reg& box4MaxY, const Vec4Reg& box4MaxZ)
    {
        return AABox4DistanceSqrToPoint(point.SplatX(), point.SplatY(), point.SplatZ(), box4MinX, box4MinY, box4MinZ, box4MaxX, box4MaxY, box4MaxZ);
    }

    //----------------------------------------------------------------------------------------------------
    /// @brief : Test 4 AABBs and a sphere. 
    //----------------------------------------------------------------------------------------------------
    NES_INLINE UVec4Reg AABox4VsSphere(const Vec4Reg& centerX, const Vec4Reg& centerY, const Vec4Reg& centerZ, const Vec4Reg& sphereRadiusSqr, const Vec4Reg& box4MinX, const Vec4Reg& box4MinY, const Vec4Reg& box4MinZ, const Vec4Reg& box4MaxX, const Vec4Reg& box4MaxY, const Vec4Reg& box4MaxZ)
    {
        const Vec4Reg distanceSqr = AABox4DistanceSqrToPoint(centerX, centerY, centerZ, box4MinX, box4MinY, box4MinZ, box4MaxX, box4MaxY, box4MaxZ);
        return Vec4Reg::LessOrEqual(distanceSqr, sphereRadiusSqr);
    }

    //----------------------------------------------------------------------------------------------------
    /// @brief : Test 4 AABBs and a sphere. 
    //----------------------------------------------------------------------------------------------------
    NES_INLINE UVec4Reg AABox4VsSphere(const Vec3& center, const float radiusSqr, const Vec4Reg& box4MinX, const Vec4Reg& box4MinY, const Vec4Reg& box4MinZ, const Vec4Reg& box4MaxX, const Vec4Reg& box4MaxY, const Vec4Reg& box4MaxZ)
    {
        return AABox4VsSphere(center.SplatX(), center.SplatY(), center.SplatZ(), Vec4Reg::Replicate(radiusSqr), box4MinX, box4MinY, box4MinZ, box4MaxX, box4MaxY, box4MaxZ);
    }
}