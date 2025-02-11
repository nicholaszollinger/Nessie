// Sphere.h
#pragma once
#include "Vector3.h"

namespace nes
{
    namespace math
    {
        //----------------------------------------------------------------------------------------------------
        ///		@brief : Returns the Diameter given a radius. 
        //----------------------------------------------------------------------------------------------------
        template <FloatingPointType Type>
        Type Diameter(const Type radius)
        {
            return static_cast<Type>(2.f) * radius;
        }
        
    }
    
    //----------------------------------------------------------------------------------------------------
    ///		@brief : A 2D Sphere (circle) represented by a center point and a radius. 
    //----------------------------------------------------------------------------------------------------
    template <FloatingPointType Type>
    struct TSphere2
    {
        TVector2<Type> m_center;
        Type m_radius;

        constexpr TSphere2() : m_center(), m_radius(static_cast<Type>(1.0)) {}
        constexpr TSphere2(const TVector2<Type>& center, Type radius) : m_center(center), m_radius(radius) {}

        constexpr Type Diameter() const      { return math::Diameter(m_radius); }
        constexpr Type Circumference() const { return TSphere2::Circumference(m_radius); }
        constexpr Type Area() const          { return TSphere2::Area(m_radius); }

        static constexpr Type Area(const Type radius);
        static constexpr Type Circumference(const Type radius);

        bool ContainsPoint(const TVector2<Type>& point) const;
    };
    
    //----------------------------------------------------------------------------------------------------
    ///		@brief : Sphere represented by a center point and radius.  
    //----------------------------------------------------------------------------------------------------
    template <FloatingPointType Type>
    struct TSphere3
    {
        TVector3<Type> m_center{};
        Type m_radius = static_cast<Type>(1);

        /// Default constructor creates a unit Sphere around the origin.
        constexpr TSphere3() = default;
        constexpr TSphere3(const TVector3<Type>& center, const Type radius);

        constexpr Type Diameter() const         { return math::Diameter(m_radius); }
        constexpr Type Volume() const           { return TSphere3::Volume(m_radius); }
        constexpr Type SurfaceArea() const      { return TSphere3::SurfaceArea(m_radius); }
        
        static constexpr Type Volume(const Type radius);
        static constexpr Type SurfaceArea(const Type radius);
        
        bool ContainsPoint(const TVector3<Type>& point) const;
    };

    using Circlef = TSphere2<float>;
    using Circled = TSphere2<double>;
    using Circle = TSphere2<NES_MATH_DEFAULT_REAL_TYPE>;

    using Spheref = TSphere3<float>;
    using Sphered = TSphere3<double>;
    using Sphere = TSphere3<NES_MATH_DEFAULT_REAL_TYPE>;
}

namespace nes
{
    //----------------------------------------------------------------------------------------------------
    ///		@brief : Return the Area of a Circle given a radius. 
    //----------------------------------------------------------------------------------------------------
    template <FloatingPointType Type>
    constexpr Type TSphere2<Type>::Area(const Type radius)
    {
        return math::Pi<Type>() * math::Squared(radius);
    }

    //----------------------------------------------------------------------------------------------------
    ///		@brief : Returns the Circumference (perimeter) of a Circle given a radius.
    //----------------------------------------------------------------------------------------------------
    template <FloatingPointType Type>
    constexpr Type TSphere2<Type>::Circumference(const Type radius)
    {
        return math::Pi<Type>() * static_cast<Type>(2.f) * radius;
    }
    
    //----------------------------------------------------------------------------------------------------
    ///		@brief : Tests whether a point is inside or on the bounds of the circle.
    //----------------------------------------------------------------------------------------------------
    template <FloatingPointType Type>
    bool TSphere2<Type>::ContainsPoint(const TVector2<Type>& point) const
    {
        return (point - m_center).SquaredMagnitude() <= math::Squared(m_radius);
    }
    
    //----------------------------------------------------------------------------------------------------
    ///		@brief : Constructs a Sphere with a given position and radius.
    //----------------------------------------------------------------------------------------------------
    template <FloatingPointType Type>
    constexpr TSphere3<Type>::TSphere3(const TVector3<Type>& center, const Type radius)
        : m_center(center)
        , m_radius(radius)
    {
        //
    }

    //----------------------------------------------------------------------------------------------------
    ///		@brief : Returns the Volume of a Sphere, given a radius.
    //----------------------------------------------------------------------------------------------------
    template <FloatingPointType Type>
    constexpr Type TSphere3<Type>::Volume(const Type radius)
    {
        // V = 4/3 * pi * r^3
        return static_cast<Type>(4.0 / 3.0) * math::Pi<Type>() * math::Cubed(radius);
    }

    //----------------------------------------------------------------------------------------------------
    ///		@brief : Returns the Surface Area of a Sphere, given a radius. 
    //----------------------------------------------------------------------------------------------------
    template <FloatingPointType Type>
    constexpr Type TSphere3<Type>::SurfaceArea(const Type radius)
    {
        // S = 4 * pi * r^2.
        return static_cast<Type>(4.0) * math::Pi<Type>() * math::Squared(radius);
    }
    
    //----------------------------------------------------------------------------------------------------
    ///		@brief : Tests whether a point lies within the Sphere or on its surface. 
    //----------------------------------------------------------------------------------------------------
    template <FloatingPointType Type>
    bool TSphere3<Type>::ContainsPoint(const TVector3<Type>& point) const
    {
        // [Consider]: Probably need a tolerance check here. 
        return (point - m_center).SquaredMagnitude() <= math::Squared(m_radius);
    }
}