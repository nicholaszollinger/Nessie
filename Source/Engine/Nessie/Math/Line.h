// Lines.h
#pragma once
#include "Geometry.h"
#include "Vector3.h"

namespace nes
{
    //----------------------------------------------------------------------------------------------------
    ///		@brief : Defines an infinite Line expressed in 2D coordinates. In 2D, a line can also be thought of
    ///         as a Plane.
    //----------------------------------------------------------------------------------------------------
    template <FloatingPointType Type>
    struct TLine2
    {
        /// Origin or Center point of a line. Really, this is an arbitrary point on an infinite line, but
        /// it can be useful to convert between lines and Rays.
        TVector2<Type> m_origin = TVector2<Type>::GetZeroVector();

        /// Normalized Direction of the Line, the "Slope". The Line will extend infinitely in this
        /// and the exact opposite direction.
        TVector2<Type> m_direction = TVector2<Type>::GetRightVector();

        constexpr TLine2() = default;
        constexpr TLine2(const TVector2<Type>& origin, const TVector2<Type>& direction);

        TVector2<Type> PointAlongLine(const Type distance);
        TVector2<Type> ClosestPoint(const TVector2<Type>& queryPoint);
        Type ProjectedDistance(const TVector2<Type>& point);
        int WhichSide(const TVector2<Type>& queryPoint, const Type tolerance = math::PrecisionDelta<Type>());

        static constexpr TLine2 PerpendicularBisector(const TVector2<Type>& a, const TVector2<Type>& b);
        static constexpr TLine2 MakeFromTwoPoints(const TVector2<Type>& a, const TVector2<Type>& b);
    };

    //----------------------------------------------------------------------------------------------------
    ///		@brief : Defines an infinite line expressed in 3D coordinates.
    //----------------------------------------------------------------------------------------------------
    template <FloatingPointType Type>
    struct TLine3
    {
        /// Origin or Center point of a line. Really, this is an arbitrary point on an infinite line, but
        /// it can be useful to convert between lines and Rays.
        TVector3<Type> m_origin = TVector3<Type>::GetZeroVector();

        /// Normalized Direction of the Line, the "Slope". The Line will extend infinitely in this
        /// and the exact opposite direction.
        TVector3<Type> m_direction = TVector3<Type>::GetRightVector();

        constexpr TLine3() = default;
        constexpr TLine3(const TVector3<Type>& origin, const TVector3<Type>& direction);

        TVector3<Type> PointAlongLine(const Type distance);
        TVector3<Type> ClosestPoint(const TVector3<Type>& queryPoint);
        Type ProjectedDistance(const TVector3<Type>& point);

        static constexpr TLine3 MakeFromTwoPoints(const TVector3<Type>& a, const TVector3<Type>& b);
    };
    
    using Line2f = TLine2<float>;
    using Line2d = TLine2<double>;
    using Line2D = TLine2<NES_MATH_DEFAULT_REAL_TYPE>;
    
    using Line3f = TLine3<float>;
    using Line3d = TLine3<double>;
    using Line = TLine3<NES_MATH_DEFAULT_REAL_TYPE>;
}

namespace nes
{
    //----------------------------------------------------------------------------------------------------
    ///		@brief : Construct a line from an origin and direction. 
    //----------------------------------------------------------------------------------------------------
    template <FloatingPointType Type>
    constexpr TLine2<Type>::TLine2(const TVector2<Type>& origin, const TVector2<Type>& direction)
        : m_origin(origin)
    {
        m_direction = direction.Normalized();
    }

    //----------------------------------------------------------------------------------------------------
    ///		@brief : Create a perpendicular line to the segment spanning from point "a" to point "b".
    ///             Every point on this line will be equidistant to both points "a" and "b".
    //----------------------------------------------------------------------------------------------------
    template <FloatingPointType Type>
    constexpr TLine2<Type> TLine2<Type>::PerpendicularBisector(const TVector2<Type>& a, const TVector2<Type>& b)
    {
        TVector2<Type> midpoint = (a + b) * 0.5f;
        return TLine2(midpoint, b - a);
    }

    //----------------------------------------------------------------------------------------------------
    ///		@brief : Create a Line that intersects both points a and b.
    //----------------------------------------------------------------------------------------------------
    template <FloatingPointType Type>
    constexpr TLine2<Type> TLine2<Type>::MakeFromTwoPoints(const TVector2<Type>& a, const TVector2<Type>& b)
    {
        return TLine2(a, (b - a).Normalize());
    }

    //----------------------------------------------------------------------------------------------------
    ///		@brief : Returns a point on the line that with a "distance" from the origin. 
    //----------------------------------------------------------------------------------------------------
    template <FloatingPointType Type>
    TVector2<Type> TLine2<Type>::PointAlongLine(const Type distance)
    {
        return m_origin + m_direction * distance;
    }

    //----------------------------------------------------------------------------------------------------
    ///		@brief : Returns the point on the line that is the closest to the query point. 
    //----------------------------------------------------------------------------------------------------
    template <FloatingPointType Type>
    TVector2<Type> TLine2<Type>::ClosestPoint(const TVector2<Type>& queryPoint)
    {
        // Get the signed distance (dot product) of the query point and the origin, then
        // return the point that is that distance along the line.
        const Type projectedDistance = ProjectedDistance(queryPoint);
        return m_origin + m_direction * projectedDistance;
    }

    //----------------------------------------------------------------------------------------------------
    ///		@brief : Returns the signed distance between the projected point and the origin. This is the
    ///             dot product of the direction of the line and the vector spanning from the origin to the point.
    //----------------------------------------------------------------------------------------------------
    template <FloatingPointType Type>
    Type TLine2<Type>::ProjectedDistance(const TVector2<Type>& point)
    {
        return TVector2<Type>::Dot(point - m_origin, m_direction);
    }

    //----------------------------------------------------------------------------------------------------
    //		NOTES:
    //		
    ///		@brief : Determines which side the query point is located on the line with respected to the direction.
    ///             - If the result is +1, then the point is to the "right" of the line.
    ///             - If the result is -1, then the point is to the "left" of the line.
    ///             - If the result is 0, then the point is on the line.
    ///     @param queryPoint : Point that we are checking.
    ///     @param tolerance : Optional tolerance given when determining if the point is on the line.
    //----------------------------------------------------------------------------------------------------
    template <FloatingPointType Type>
    int TLine2<Type>::WhichSide(const TVector2<Type>& queryPoint, const Type tolerance)
    {
        // Calculate the 2x2 determinant.
        // The top row is the vector that goes to the point.
        // The bottom row is direction of the line.
        const TVector2 toPoint = queryPoint - m_origin;
        Type determinant = toPoint.x * m_direction.y - toPoint.y * m_direction.x;

        if (determinant > tolerance)
            return +1;

        return determinant < -tolerance ? -1 : 0;
    }

    //----------------------------------------------------------------------------------------------------
    ///		@brief : Returns a point on the line that with a "distance" from the origin. 
    //----------------------------------------------------------------------------------------------------
    template <FloatingPointType Type>
    TVector3<Type> TLine3<Type>::PointAlongLine(const Type distance)
    {
        return m_origin + m_direction * distance;
    }

    //----------------------------------------------------------------------------------------------------
    ///		@brief : Returns the point on the line that is the closest to the query point. 
    //----------------------------------------------------------------------------------------------------
    template <FloatingPointType Type>
    TVector3<Type> TLine3<Type>::ClosestPoint(const TVector3<Type>& queryPoint)
    {
        // Get the signed distance (dot product) of the query point and the origin, then
        // return the point that is that distance along the line.
        Type projectedDistance = ProjectedDistance(queryPoint);
        return m_origin + m_direction * projectedDistance;
    }

    //----------------------------------------------------------------------------------------------------
    ///		@brief : Returns the signed distance between the projected point and the origin. This is the
    ///             dot product of the direction of the line and the vector spanning from the origin to the point.
    //----------------------------------------------------------------------------------------------------
    template <FloatingPointType Type>
    Type TLine3<Type>::ProjectedDistance(const TVector3<Type>& point)
    {
        return TVector3<Type>::Dot(point - m_origin, m_direction);
    }

    //----------------------------------------------------------------------------------------------------
    ///		@brief : Construct a line from an Origin and Direction 
    //----------------------------------------------------------------------------------------------------
    template <FloatingPointType Type>
    constexpr TLine3<Type>::TLine3(const TVector3<Type>& origin, const TVector3<Type>& direction)
        : m_origin(origin)
    {
        m_direction = direction.Normalized();
    }

    //----------------------------------------------------------------------------------------------------
    ///		@brief : Create a Line that intersects both points a and b.
    //----------------------------------------------------------------------------------------------------
    template <FloatingPointType Type>
    constexpr TLine3<Type> TLine3<Type>::MakeFromTwoPoints(const TVector3<Type>& a, const TVector3<Type>& b)
    {
        return TLine3(a, (b - a).Normalize());
    }
}
