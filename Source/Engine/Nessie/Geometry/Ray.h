// Ray.h
#pragma once
#include "Math/MathConfig.h"
#include "Math/Vec2.h"
#include "Math/Vec3.h"

namespace nes
{
    //----------------------------------------------------------------------------------------------------
    ///	@brief : Ray expressed in 2D coordinates. A Ray contains an Origin position and normalized Direction.
    //----------------------------------------------------------------------------------------------------
    struct Ray2
    {
        Vec2                m_origin    = Vec2(0, 0);
        Vec2                m_direction = Vec2(1, 0);

        Ray2() = default;
        Ray2(const Vec2 origin, const Vec2 direction)   : m_origin(origin), m_direction(direction) { m_direction.Normalize(); }

        NES_INLINE bool     operator==(const Ray2& other) const     { return m_origin == other.m_origin && m_direction == other.m_direction; }
        NES_INLINE bool     operator!=(const Ray2& other) const     { return !(*this == other); }

        //----------------------------------------------------------------------------------------------------
        ///	@brief : Calculates the position starting at the origin and moving a distance in the Ray's
        ///     direction. If the distance is negative, this will return the origin.
        //----------------------------------------------------------------------------------------------------
        NES_INLINE Vec2     PositionAlongRay(const float distance) const;

        //----------------------------------------------------------------------------------------------------
        ///	@brief : Get the closest point along the Ray to the query point.  
        //----------------------------------------------------------------------------------------------------
        NES_INLINE Vec2     ClosestPoint(const Vec2 point) const;

        //----------------------------------------------------------------------------------------------------
        ///	@brief : Get the distance to the closest point along the Ray. 
        //----------------------------------------------------------------------------------------------------
        NES_INLINE float    DistanceToPoint(const Vec2 point) const         { return std::sqrt(DistanceSqrToPoint(point));}

        //----------------------------------------------------------------------------------------------------
        ///	@brief : Get the squared distance to the closest point along the Ray. 
        //----------------------------------------------------------------------------------------------------
        NES_INLINE float    DistanceSqrToPoint(const Vec2 point) const;

        //----------------------------------------------------------------------------------------------------
        /// @brief : Returns this ray translated by the vector.
        //----------------------------------------------------------------------------------------------------
        NES_INLINE Ray2     Translated(const Vec2 translation) const        { return Ray2(m_origin + translation, m_direction); }
    };

    //----------------------------------------------------------------------------------------------------
    ///		@brief : Ray expressed in 3D coordinates. A Ray contains an Origin position and normalized Direction.
    //----------------------------------------------------------------------------------------------------
    struct Ray
    {
        Vec3                m_origin    = Vec3(0, 0, 0);
        Vec3                m_direction = Vec3(1, 0, 0);

        Ray() = default;
        Ray(const Vec3 origin, const Vec3 direction)    : m_origin(origin), m_direction(direction) { m_direction.Normalize(); }

        NES_INLINE bool     operator==(const Ray& other) const      { return m_origin == other.m_origin && m_direction == other.m_direction; }
        NES_INLINE bool     operator!=(const Ray& other) const      { return !(*this == other); }

        //----------------------------------------------------------------------------------------------------
        ///	@brief : Calculates the position starting at the origin and moving a distance in the Ray's
        ///     direction. If the distance is negative, this will return the origin.
        //----------------------------------------------------------------------------------------------------
        NES_INLINE Vec3     PositionAlongRay(const float distance);

        //----------------------------------------------------------------------------------------------------
        ///	@brief : Get the closest point along the Ray to the query point.  
        //----------------------------------------------------------------------------------------------------
        NES_INLINE Vec3     ClosestPoint(const Vec3 point) const;

        //----------------------------------------------------------------------------------------------------
        ///	@brief : Get the distance to the closest point along the Ray. 
        //----------------------------------------------------------------------------------------------------
        NES_INLINE float    DistanceToPoint(const Vec3 point) const         { return std::sqrt(DistanceSqrToPoint(point));}

        //----------------------------------------------------------------------------------------------------
        ///	@brief : Get the squared distance to the closest point along the Ray. 
        //----------------------------------------------------------------------------------------------------
        NES_INLINE float    DistanceSqrToPoint(const Vec3 point) const;

        //----------------------------------------------------------------------------------------------------
        /// @brief : Returns this ray transformed by the matrix.
        //----------------------------------------------------------------------------------------------------
        NES_INLINE Ray      Transformed(const Mat44& transform) const;

        //----------------------------------------------------------------------------------------------------
        /// @brief : Returns this ray translated by the vector.
        //----------------------------------------------------------------------------------------------------
        NES_INLINE Ray      Translated(const Vec3 translation) const        { return Ray(m_origin + translation, m_direction); }
    };
}

#include "Ray.inl"