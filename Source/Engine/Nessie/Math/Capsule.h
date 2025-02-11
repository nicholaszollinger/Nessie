// Capsule.h
#pragma once
#include "Segment.h"
#include "Sphere.h"

namespace nes
{
    //----------------------------------------------------------------------------------------------------
    // TODO:
    // - Intersect with another Capsule.
    // - Intersect with a Sphere.
    // - Constructor for a set of points.
    //----------------------------------------------------------------------------------------------------
    
    //----------------------------------------------------------------------------------------------------
    ///		@brief : A 2D Capsule stored as a line segment and a radius. 
    //----------------------------------------------------------------------------------------------------
    template <FloatingPointType Type>
    struct TCapsule2
    {
        TSegment2<Type> m_segment; // The medial line between the center of the two circules.
        Type m_radius{};           // Radius of the two end-cap circles.

        constexpr TCapsule2() = default;
        constexpr TCapsule2(const TSegment2<Type>& segment, Type radius);
        constexpr TCapsule2(const TVector2<Type>& start, const TVector2<Type>& end, Type radius);

        Type Area() const;
        Type Length() const;
        constexpr Type SquaredLength() const;
        constexpr TVector3<Type> Center() const;
    };

    //----------------------------------------------------------------------------------------------------
    ///     @brief : A 3D Capsule stored as a line segment and a radius. 
    //----------------------------------------------------------------------------------------------------
    template <FloatingPointType Type>
    struct TCapsule3
    {
        TSegment3<Type> m_segment{}; // The medial line between the center of the Spheres. 
        Type m_radius{};             // Radius of the two end-cap spheres.
        
        constexpr TCapsule3() = default;
        constexpr TCapsule3(const TSegment3<Type>& segment, Type radius);
        constexpr TCapsule3(const TVector3<Type>& start, const TVector3<Type>& end, Type radius);

        Type Volume() const;
        Type Length() const;
        constexpr Type SquaredLength() const;
        constexpr TVector3<Type> Center() const;
    };

    using Capsule2f = TCapsule2<float>;
    using Capsule2d = TCapsule2<double>;
    using Capsule2D = TCapsule2<NES_MATH_DEFAULT_REAL_TYPE>;
    
    using Capsule3f = TCapsule3<float>;
    using Capsule3d = TCapsule3<double>;
    using Capsule = TCapsule3<NES_MATH_DEFAULT_REAL_TYPE>;
}

namespace nes
{
    template <FloatingPointType Type>
    constexpr TCapsule2<Type>::TCapsule2(const TSegment2<Type>& segment, Type radius)
        : m_segment(segment)
        , m_radius(radius)
    {
        //
    }

    template <FloatingPointType Type>
    constexpr TCapsule2<Type>::TCapsule2(const TVector2<Type>& start, const TVector2<Type>& end, Type radius)
        : m_segment(start, end)
        , m_radius(radius)
    {
        //
    }

    //----------------------------------------------------------------------------------------------------
    ///		@brief : Returns the Area of the Capsule. 
    //----------------------------------------------------------------------------------------------------
    template <FloatingPointType Type>
    Type TCapsule2<Type>::Area() const
    {
        // Area of the rectangle plus the area of one circle.
        const Type rectArea = m_segment.Length() * m_radius * static_cast<Type>(2.f);
        return rectArea + (math::Pi<Type>() * math::Squared(m_radius));
    }

    //----------------------------------------------------------------------------------------------------
    //		NOTES:
    //      Unreal has the Length functions return just the length of the segment. I feel like you would just
    //      use the public segment variable to find that if you needed it. Perhaps there is a reason I will
    //      find out later.
    //
    ///		@brief : Returns the total length of the segment plus twice the radius. 
    //----------------------------------------------------------------------------------------------------
    template <FloatingPointType Type>
    Type TCapsule2<Type>::Length() const
    {
        return std::sqrt(SquaredLength());
    }

    //----------------------------------------------------------------------------------------------------
    //		NOTES:
    //      Unreal has the Length functions return just the length of the segment. I feel like you would just
    //      use the public segment variable to find that if you needed it. Perhaps there is a reason I will
    //      find out later.
    //
    ///		@brief : Returns the squared total length of the segment plus twice the radius. 
    //----------------------------------------------------------------------------------------------------
    template <FloatingPointType Type>
    constexpr Type TCapsule2<Type>::SquaredLength() const
    {
        return m_segment.SquaredLength() + (static_cast<Type>(2) * math::Squared(m_radius));
    }

    //----------------------------------------------------------------------------------------------------
    ///		@brief : Returns the center of the capsule, which is the midpoint on the medial line segment. 
    //----------------------------------------------------------------------------------------------------
    template <FloatingPointType Type>
    TVector3<Type> TCapsule2<Type>::Center() const
    {
        return m_segment.Center();
    }

    template <FloatingPointType Type>
    constexpr TCapsule3<Type>::TCapsule3(const TSegment3<Type>& segment, Type radius)
        : m_segment(segment)
        , m_radius(radius)
    {
        //
    }

    template <FloatingPointType Type>
    constexpr TCapsule3<Type>::TCapsule3(const TVector3<Type>& start, const TVector3<Type>& end, Type radius)
        : m_segment(start, end)
        , m_radius(radius)
    {
        //
    }

    //----------------------------------------------------------------------------------------------------
    //      NOTES:
    //      - This is a function that would be sped up if the line segment stored an additional extent.
    //      - Here I have to do the sqrt when getting the Length of the segment.
    //
    ///		@brief : Returns the Volume of the Capsule.
    //----------------------------------------------------------------------------------------------------
    template <FloatingPointType Type>
    Type TCapsule3<Type>::Volume() const
    {
        // Pi * radius^2
        Type piRSqr = math::Pi<Type>() * math::Squared(m_radius);

        return piRSqr * (static_cast<Type>(2) * m_segment.Length()) // Cylinder Volume 
            + static_cast<Type>(4.f / 3.f) * piRSqr * m_radius;     // + Sphere Volume
    }

    //----------------------------------------------------------------------------------------------------
    //		NOTES:
    //      Unreal has the Length functions return just the length of the segment. I feel like you would just
    //      use the public segment variable to find that if you needed it. Perhaps there is a reason I will
    //      find out later.
    //
    ///		@brief : Returns the total length of the segment plus twice the radius. 
    //----------------------------------------------------------------------------------------------------
    template <FloatingPointType Type>
    Type TCapsule3<Type>::Length() const
    {
        return std::sqrt(SquaredLength());
    }

    //----------------------------------------------------------------------------------------------------
    //		NOTES:
    //      Unreal has the Length functions return just the length of the segment. I feel like you would just
    //      use the public segment variable to find that if you needed it. Perhaps there is a reason I will
    //      find out later.
    //
    ///		@brief : Returns the squared total length of the segment plus twice the radius. 
    //----------------------------------------------------------------------------------------------------
    template <FloatingPointType Type>
    constexpr Type TCapsule3<Type>::SquaredLength() const
    {
        return m_segment.SquaredLength() + (static_cast<Type>(2) * math::Squared(m_radius));
    }

    //----------------------------------------------------------------------------------------------------
    ///		@brief : Returns the center of the capsule, which is the midpoint on the medial line segment. 
    //----------------------------------------------------------------------------------------------------
    template <FloatingPointType Type>
    constexpr TVector3<Type> TCapsule3<Type>::Center() const
    {
        return m_segment.Center();
    }
}
