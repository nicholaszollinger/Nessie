// Lines.h
#pragma once
#include "Vector3.h"

namespace nes
{
    //----------------------------------------------------------------------------------------------------
    ///		@brief : Defines an infinite Line expressed in 2D coordinates. 
    //----------------------------------------------------------------------------------------------------
    template <FloatingPointType Type>
    struct TLine2
    {
        /// Origin or Center point of a line. Really, this is an arbitrary point on an infinite line, but
        /// it can be useful to convert between lines and Rays.
        TVector2<Type> m_origin = TVector2<Type>::GetZeroVector();

        /// Normalized Direction of the Line, the "Slope".
        TVector2<Type> m_direction = TVector2<Type>::GetRightVector();

        constexpr TLine2() = default;
        constexpr TLine2(const TVector2<Type>& origin, const TVector2<Type>& direction);

        static constexpr TLine2 MakeFromTwoPoints(const TVector2<Type>& a, const TVector2<Type>& b);
    };

    //----------------------------------------------------------------------------------------------------
    ///		@brief : A Line Segment between a start and end point, expressed in 2D coordinates. 
    //----------------------------------------------------------------------------------------------------
    template <FloatingPointType Type>
    struct TLineSegment2
    {
        TVector2<Type> m_start{};
        TVector2<Type> m_end{};

        constexpr TLineSegment2() = default;
        constexpr TLineSegment2(const TVector2<Type>& start, const TVector2<Type>& end);

        Type Length() const;
        Type SquaredLength() const;
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

        /** Normalized Direction of the Line, the "Slope". */
        TVector3<Type> m_direction = TVector3<Type>::GetRightVector();

        constexpr TLine3() = default;
        constexpr TLine3(const TVector3<Type>& origin, const TVector3<Type>& direction);

        static constexpr TLine3 MakeFromTwoPoints(const TVector3<Type>& a, const TVector3<Type>& b);
    };

    //----------------------------------------------------------------------------------------------------
    ///		@brief : A Line Segment between a start and end point, expressed in 2D coordinates. 
    //----------------------------------------------------------------------------------------------------
    template <FloatingPointType Type>
    struct TLineSegment3
    {
        TVector3<Type> m_start{};
        TVector3<Type> m_end{};

        constexpr TLineSegment3() = default;
        constexpr TLineSegment3(const TVector3<Type>& start, const TVector3<Type>& end);

        Type Length() const;
        Type SquaredLength() const;
    };
    
    using Line2f = TLine2<float>;
    using Line2d = TLine2<double>;
    using Line2D = TLine2<NES_MATH_DEFAULT_REAL_TYPE>;
    
    using LineSegment2f = TLineSegment2<float>;
    using LineSegment2d = TLineSegment2<double>;
    using LineSegment2D = TLineSegment2<NES_MATH_DEFAULT_REAL_TYPE>;
    
    using Line3f = TLine3<float>;
    using Line3d = TLine3<double>;
    using Line = TLine3<NES_MATH_DEFAULT_REAL_TYPE>;
    
    using LineSegment3f = TLineSegment3<float>;
    using LineSegment3d = TLineSegment3<double>;
    using LineSegment = TLine3<NES_MATH_DEFAULT_REAL_TYPE>;
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
        m_direction = direction.GetNormalized();
    }

    //----------------------------------------------------------------------------------------------------
    ///		@brief : Create a Line that intersects both points a and b.
    //----------------------------------------------------------------------------------------------------
    template <FloatingPointType Type>
    constexpr TLine2<Type> TLine2<Type>::MakeFromTwoPoints(const TVector2<Type>& a, const TVector2<Type>& b)
    {
        return TLine2<Type>(a, (b - a).Normalize());
    }

    //----------------------------------------------------------------------------------------------------
    ///		@brief : Constructs a Line Segment between the start and end points. 
    //----------------------------------------------------------------------------------------------------
    template <FloatingPointType Type>
    constexpr TLineSegment2<Type>::TLineSegment2(const TVector2<Type>& start, const TVector2<Type>& end)
        : m_start(start)
        , m_end(end)
    {
        //
    }

    //----------------------------------------------------------------------------------------------------
    ///		@brief : Returns the Length of the line segment. 
    //----------------------------------------------------------------------------------------------------
    template <FloatingPointType Type>
    Type TLineSegment2<Type>::Length() const
    {
        return (m_end - m_start).Magnitude();
    }

    //----------------------------------------------------------------------------------------------------
    ///		@brief : Returns the Squared Length of the line segment.
    //----------------------------------------------------------------------------------------------------
    template <FloatingPointType Type>
    Type TLineSegment2<Type>::SquaredLength() const
    {
        return (m_end - m_start).SquaredMagnitude();
    }

    //----------------------------------------------------------------------------------------------------
    ///		@brief : Construct a line from an Origin and Direction 
    //----------------------------------------------------------------------------------------------------
    template <FloatingPointType Type>
    constexpr TLine3<Type>::TLine3(const TVector3<Type>& origin, const TVector3<Type>& direction)
        : m_origin(origin)
    {
        m_direction = direction.GetNormalized();
    }

    //----------------------------------------------------------------------------------------------------
    ///		@brief : Create a Line that intersects both points a and b.
    //----------------------------------------------------------------------------------------------------
    template <FloatingPointType Type>
    constexpr TLine3<Type> TLine3<Type>::MakeFromTwoPoints(const TVector3<Type>& a, const TVector3<Type>& b)
    {
        return TLine3<Type>(a, (b - a).Normalize());
    }

    //----------------------------------------------------------------------------------------------------
    ///		@brief : Constructs a line segment between the start and end points. 
    //----------------------------------------------------------------------------------------------------
    template <FloatingPointType Type>
    constexpr TLineSegment3<Type>::TLineSegment3(const TVector3<Type>& start, const TVector3<Type>& end)
        : m_start(start)
        , m_end(end)
    {
        //
    }

    //----------------------------------------------------------------------------------------------------
    ///		@brief : Returns the length of the line segment. 
    //----------------------------------------------------------------------------------------------------
    template <FloatingPointType Type>
    Type TLineSegment3<Type>::Length() const
    {
        return (m_end - m_start).Magnitude();
    }

    //----------------------------------------------------------------------------------------------------
    ///		@brief : Returns the squared length of the line segement. 
    //----------------------------------------------------------------------------------------------------
    template <FloatingPointType Type>
    Type TLineSegment3<Type>::SquaredLength() const
    {
        return (m_end - m_start).SquaredMagnitude();
    }
}
