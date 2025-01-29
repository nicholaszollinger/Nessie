// Circle.h
#pragma once
#include "Vector2.h"

namespace nes
{
    template <FloatingPointType Type>
    struct TCircle
    {
        TVector2<Type> m_center;
        Type m_radius;

        constexpr TCircle() : m_center(), m_radius(static_cast<Type>(1.0)) {}
        constexpr TCircle(const TVector2<Type>& center, Type radius) : m_center(center), m_radius(radius) {}

        constexpr Type Diameter() const { return m_radius * static_cast<Type>(2.0); }
        constexpr Type Circumference() const { return Diameter() * math::Pi<Type>(); }
        constexpr Type Area() const;

        bool ContainsPoint(const TVector2<Type>& point) const;
    };
}

NES_MATH_DECLARE_ALIASES_FOR_TEMPLATE_F(Circle);
NES_MATH_DECLARE_GLOBAL_TYPE_ALIAS_F(TCircle, Circle);

namespace nes
{
    template <FloatingPointType Type>
    constexpr Type TCircle<Type>::Area() const
    {
        return math::Pi<Type>() * math::Squared(m_radius);
    }

    //----------------------------------------------------------------------------------------------------
    ///		@brief : Tests whether a point is inside or on the bounds of the circle.
    //----------------------------------------------------------------------------------------------------
    template <FloatingPointType Type>
    bool TCircle<Type>::ContainsPoint(const TVector2<Type>& point) const
    {
        return (point - m_center).SquaredMagnitude() <= math::Squared(m_radius);
    }
}