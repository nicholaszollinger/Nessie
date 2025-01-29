// Sphere.h
#pragma once
#include "Vector3.h"

namespace nes
{
    //----------------------------------------------------------------------------------------------------
    //		NOTES:
    //		
    ///		@brief : Sphere class containing a center point and radius.  
    //----------------------------------------------------------------------------------------------------
    template <FloatingPointType Type>
    struct TSphere
    {
        TVector3<Type> m_center{};
        Type m_radius = static_cast<Type>(1);

        /// Default constructor creates a unit Sphere around the origin.
        constexpr TSphere() = default;
        constexpr TSphere(const TVector3<Type>& center, const Type radius);

        constexpr Type Diameter() const { return m_radius * static_cast<Type>(2.0); }
        constexpr Type Volume() const;
        constexpr Type SurfaceArea() const;
        
        bool ContainsPoint(const TVector3<Type>& point) const;
    };
}

NES_MATH_DECLARE_ALIASES_FOR_TEMPLATE_F(Sphere);
NES_MATH_DECLARE_GLOBAL_TYPE_ALIAS_F(TSphere, Sphere);

namespace nes
{
    //----------------------------------------------------------------------------------------------------
    ///		@brief : Constructs a Sphere with a given position and radius.
    //----------------------------------------------------------------------------------------------------
    template <FloatingPointType Type>
    constexpr TSphere<Type>::TSphere(const TVector3<Type>& center, const Type radius)
        : m_center(center)
        , m_radius(radius)
    {
        //
    }

    template <FloatingPointType Type>
    constexpr Type TSphere<Type>::Volume() const
    {
        // V = 4/3 * pi * r^3
        return static_cast<Type>(4.0 / 3.0) * math::Pi<Type>() * math::Cubed(m_radius);
    }

    template <FloatingPointType Type>
    constexpr Type TSphere<Type>::SurfaceArea() const
    {
        // S = 4 * pi * r^2.
        return static_cast<Type>(4.0) * math::Pi<Type>() * math::Squared(m_radius);
    }
    
    //----------------------------------------------------------------------------------------------------
    ///		@brief : Tests whether a point lies within the Sphere or on its surface. 
    //----------------------------------------------------------------------------------------------------
    template <FloatingPointType Type>
    bool TSphere<Type>::ContainsPoint(const TVector3<Type>& point) const
    {
        // [Consider]: Probably need a tolerance check here. 
        return (point - m_center).SquaredMagnitude() <= math::Squared(m_radius);
    }
}