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
}

NES_MATH_DECLARE_ALIASES_FOR_TEMPLATE_F(Line2)
NES_MATH_DECLARE_ALIASES_FOR_TEMPLATE_F(Line3)
NES_MATH_DECLARE_GLOBAL_TYPE_ALIAS_F(TLine2, Line2D);
NES_MATH_DECLARE_GLOBAL_TYPE_ALIAS_F(TLine3, Line);

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
}
