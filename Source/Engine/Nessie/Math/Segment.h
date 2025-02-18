// Segment.h
#pragma once
#include "Vector3.h"

namespace nes
{
    //----------------------------------------------------------------------------------------------------
    //      Unreal decided to store the segment as an origin, direction and extent (half-length).
    //      This adds an extra float, but would make certain operations faster, namely the Length & direction.
    //
    ///		@brief : A Line Segment between a start and end point, expressed in 2D coordinates. 
    //----------------------------------------------------------------------------------------------------
    template <FloatingPointType Type>
    struct TSegment2
    {
        TVector2<Type> m_start{};
        TVector2<Type> m_end{};

        constexpr TSegment2() = default;
        constexpr TSegment2(const TVector2<Type>& start, const TVector2<Type>& end);

        Type Length() const;
        constexpr Type SquaredLength() const;
        constexpr TVector2<Type> Center() const;
        constexpr TVector2<Type> ClosestPoint(const TVector2<Type>& queryPoint);
    };
    
    //----------------------------------------------------------------------------------------------------
    ///		@brief : A Line Segment between a start and end point, expressed in 3D coordinates. 
    //----------------------------------------------------------------------------------------------------
    template <FloatingPointType Type>
    struct TSegment3
    {
        TVector3<Type> m_start{};
        TVector3<Type> m_end{};

        constexpr TSegment3() = default;
        constexpr TSegment3(const TVector3<Type>& start, const TVector3<Type>& end);
        
        Type Length() const;
        constexpr Type SquaredLength() const;
        constexpr TVector3<Type> Center() const;
        constexpr TVector3<Type> ClosestPoint(const TVector3<Type>& queryPoint);
    };
    
    using Segment2f = TSegment2<float>;
    using Segment2d = TSegment2<double>;
    using Segment2D = TSegment2<NES_MATH_DEFAULT_REAL_TYPE>;
    
    using Segment3f = TSegment3<float>;
    using Segment3d = TSegment3<double>;
    using Segment = TSegment3<NES_MATH_DEFAULT_REAL_TYPE>;
}

namespace nes
{
    //----------------------------------------------------------------------------------------------------
    ///		@brief : Constructs a Line Segment between the start and end points. 
    //----------------------------------------------------------------------------------------------------
    template <FloatingPointType Type>
    constexpr TSegment2<Type>::TSegment2(const TVector2<Type>& start, const TVector2<Type>& end)
        : m_start(start)
        , m_end(end)
    {
        //
    }

    //----------------------------------------------------------------------------------------------------
    ///		@brief : Returns the Length of the line segment. 
    //----------------------------------------------------------------------------------------------------
    template <FloatingPointType Type>
    Type TSegment2<Type>::Length() const
    {
        return (m_end - m_start).Magnitude();
    }

    //----------------------------------------------------------------------------------------------------
    ///		@brief : Returns the Squared Length of the line segment.
    //----------------------------------------------------------------------------------------------------
    template <FloatingPointType Type>
    constexpr Type TSegment2<Type>::SquaredLength() const
    {
        return (m_end - m_start).SquaredMagnitude();
    }

    //----------------------------------------------------------------------------------------------------
    ///		@brief : Returns the center of the line segment, or midpoint. 
    //----------------------------------------------------------------------------------------------------
    template <FloatingPointType Type>
    constexpr TVector2<Type> TSegment2<Type>::Center() const
    {
        return (m_start + m_end) * static_cast<Type>(0.5f);
    }

    //----------------------------------------------------------------------------------------------------
    ///		@brief : Return the closest point on the Segment to the query point. 
    //----------------------------------------------------------------------------------------------------
    template <FloatingPointType Type>
    constexpr TVector2<Type> TSegment2<Type>::ClosestPoint(const TVector2<Type>& queryPoint)
    {
        TVector2<Type> toEnd = (m_end - m_start);
        Type distanceSqr = toEnd.SquaredMagnitude();

        Type projectedDistance = TVector2<Type>::Dot(queryPoint - m_start, toEnd);
        if (projectedDistance < 0)
            return m_start;

        if (math::Squared(projectedDistance) > distanceSqr)
            return m_end;

        return m_start + projectedDistance * toEnd;
    }
    
    //----------------------------------------------------------------------------------------------------
    ///		@brief : Constructs a line segment between the start and end points. 
    //----------------------------------------------------------------------------------------------------
    template <FloatingPointType Type>
    constexpr TSegment3<Type>::TSegment3(const TVector3<Type>& start, const TVector3<Type>& end)
        : m_start(start)
        , m_end(end)
    {
        //
    }

    //----------------------------------------------------------------------------------------------------
    ///		@brief : Returns the length of the line segment. 
    //----------------------------------------------------------------------------------------------------
    template <FloatingPointType Type>
    Type TSegment3<Type>::Length() const
    {
        return (m_end - m_start).Magnitude();
    }

    //----------------------------------------------------------------------------------------------------
    ///		@brief : Returns the squared length of the line segment. 
    //----------------------------------------------------------------------------------------------------
    template <FloatingPointType Type>
    constexpr Type TSegment3<Type>::SquaredLength() const
    {
        return (m_end - m_start).SquaredMagnitude();
    }

    //----------------------------------------------------------------------------------------------------
    ///		@brief : Returns the center of the line segment, or midpoint. 
    //----------------------------------------------------------------------------------------------------
    template <FloatingPointType Type>
    constexpr TVector3<Type> TSegment3<Type>::Center() const
    {
        return (m_start + m_end) * static_cast<Type>(0.5f);
    }

    //----------------------------------------------------------------------------------------------------
    ///		@brief : Return the closest point on the Segment to the query point. 
    //----------------------------------------------------------------------------------------------------
    template <FloatingPointType Type>
    constexpr TVector3<Type> TSegment3<Type>::ClosestPoint(const TVector3<Type>& queryPoint)
    {
        TVector3<Type> toEnd = (m_end - m_start);
        Type distanceSqr = toEnd.SquaredMagnitude();

        Type projectedDistance = TVector3<Type>::Dot(queryPoint - m_start, toEnd);
        if (projectedDistance < 0)
            return m_start;

        if (math::Squared(projectedDistance) > distanceSqr)
            return m_end;

        return m_start + projectedDistance * toEnd;
    }
}