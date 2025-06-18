// Circle.h
#pragma once
#include "Math/Vec2.h"

namespace nes
{
    //---------------------------------------------------------------------------------------------------
    /// @brief : A 2D circle represented by a center and radius. 
    //----------------------------------------------------------------------------------------------------
    struct Circle
    {
        Vec2    m_center;
        float   m_radius;

        /// The default constructor sets the center at the origin and the radius to 1.
        Circle()                                        : m_center(Vec2::Zero()), m_radius(1.0f) {}
        Circle(const Vec2 center, const float radius)   : m_center(center), m_radius(radius) {}

        //----------------------------------------------------------------------------------------------------
        /// @brief : Get the diameter of the circle (2 * radius). 
        //----------------------------------------------------------------------------------------------------
        NES_INLINE float    Diameter() const            { return m_radius * 2.f; }

        //----------------------------------------------------------------------------------------------------
        /// @brief : Get the circumference (perimeter length of the circle). 
        //----------------------------------------------------------------------------------------------------
        NES_INLINE float    Circumference() const       { return 2.f * math::Pi<float>() * m_radius; }

        //----------------------------------------------------------------------------------------------------
        /// @brief : Grow the circle (if necessary) to contain the point. 
        //----------------------------------------------------------------------------------------------------
        NES_INLINE void     Encapsulate(const Vec2 point);

        //----------------------------------------------------------------------------------------------------
        /// @brief : Check if this circle intersects with the other. 
        //----------------------------------------------------------------------------------------------------
        NES_INLINE bool     Overlaps(const Circle& other) const;

        //----------------------------------------------------------------------------------------------------
        /// @brief : Returns true if the point is inside the circle.
        //----------------------------------------------------------------------------------------------------
        NES_INLINE bool     Contains(const Vec2 point) const;
    };

    namespace math
    {
        //----------------------------------------------------------------------------------------------------
        ///	@brief : Creates an approximate circle to encompass the points in the array by defining an AABB2
        ///     to encompass the points. This should be a first pass when devising a full bounding circle.
        //----------------------------------------------------------------------------------------------------
        inline void         ApproximateCircleFromDistancePoints(Circle& circle, const Float2* points, const size_t count);

        //----------------------------------------------------------------------------------------------------
        ///	@brief : Creates an approximate circle to encompass the points in the array by defining an AABB2
        ///     to encompass the points. This should be a first pass when devising a full bounding circle.
        //----------------------------------------------------------------------------------------------------
        inline void         ApproximateCircleFromDistancePoints(Circle& circle, const Vec2* points, const size_t count);

        //----------------------------------------------------------------------------------------------------
        ///	@brief : Compute a bounding circle that encompasses all points in the array. This is done
        ///     in two passes: first get an approximation that encompasses the two most distant points,
        ///     then grow the circle to encompass all points.
        //----------------------------------------------------------------------------------------------------
        inline void         RitterBoundingCircle(Circle& circle, const Float2* points, const size_t count);

        //----------------------------------------------------------------------------------------------------
        ///	@brief : Compute a bounding circle that encompasses all points in the array. This is done
        ///     in two passes: first get an approximation that encompasses the two most distant points,
        ///     then grow the circle to encompass all points.
        //----------------------------------------------------------------------------------------------------
        inline void         RitterBoundingCircle(Circle& circle, const Vec2* points, const size_t count);
    }
}

#include "Circle.inl"
