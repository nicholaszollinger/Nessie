// Ray.h
#pragma once
#include "Vector3.h"
#include "MathConfig.h"

namespace nes
{
    //----------------------------------------------------------------------------------------------------
    ///		@brief : Ray expressed in 2D coordinates. A Ray contains an Origin position and normalized Direction.
    //----------------------------------------------------------------------------------------------------
    template <FloatingPointType Type>
    struct TRay2
    {
        TVector2<Type> m_origin = TVector2<Type>(0, 0);
        TVector2<Type> m_direction = TVector2<Type>(1, 0);

        constexpr TRay2() = default;
        constexpr TRay2(const TVector2<Type>& origin, const TVector2<Type>& direction);

        constexpr bool operator==(const TRay2<Type>& other) const;
        constexpr bool operator!=(const TRay2<Type>& other) const { return !(*this == other); }

        constexpr TVector2<Type> PositionAlongRay(const Type distance);
        Type DistanceToPoint(const TVector2<Type>& point) const;
        constexpr Type SquaredDistanceToPoint(const TVector2<Type>& point) const;
        constexpr TVector2<Type> ClosestPointToPoint(const TVector2<Type>& point) const;

        std::string ToString() const;
    };

    //----------------------------------------------------------------------------------------------------
    ///		@brief : Ray expressed in 3D coordinates. A Ray contains an Origin position and normalized Direction.
    //----------------------------------------------------------------------------------------------------
    template <FloatingPointType Type>
    struct TRay3
    {
        TVector3<Type> m_origin = TVector3<Type>(0, 0, 0);
        TVector3<Type> m_direction = TVector3<Type>(1, 0, 0);

        constexpr TRay3() = default;
        constexpr TRay3(const TVector3<Type>& origin, const TVector3<Type>& direction);

        constexpr bool operator==(const TRay3& other) const;
        constexpr bool operator!=(const TRay3& other) const { return !(*this == other); }

        constexpr TVector3<Type> PositionAlongRay(const Type distance);
        Type DistanceToPoint(const TVector3<Type>& point) const;
        constexpr Type SquaredDistanceToPoint(const TVector3<Type>& point) const;
        constexpr TVector3<Type> ClosestPointToPoint(const TVector3<Type>& point) const;

        std::string ToString() const;
    };

    using Ray2f = TRay2<float>;
    using Ray2d = TRay2<double>;
    using Ray2D = TRay2<NES_PRECISION_TYPE>;

    using Ray3f = TRay3<float>;
    using Ray3d = TRay3<double>;
    using Ray = TRay3<NES_PRECISION_TYPE>;
}

namespace nes
{
    //----------------------------------------------------------------------------------------------------
    ///		@brief : Construct a 2D Ray with an Origin position and a direction.
    //----------------------------------------------------------------------------------------------------
    template <FloatingPointType Type>
    constexpr TRay2<Type>::TRay2(const TVector2<Type>& origin, const TVector2<Type>& direction)
        : m_origin(origin)
        , m_direction(direction)
    {
        m_direction.Normalize();
    }

    template <FloatingPointType Type>
    constexpr bool TRay2<Type>::operator==(const TRay2<Type>& other) const
    {
        return m_origin == other.m_origin && m_direction == other.m_direction;
    }

    //----------------------------------------------------------------------------------------------------
    ///		@brief : Calculates the position starting at the origin and moving a distance in the Ray's
    ///             direction. If the distance is negative, this will return the origin.
    ///		@param distance : Distance along the ray.
    //----------------------------------------------------------------------------------------------------
    template <FloatingPointType Type>
    constexpr TVector2<Type> TRay2<Type>::PositionAlongRay(const Type distance)
    {
        if (distance <= 0)
            return m_origin;
        
        return m_origin + (m_direction * distance);
    }

    //----------------------------------------------------------------------------------------------------
    ///		@brief : Get the distance to the closest point along the Ray. 
    //----------------------------------------------------------------------------------------------------
    template <FloatingPointType Type>
    Type TRay2<Type>::DistanceToPoint(const TVector2<Type>& point) const
    {
        return std::sqrt(SquaredDistanceToPoint(point));
    }

    //----------------------------------------------------------------------------------------------------
    ///		@brief : Get the squared distance to the closest point along the Ray.
    //----------------------------------------------------------------------------------------------------
    template <FloatingPointType Type>
    constexpr Type TRay2<Type>::SquaredDistanceToPoint(const TVector2<Type>& point) const
    {
        const Type projectedDistance = TVector2<Type>::Dot(m_direction, point - m_origin);

        // If the projected point is behind the origin, return the squared distance
        // to the origin point.
        if (projectedDistance < 0)
        {
            return TVector2<Type>::DistanceSquared(m_origin, point);
        }

        // Otherwise, get the distance squared from the projected position along the ray to
        // the query point.
        const TVector2<Type> projectedPoint = m_origin + (m_direction * projectedDistance);
        return TVector2<Type>::DistanceSquared(point, projectedPoint);
    }

    //----------------------------------------------------------------------------------------------------
    ///		@brief : Get the closest point along the Ray to the query point.  
    //----------------------------------------------------------------------------------------------------
    template <FloatingPointType Type>
    constexpr TVector2<Type> TRay2<Type>::ClosestPointToPoint(const TVector2<Type>& point) const
    {
        const Type projectedDistance = TVector2<Type>::Dot(m_direction, point - m_origin);

        // If the projected point would be behind the ray, return the origin.
        if (projectedDistance < 0)
            return m_origin;

        // Otherwise return the point that is the projected distance along the ray.
        return m_origin + (m_direction * projectedDistance);
    }

    template <FloatingPointType Type>
    std::string TRay2<Type>::ToString() const
    {
        return CombineIntoString("Origin: ", m_origin.ToString(), " Direction: ", m_direction.ToString());
    }

    //----------------------------------------------------------------------------------------------------
    ///		@brief : Construct a Ray with an Origin position and a direction.
    //----------------------------------------------------------------------------------------------------
    template <FloatingPointType Type>
    constexpr TRay3<Type>::TRay3(const TVector3<Type>& origin, const TVector3<Type>& direction)
        : m_origin(origin)
        , m_direction(direction)
    {
        m_direction.Normalize();
    }

    template <FloatingPointType Type>
    constexpr bool TRay3<Type>::operator==(const TRay3& other) const
    {
        return m_origin == other.m_origin && m_direction == other.m_direction;
    }

    //----------------------------------------------------------------------------------------------------
    ///		@brief : Calculates the position starting at the origin and moving a distance in the Ray's
    ///             direction. If the distance is negative, this will return the origin.
    ///		@param distance : Distance along the ray.
    //----------------------------------------------------------------------------------------------------
    template <FloatingPointType Type>
    constexpr TVector3<Type> TRay3<Type>::PositionAlongRay(const Type distance)
    {
        if (distance <= 0)
            return m_origin;
        
        return m_origin + (m_direction * distance);
    }

    //----------------------------------------------------------------------------------------------------
    ///		@brief : Get the distance to the closest point along the Ray. 
    //----------------------------------------------------------------------------------------------------
    template <FloatingPointType Type>
    Type TRay3<Type>::DistanceToPoint(const TVector3<Type>& point) const
    {
        return std::sqrt(SquaredDistanceToPoint(point));
    }

    //----------------------------------------------------------------------------------------------------
    ///		@brief : Get the squared distance to the closest point along the Ray.
    //----------------------------------------------------------------------------------------------------
    template <FloatingPointType Type>
    constexpr Type TRay3<Type>::SquaredDistanceToPoint(const TVector3<Type>& point) const
    {
        const Type projectedDistance = TVector3<Type>::Dot(m_direction, point - m_origin);

        // If the projected point is behind the origin, return the squared distance
        // to the origin point.
        if (projectedDistance < 0)
        {
            return TVector3<Type>::DistanceSquared(m_origin, point);
        }

        // Otherwise, get the distance squared from the projected position along the ray to
        // the query point.
        const TVector3<Type> projectedPoint = m_origin + (m_direction * projectedDistance);
        return TVector3<Type>::DistanceSquared(point, projectedPoint);
    }

    //----------------------------------------------------------------------------------------------------
    ///		@brief : Get the closest point along the Ray to the query point.  
    //----------------------------------------------------------------------------------------------------
    template <FloatingPointType Type>
    constexpr TVector3<Type> TRay3<Type>::ClosestPointToPoint(const TVector3<Type>& point) const
    {
        const Type projectedDistance = TVector3<Type>::Dot(m_direction, point - m_origin);

        // If the projected point would be behind the ray, return the origin.
        if (projectedDistance < 0)
            return m_origin;

        // Otherwise return the point that is the projected distance along the ray.
        return m_origin + (m_direction * projectedDistance);
    }

    template <FloatingPointType Type>
    std::string TRay3<Type>::ToString() const
    {
        return CombineIntoString("Origin: ", m_origin.ToString(), " Direction: ", m_direction.ToString());
    }
}
