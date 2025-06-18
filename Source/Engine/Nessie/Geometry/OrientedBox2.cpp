// OrientedBox2.cpp
#include "OrientedBox2.h"

namespace nes
{
    OrientedBox2::OrientedBox2(const Mat33& orientation, const AABox2& box)
        : m_halfExtents(box.Extent())
    {
        // [TODO]: Fix when you implement the other Mat3 class.
        m_orientation = orientation.PreTranslated(box.Center());
    }

    Vec2 OrientedBox2::Center() const
    {
        // [TODO]: Fix when you implement the other Mat3 class.
        return Vec2(m_orientation[2][0], m_orientation[2][1]);
    }

    bool OrientedBox2::Overlaps(const OrientedBox2& box, const float tolerance) const
    {
        // Compute the Rotation Matrix expressing "other" in this Box's coordinate frame.
        Mat33 orientation{};
        Mat33 orientationAbs{};
        
        for (int i = 0; i < 3; ++i)
        {
            for (int j = 0; j < 3; ++j)
            {
                orientation[i][j] = Vec3::Dot(m_orientation.GetRow3(i), box.m_orientation.GetColumn3(j));
        
                // Compute common subexpressions. Add in an epsilon term to counteract arithmetic errors when two edges are
                // parallel, and their cross-product is (near) null.
                orientationAbs[i][j] = math::Abs(orientation[i][j]) + tolerance;  
            }
        }
        
        // Compute the translation vector
        Vec2 translation = box.Center() - Center();
        
        // Bring the translation into this coordinate frame:
        translation = Vec2
        (
            Vec2::Dot(translation, m_orientation.GetRow2(0)),
            Vec2::Dot(translation, m_orientation.GetRow2(1))
        );
        
        float radiusA;
        float radiusB;
        
        // Test to find a separating Axis "L". "R" == m_localOrientation matrix, "R[0]" == X Axis of the local orientation.
        // Test L = R[0]
        // Test L = R[1]
        for (int i = 0; i < 2; ++i)
        {
            radiusA = m_halfExtents[i];
            radiusB = (box.m_halfExtents[0] * orientationAbs[i][0])
                    + (box.m_halfExtents[1] * orientationAbs[i][1]);
        
            if (math::Abs(translation[i]) > radiusA + radiusB)
                return false;
        }
        
        // Test L = other.R[0]
        // Test L = other.R[1]
        for (int i = 0; i < 2; ++i)
        {
            radiusA = (m_halfExtents[0] * orientationAbs[i][0])
                    + (m_halfExtents[1] * orientationAbs[i][1]);
            radiusB = box.m_halfExtents[i];
        
            if (math::Abs((translation[0] * orientation[0][i]) + (translation[1] * orientation[1][i])) > radiusA + radiusB)
                return false;
        }

        // [TODO]: Is this everything?
        // There is no separating axis, so they must be intersecting.
        return true;
    }

    Vec2 OrientedBox2::ClosestPoint(const Vec2 point) const
    {
        const Vec2 center = Center();
        const Vec2 toPoint = (point - center);

        // Start with the center, and make steps to the border from there.
        Vec2 result = center;

        // For each Oriented Axis...
        for (int i = 0; i < 2; ++i)
        {
            // ...project the toPoint vector onto that axis to get the
            // distance along the axis of toPoint from the center.
            const Vec2 axis = Vec2(m_orientation[i][0], m_orientation[i][1]);
            float distance = Vec2::Dot(toPoint, axis); 

            // Clamp the distance to the extents
            distance = math::Clamp(distance, -m_halfExtents[i], m_halfExtents[i]);

            // Move the distance on that axis to get the final coordinate.  
            result += distance * axis; 
        }

        return result;
    }
}
