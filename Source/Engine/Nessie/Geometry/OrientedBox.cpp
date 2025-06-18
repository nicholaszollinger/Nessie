// OrientedBox.cpp
#include "OrientedBox.h"

namespace nes
{
    OrientedBox::OrientedBox(const Mat44& orientation, const AABox& box)
        : OrientedBox(orientation.PreTranslated(box.Center()), box.Extent())
    {
        //
    }

	Vec3 OrientedBox::Center() const
    {
	    return m_orientation.GetColumn3(3);
    }

    bool OrientedBox::Overlaps(const AABox& box, const float tolerance) const
    {
        // Taken from: Real Time Collision Detection - Christer Ericson
		// Chapter 4.4.1, page 103-105.
    	// Note that the code is swapped around: A is the AABox and B is the oriented box
    	// (this saves us from having to invert the orientation of the oriented box)
    	
		// Convert AABox to center / extent representation
		const Vec3 aCenter = box.Center();
		Vec3 aHalfExtents = box.Extent();

		// Compute rotation matrix expressing b in a's coordinate frame
		Mat44 rot(m_orientation.GetColumn4(0), m_orientation.GetColumn4(1), m_orientation.GetColumn4(2), m_orientation.GetColumn4(3) - Vec4(aCenter, 0));

		// Compute common subexpressions. Add in an epsilon term to
		// counteract arithmetic errors when two edges are parallel and
		// their cross-product is (near) null (see text for details)
		const Vec3 epsilon = Vec3::Replicate(tolerance);
		Vec3 absR[3]
		{
			rot.GetAxisX().Abs() + epsilon,
			rot.GetAxisY().Abs() + epsilon,
			rot.GetAxisZ().Abs() + epsilon
		};

		// Test axes L = A0, L = A1, L = A2
		float radiusA, radiusB;
		for (int i = 0; i < 3; i++)
		{
			radiusA = aHalfExtents[i];
			radiusB = m_halfExtents[0] * absR[0][i] + m_halfExtents[1] * absR[1][i] + m_halfExtents[2] * absR[2][i];
			if (math::Abs(rot[3][i]) > radiusA + radiusB)
				return false;
		}

		// Test axes L = B0, L = B1, L = B2
		for (int i = 0; i < 3; i++)
		{
			radiusA = aHalfExtents.Dot(absR[i]);
			radiusB = m_halfExtents[i];
			if (math::Abs(rot.GetTranslation().Dot(rot.GetColumn3(i))) > radiusA + radiusB)
				return false;
		}

		// Test axis L = A0 x B0
		radiusA = aHalfExtents[1] * absR[0][2] + aHalfExtents[2] * absR[0][1];
		radiusB = m_halfExtents[1] * absR[2][0] + m_halfExtents[2] * absR[1][0];
		if (math::Abs(rot[3][2] * rot[0][1] - rot[3][1] * rot[0][2]) > radiusA + radiusB)
			return false;

		// Test axis L = A0 x B1
		radiusA = aHalfExtents[1] * absR[1][2] + aHalfExtents[2] * absR[1][1];
		radiusB = m_halfExtents[0] * absR[2][0] + m_halfExtents[2] * absR[0][0];
		if (math::Abs(rot[3][2] * rot[1][1] - rot[3][1] * rot[1][2]) > radiusA + radiusB)
			return false;

		// Test axis L = A0 x B2
		radiusA = aHalfExtents[1] * absR[2][2] + aHalfExtents[2] * absR[2][1];
		radiusB = m_halfExtents[0] * absR[1][0] + m_halfExtents[1] * absR[0][0];
		if (math::Abs(rot[3][2] * rot[2][1] - rot[3][1] * rot[2][2]) > radiusA + radiusB)
			return false;

		// Test axis L = A1 x B0
		radiusA = aHalfExtents[0] * absR[0][2] + aHalfExtents[2] * absR[0][0];
		radiusB = m_halfExtents[1] * absR[2][1] + m_halfExtents[2] * absR[1][1];
		if (math::Abs(rot[3][0] * rot[0][2] - rot[3][2] * rot[0][0]) > radiusA + radiusB)
			return false;

		// Test axis L = A1 x B1
		radiusA = aHalfExtents[0] * absR[1][2] + aHalfExtents[2] * absR[1][0];
		radiusB = m_halfExtents[0] * absR[2][1] + m_halfExtents[2] * absR[0][1];
		if (math::Abs(rot[3][0] * rot[1][2] - rot[3][2] * rot[1][0]) > radiusA + radiusB)
			return false;

		// Test axis L = A1 x B2
		radiusA = aHalfExtents[0] * absR[2][2] + aHalfExtents[2] * absR[2][0];
		radiusB = m_halfExtents[0] * absR[1][1] + m_halfExtents[1] * absR[0][1];
		if (math::Abs(rot[3][0] * rot[2][2] - rot[3][2] * rot[2][0]) > radiusA + radiusB)
			return false;

		// Test axis L = A2 x B0
		radiusA = aHalfExtents[0] * absR[0][1] + aHalfExtents[1] * absR[0][0];
		radiusB = m_halfExtents[1] * absR[2][2] + m_halfExtents[2] * absR[1][2];
		if (math::Abs(rot[3][1] * rot[0][0] - rot[3][0] * rot[0][1]) > radiusA + radiusB)
			return false;

		// Test axis L = A2 x B1
		radiusA = aHalfExtents[0] * absR[1][1] + aHalfExtents[1] * absR[1][0];
		radiusB = m_halfExtents[0] * absR[2][2] + m_halfExtents[2] * absR[0][2];
		if (math::Abs(rot[3][1] * rot[1][0] - rot[3][0] * rot[1][1]) > radiusA + radiusB)
			return false;

		// Test axis L = A2 x B2
		radiusA = aHalfExtents[0] * absR[2][1] + aHalfExtents[1] * absR[2][0];
		radiusB = m_halfExtents[0] * absR[1][2] + m_halfExtents[1] * absR[0][2];
		if (math::Abs(rot[3][1] * rot[2][0] - rot[3][0] * rot[2][1]) > radiusA + radiusB)
			return false;

		// Since no separating axis is found, the OBB and AAB must be intersecting
		return true;
    }

    bool OrientedBox::Overlaps(const OrientedBox& box, const float tolerance) const
    {
    	// Taken from: Real Time Collision Detection - Christer Ericson
		// Chapter 4.4.1, page 103-105.
		// Note that the code is swapped around: A is the AABox and B is the oriented box
		// (this saves us from having to invert the orientation of the oriented box)

		// Compute rotation matrix expressing b in a's coordinate frame
    	Mat44 rot = m_orientation.InversedRotationTranslation() * box.m_orientation;

		// Compute common subexpressions. Add in an epsilon term to
		// counteract arithmetic errors when two edges are parallel, and
		// their cross-product is (near) null (see text for details)
		const Vec3 epsilon = Vec3::Replicate(tolerance);
		Vec3 absR[3]
		{
			rot.GetAxisX().Abs() + epsilon,
			rot.GetAxisY().Abs() + epsilon,
			rot.GetAxisZ().Abs() + epsilon
		};

		// Test axes L = A0, L = A1, L = A2
		float radiusA, radiusB;
		for (int i = 0; i < 3; i++)
		{
			radiusA = m_halfExtents[i];
			radiusB = box.m_halfExtents[0] * absR[0][i] + box.m_halfExtents[1] * absR[1][i] + box.m_halfExtents[2] * absR[2][i];
			if (math::Abs(rot[3][i]) > radiusA + radiusB)
				return false;
		}

		// Test axes L = B0, L = B1, L = B2
		for (int i = 0; i < 3; i++)
		{
			radiusA = m_halfExtents.Dot(absR[i]);
			radiusB = box.m_halfExtents[i];
			if (math::Abs(rot.GetTranslation().Dot(rot.GetColumn3(i))) > radiusA + radiusB)
				return false;
		}

		// Test axis L = A0 x B0
		radiusA = m_halfExtents[1] * absR[0][2] + m_halfExtents[2] * absR[0][1];
		radiusB = box.m_halfExtents[1] * absR[2][0] + box.m_halfExtents[2] * absR[1][0];
		if (math::Abs(rot[3][2] * rot[0][1] - rot[3][1] * rot[0][2]) > radiusA + radiusB)
			return false;

		// Test axis L = A0 x B1
		radiusA = m_halfExtents[1] * absR[1][2] + m_halfExtents[2] * absR[1][1];
		radiusB = box.m_halfExtents[0] * absR[2][0] + box.m_halfExtents[2] * absR[0][0];
		if (math::Abs(rot[3][2] * rot[1][1] - rot[3][1] * rot[1][2]) > radiusA + radiusB)
			return false;

		// Test axis L = A0 x B2
		radiusA = m_halfExtents[1] * absR[2][2] + m_halfExtents[2] * absR[2][1];
		radiusB = box.m_halfExtents[0] * absR[1][0] + box.m_halfExtents[1] * absR[0][0];
		if (math::Abs(rot[3][2] * rot[2][1] - rot[3][1] * rot[2][2]) > radiusA + radiusB)
			return false;

		// Test axis L = A1 x B0
		radiusA = m_halfExtents[0] * absR[0][2] + m_halfExtents[2] * absR[0][0];
		radiusB = box.m_halfExtents[1] * absR[2][1] + box.m_halfExtents[2] * absR[1][1];
		if (math::Abs(rot[3][0] * rot[0][2] - rot[3][2] * rot[0][0]) > radiusA + radiusB)
			return false;

		// Test axis L = A1 x B1
		radiusA = m_halfExtents[0] * absR[1][2] + m_halfExtents[2] * absR[1][0];
		radiusB = box.m_halfExtents[0] * absR[2][1] + box.m_halfExtents[2] * absR[0][1];
		if (math::Abs(rot[3][0] * rot[1][2] - rot[3][2] * rot[1][0]) > radiusA + radiusB)
			return false;

		// Test axis L = A1 x B2
		radiusA = m_halfExtents[0] * absR[2][2] + m_halfExtents[2] * absR[2][0];
		radiusB = box.m_halfExtents[0] * absR[1][1] + box.m_halfExtents[1] * absR[0][1];
		if (math::Abs(rot[3][0] * rot[2][2] - rot[3][2] * rot[2][0]) > radiusA + radiusB)
			return false;

		// Test axis L = A2 x B0
		radiusA = m_halfExtents[0] * absR[0][1] + m_halfExtents[1] * absR[0][0];
		radiusB = box.m_halfExtents[1] * absR[2][2] + box.m_halfExtents[2] * absR[1][2];
		if (math::Abs(rot[3][1] * rot[0][0] - rot[3][0] * rot[0][1]) > radiusA + radiusB)
			return false;

		// Test axis L = A2 x B1
		radiusA = m_halfExtents[0] * absR[1][1] + m_halfExtents[1] * absR[1][0];
		radiusB = box.m_halfExtents[0] * absR[2][2] + box.m_halfExtents[2] * absR[0][2];
		if (math::Abs(rot[3][1] * rot[1][0] - rot[3][0] * rot[1][1]) > radiusA + radiusB)
			return false;

		// Test axis L = A2 x B2
		radiusA = m_halfExtents[0] * absR[2][1] + m_halfExtents[1] * absR[2][0];
		radiusB = box.m_halfExtents[0] * absR[1][2] + box.m_halfExtents[1] * absR[0][2];
		if (math::Abs(rot[3][1] * rot[2][0] - rot[3][0] * rot[2][1]) > radiusA + radiusB)
			return false;

		// Since no separating axis is found, the OBBs must be intersecting
		return true;

    }

    Vec3 OrientedBox::ClosestPointTo(const Vec3& point) const
    {
    	// Start with the center, and make steps to the border from there.
    	Vec3 result = m_orientation.GetTranslation();
    	const Vec3 toPoint = (point - result);
    	
    	// For each Oriented Axis...
    	for (int i = 0; i < 3; ++i)
    	{
    		// ...project the toPoint vector onto that axis to get the
    		// distance along the axis of toPoint from the center.
    		const Vec3 axis = m_orientation.GetColumn3(i);
    		float distance = Vec3::Dot(toPoint, axis);

    		// Clamp the distance to the extents
    		distance = math::Clamp(distance, -m_halfExtents[i], m_halfExtents[i]);

    		// Move the distance on that axis to get the final coordinate.  
    		result += distance * axis; 
    	}

    	return result;
    }

    float OrientedBox::DistanceToPoint(const Vec3& point) const
    {
    	return (point - ClosestPointTo(point)).Length();
    }

    float OrientedBox::DistanceSqrToPoint(const Vec3& point) const
    {
    	return (point - ClosestPointTo(point)).LengthSqr();
    	
    }
}
