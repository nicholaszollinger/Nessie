// Plane.h
#pragma once
#include "Vector3.h"

namespace nes
{
    //----------------------------------------------------------------------------------------------------
    ///		@brief : Three-dimensional Plane.  
    //----------------------------------------------------------------------------------------------------
    template <FloatingPointType Type>
    struct TPlane
    {
        // Plane Normal. Any point "X" that is on the plane must satisfy Dot(m_normal, X) = m_distance;
        TVector3<Type> m_normal = TVector3<Type>::GetUpVector();
        
        // Distance of the Plane from the origin.
        Type m_distance = {};

        constexpr TPlane() = default;
        TPlane(const TVector3<Type>& normal, Type distance);
        TPlane(const TVector3<Type>& normal, const TVector3<Type>& point);
        TPlane(const TVector3<Type>& a, const TVector3<Type>& b, const TVector3<Type>& c);

        constexpr bool operator==(const TPlane& other) const;
        constexpr bool operator!=(const TPlane& other) const { return !(*this == other); }

        TVector3<Type> Origin() const;
        Type SignedDistanceToPoint(const TVector3<Type>& point) const;
        bool IsOnPlane(const TVector3<Type>& point) const;
    };
    
    using Planef = TPlane<float>;
    using Planed = TPlane<double>;
    using Plane = TPlane<NES_MATH_DEFAULT_REAL_TYPE>;
}

namespace nes
{
    //----------------------------------------------------------------------------------------------------
    ///		@brief : Construct a plane from a normal and a distance from the origin.
    //----------------------------------------------------------------------------------------------------
    template <FloatingPointType Type>
    TPlane<Type>::TPlane(const TVector3<Type>& normal, Type distance)
        : m_normal(normal)
        , m_distance(distance)
    {
        m_normal.Normalize(); // ensure that the plane is normalized.
    }

    //----------------------------------------------------------------------------------------------------
    ///		@brief : Construct a Plane from a normal and a point on that normal.
    //----------------------------------------------------------------------------------------------------
    template <FloatingPointType Type>
    TPlane<Type>::TPlane(const TVector3<Type>& normal, const TVector3<Type>& point)
    {
        m_normal = normal.Normalized(); // ensure normalization.
        m_distance = TVector3<Type>::Dot(m_normal, point);
    }

    //----------------------------------------------------------------------------------------------------
    ///		@brief : Construct a plane from 3 non-collinear points (ordered counterclockwise).
    //----------------------------------------------------------------------------------------------------
    template <FloatingPointType Type>
    TPlane<Type>::TPlane(const TVector3<Type>& a, const TVector3<Type>& b,
        const TVector3<Type>& c)
    {
        m_normal = TVector3<Type>::Cross(b - a, c - a).Normalized();
        m_distance = TVector3<Type>::Dot(m_normal, a);
    }

    template <FloatingPointType Type>
    constexpr bool TPlane<Type>::operator==(const TPlane& other) const
    {
        return m_normal == other.m_normal && math::CheckEqualFloats(m_distance, other.m_distance);
    }

    //----------------------------------------------------------------------------------------------------
    ///		@brief : Get this Plane's origin. 
    //----------------------------------------------------------------------------------------------------
    template <FloatingPointType Type>
    TVector3<Type> TPlane<Type>::Origin() const
    {
        // m_distance represents a distance from the Origin, and m_normal is the direction from the Origin,
        // so the Plane's origin will be the m_distance away from the Origin in the direction of the normal.
        return m_normal * m_distance;
    }

    //----------------------------------------------------------------------------------------------------
    ///		@brief : Returns the distance of a point to the plane. If the result is negative, then the point is
    ///             behind the plane. If positive, the point is in front. If equal to zero, then the point is on
    ///             the plane & considered coplanar.
    ///             This can also be thought of as the Plane's Dot product.
    //----------------------------------------------------------------------------------------------------
    template <FloatingPointType Type>
    Type TPlane<Type>::SignedDistanceToPoint(const TVector3<Type>& point) const
    {
        return TVector3<Type>::Dot(m_normal, point);
    }

    //----------------------------------------------------------------------------------------------------
    ///		@brief : Returns true if the point lies on the plane's surface. This is the same as checking if
    ///             SignedDistanceToPoint is equal to zero.
    //----------------------------------------------------------------------------------------------------
    template <FloatingPointType Type>
    bool TPlane<Type>::IsOnPlane(const TVector3<Type>& point) const
    {
        return math::CheckEqualFloats(SignedDistanceToPoint(point), static_cast<Type>(0));
    }
}
