// OrientedBox2.h
#pragma once
#include "AABox2.h"
#include "Math/Mat33.h"

namespace nes
{
    //----------------------------------------------------------------------------------------------------
    // [TODO]: Overlaps AABox2.
    ///	@brief : An Oriented Bounding Box (OBB) is a 2D rect, but with an arbitrary orientation.
    //----------------------------------------------------------------------------------------------------
    class OrientedBox2
    {
    public:
        // [TODO]: Mat33 class to replace the orientation
        Mat33   m_orientation;    /// Transform that positions and rotates the local space, axis-aligned box into world space
        Vec2    m_halfExtents;     /// Half-extents (half the size of the edge) of the local space axis-aligned box.

        OrientedBox2() = default;
        OrientedBox2(const Mat33& orientation, const Vec2 halfExtents) : m_orientation(orientation), m_halfExtents(halfExtents) {}
        OrientedBox2(const Mat33& orientation, const AABox2& box);

        //----------------------------------------------------------------------------------------------------
        /// @brief : Get the center of the oriented box. 
        //----------------------------------------------------------------------------------------------------
        Vec2    Center() const;

        //----------------------------------------------------------------------------------------------------
        /// @brief : Test if two oriented bounding boxes intersect each other.
        //----------------------------------------------------------------------------------------------------
        bool    Overlaps(const OrientedBox2& box, const float tolerance = 1.0e-6f) const;

        //----------------------------------------------------------------------------------------------------
        ///	@brief : Returns the closest point on or in the oriented box to the query point. 
        //----------------------------------------------------------------------------------------------------
        Vec2    ClosestPoint(const Vec2 point) const;

        //----------------------------------------------------------------------------------------------------
        ///	@brief : Returns the distance from the query point to the closest point on the oriented box. 
        //----------------------------------------------------------------------------------------------------
        float   DistanceToPoint(const Vec2 point) const            { return std::sqrt(DistanceSqrToPoint(point)); }

        //----------------------------------------------------------------------------------------------------
        /// @brief : Returns the squared distance from the query point to the closest point on the oriented box.  
        //----------------------------------------------------------------------------------------------------
        float   DistanceSqrToPoint(const Vec2 point) const         { return (ClosestPoint(point) - point).LengthSqr(); }
    };
}