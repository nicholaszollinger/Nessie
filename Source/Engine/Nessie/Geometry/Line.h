// Lines.h
#pragma once
#include "Geometry.h"

namespace nes
{
    //----------------------------------------------------------------------------------------------------
    ///	@brief : Defines an infinite Line expressed in 2D coordinates. In 2D, a line can also be thought of
    ///     as a Plane.
    //----------------------------------------------------------------------------------------------------
    struct Line2
    {
        /// Origin or Center point of a line. Really, this is an arbitrary point on an infinite line, but
        /// it can be useful to convert between lines and Rays.
        Vec2 m_origin = Vec2::Zero();

        /// Normalized Direction of the Line, the "Slope". The Line will extend infinitely in this
        /// and the exact opposite direction.
        Vec2 m_direction = Vec2::Right();

        /// Constructors
        Line2() = default;
        Line2(const Vec2 origin, const Vec2 direction) : m_origin(origin), m_direction(direction.Normalized()) {}

        //----------------------------------------------------------------------------------------------------
        ///	@brief : Returns a point on the line that with a "distance" from the origin. 
        //----------------------------------------------------------------------------------------------------
        NES_INLINE Vec2         PointAlongLine(const float distance) const      {  return m_origin + m_direction * distance; }

        //----------------------------------------------------------------------------------------------------
        ///	@brief : Returns the point on the line that is the closest to the query point. 
        //----------------------------------------------------------------------------------------------------
        NES_INLINE Vec2         ClosestPoint(const Vec2 queryPoint) const       { return m_origin + m_direction * ProjectedDistance(queryPoint); }

        //----------------------------------------------------------------------------------------------------
        ///	@brief : Returns the signed distance between the projected point and the origin. This is the
        ///     dot product of the line direction and the vector spanning from the origin to the point.
        //----------------------------------------------------------------------------------------------------
        NES_INLINE float        ProjectedDistance(const Vec2 queryPoint) const  { return Vec2::Dot(queryPoint - m_origin, m_direction); }

        //----------------------------------------------------------------------------------------------------
        ///	@brief : Return the distance from the query point to the closest point on the line.
        //----------------------------------------------------------------------------------------------------
        NES_INLINE float        Distance(const Vec2 queryPoint) const           { return std::sqrt(DistanceSqr(queryPoint)); }

        //----------------------------------------------------------------------------------------------------
        ///	@brief : Return the squared distance from the query point to the closest point on the line.
        //----------------------------------------------------------------------------------------------------
        NES_INLINE float        DistanceSqr(const Vec2 queryPoint) const        { return (ClosestPoint(queryPoint) - m_origin).LengthSqr(); }

        //----------------------------------------------------------------------------------------------------
        /// @brief : Determines which side the query point is located on the line with respected to the direction.
        ///     - If the result is +1, then the point is to the "right" of the line.
        ///     - If the result is -1, then the point is to the "left" of the line.
        ///     - If the result is 0, then the point is on the line.
        /// @param queryPoint : Point that we are checking.
        /// @param tolerance : Optional tolerance given when determining if the point is on the line.
        //----------------------------------------------------------------------------------------------------
        NES_INLINE int          WhichSide(const Vec2 queryPoint, const float tolerance = math::PrecisionDelta<float>()) const;

        //----------------------------------------------------------------------------------------------------
        ///	@brief : Create a perpendicular line to the segment spanning from point "a" to point "b".
        ///     Every point on this line will be equidistant to both points "a" and "b".
        //----------------------------------------------------------------------------------------------------
        static NES_INLINE Line2 PerpendicularBisector(const Vec2 a, const Vec2 b)   { return Line2((a + b) * 0.5f, b - a); }

        //----------------------------------------------------------------------------------------------------
        ///	@brief : Create a Line that intersects both points: a and b.
        //----------------------------------------------------------------------------------------------------
        static NES_INLINE Line2 MakeFromTwoPoints(const Vec2 a, const Vec2 b)       { return Line2(a, (b - a).Normalize()); }
    };

    //----------------------------------------------------------------------------------------------------
    ///	@brief : Defines an infinite line expressed in 3D coordinates.
    //----------------------------------------------------------------------------------------------------
    struct Line3
    {
        /// Origin or Center point of a line. Really, this is an arbitrary point on an infinite line, but
        /// it can be useful to convert between lines and Rays.
        Vec3 m_origin = Vec3::Zero();

        /// Normalized Direction of the Line, the "Slope". The Line will extend infinitely in this
        /// and the exact opposite direction.
        Vec3 m_direction = Vec3::Right();

        NES_INLINE Line3() = default;
        NES_INLINE Line3(const Vec3 origin, const Vec3 direction) : m_origin(origin), m_direction(direction.Normalized()) {}

        //----------------------------------------------------------------------------------------------------
        ///	@brief : Returns a point on the line that with a "distance" from the origin. 
        //----------------------------------------------------------------------------------------------------
        NES_INLINE Vec3         PointAlongLine(const float distance) const      { return m_origin + m_direction * distance; }

        //----------------------------------------------------------------------------------------------------
        ///	@brief : Returns the point on the line that is the closest to the query point. 
        //----------------------------------------------------------------------------------------------------
        NES_INLINE Vec3         ClosestPoint(const Vec3 queryPoint) const       { return m_origin + m_direction * ProjectedDistance(queryPoint); }

        //----------------------------------------------------------------------------------------------------
        ///	@brief : Returns the signed distance between the projected point and the origin. This is the
        ///     dot product of the line direction and the vector spanning from the origin to the point.
        //----------------------------------------------------------------------------------------------------
        NES_INLINE float        ProjectedDistance(const Vec3 point) const       { return Vec3::Dot(point - m_origin, m_direction); }

        //----------------------------------------------------------------------------------------------------
        ///	@brief : Return the distance from the query point to the closest point on the line.
        //----------------------------------------------------------------------------------------------------
        NES_INLINE float        Distance(const Vec3 queryPoint) const           { return std::sqrt(DistanceSqr(queryPoint)); }

        //----------------------------------------------------------------------------------------------------
        ///	@brief : Return the squared distance from the query point to the closest point on the line.
        //----------------------------------------------------------------------------------------------------
        NES_INLINE float        DistanceSqr(const Vec3 queryPoint) const        { return (ClosestPoint(queryPoint) - m_origin).LengthSqr(); }

        //----------------------------------------------------------------------------------------------------
        ///	@brief : Create a Line that intersects both points: a and b.
        //----------------------------------------------------------------------------------------------------
        static NES_INLINE Line3 MakeFromTwoPoints(const Vec3 a, const Vec3 b)   { return Line3(a, (b - a).Normalize()); }
    };
}

namespace nes
{
    int Line2::WhichSide(const Vec2 queryPoint, const float tolerance) const
    {
        // Calculate the 2x2 determinant.
        // The top row is the vector that goes to the point.
        // The bottom row is direction of the line.
        const Vec2 toPoint = queryPoint - m_origin;
        const float determinant = toPoint.x * m_direction.y - toPoint.y * m_direction.x;

        if (determinant > tolerance)
            return +1;

        return determinant < -tolerance ? -1 : 0;
    }
}
