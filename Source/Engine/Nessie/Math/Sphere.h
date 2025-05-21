// Sphere.h
#pragma once
#include "Vector3.h"

namespace nes
{
    template <FloatingPointType Type>
    struct TSphere2;

    template <FloatingPointType Type>
    struct TSphere3;
    
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

        template <FloatingPointType Type>
        void GrowSphereToContainPoint2(TSphere2<Type>& circle, const TVector2<Type>& point);

        template <FloatingPointType Type>
        void GrowSphereToContainPoint3(TSphere3<Type>& sphere, const TVector3<Type>& point);

        template <FloatingPointType Type>
        void ApproximateSphereFromDistantPoints2(TSphere2<Type>& circle, const TVector2<Type>* points, const size_t count);

        template <FloatingPointType Type>
        void ApproximateSphereFromDistantPoints3(TSphere3<Type>& sphere, const TVector3<Type>* points, const size_t count);

        template <FloatingPointType Type>
        void RitterBoundingSphere2(TSphere2<Type>& circle, const TVector2<Type>* points, const size_t count);

        template <FloatingPointType Type>
        void RitterBoundingSphere3(TSphere3<Type>& sphere, const TVector3<Type>* points, const size_t count);
    }
    
    //----------------------------------------------------------------------------------------------------
    ///		@brief : A 2D Sphere (circle) represented by a center point and a radius. 
    //----------------------------------------------------------------------------------------------------
    template <FloatingPointType Type>
    struct TSphere2
    {
        TVector2<Type> m_center{};
        Type m_radius = static_cast<Type>(0.f);
        
        constexpr TSphere2() = default;
        constexpr TSphere2(const TVector2<Type>& center, Type radius);
        TSphere2(const TVector2<Type>* points, const size_t count);

        constexpr Type Diameter() const      { return math::Diameter(m_radius); }
        constexpr Type Circumference() const { return TSphere2::Circumference(m_radius); }
        constexpr Type Area() const          { return TSphere2::Area(m_radius); }
        
        constexpr bool Intersects(const TSphere2& other) const;
        constexpr bool ContainsPoint(const TVector2<Type>& point) const;

        static constexpr Type Area(const Type radius);
        static constexpr Type Circumference(const Type radius);
    };

    //----------------------------------------------------------------------------------------------------
    ///		@brief : Sphere represented by a center point and radius.  
    //----------------------------------------------------------------------------------------------------
    template <FloatingPointType Type>
    struct TSphere3
    {
        TVector3<Type> m_center{};
        Type m_radius = static_cast<Type>(0.f);
        
        constexpr TSphere3() = default;
        constexpr TSphere3(const TVector3<Type>& center, const Type radius);
        TSphere3(const TVector3<Type>* points, const size_t count);

        constexpr Type Diameter() const         { return math::Diameter(m_radius); }
        constexpr Type Volume() const           { return TSphere3::Volume(m_radius); }
        constexpr Type SurfaceArea() const      { return TSphere3::SurfaceArea(m_radius); }
        
        constexpr bool Intersects(const TSphere3& other) const;
        constexpr bool ContainsPoint(const TVector3<Type>& point) const;

        void EncapsulatePoint(const TVector3<Type>& point);
        
        static constexpr Type Volume(const Type radius);
        static constexpr Type SurfaceArea(const Type radius);
    };

    using Circlef = TSphere2<float>;
    using Circled = TSphere2<double>;
    using Circle = TSphere2<NES_PRECISION_TYPE>;

    using Spheref = TSphere3<float>;
    using Sphered = TSphere3<double>;
    using Sphere = TSphere3<NES_PRECISION_TYPE>;
}

namespace nes
{
    namespace math
    {
        //----------------------------------------------------------------------------------------------------
        //		NOTES:
        //      pg 90 of "Real-Time Collision Detection".
        //		
        ///		@brief : Given a circle, and a point, grow the circle (if needed) to encompass the point. 
        //----------------------------------------------------------------------------------------------------
        template <FloatingPointType Type>
        void GrowSphereToContainPoint2(TSphere2<Type>& circle, const TVector2<Type>& point)
        {
            TVector2<Type> toPoint = point - circle.m_center;
            Type distSqr = toPoint.SquaredMagnitude();

            // Only update the Sphere if the point is outside the sphere:
            if (distSqr > math::Squared(circle.m_radius))
            {
                const Type dist = std::sqrt(distSqr);
                const Type newRadius = (circle.m_radius + dist) * static_cast<Type>(0.5f);
                const Type delta = (newRadius - circle.m_radius) / dist;

                circle.m_radius = newRadius;
                circle.m_center += toPoint * delta;
            }
        }

        //----------------------------------------------------------------------------------------------------
        //		NOTES:
        //      pg 90 of "Real-Time Collision Detection".
        //		
        ///		@brief : Given a sphere, and a point, grow the sphere (if needed) to encompass the point. 
        //----------------------------------------------------------------------------------------------------
        template <FloatingPointType Type>
        void GrowSphereToContainPoint3(TSphere3<Type>& sphere, const TVector3<Type>& point)
        {
            TVector3<Type> toPoint = point - sphere.m_center;
            Type distSqr = toPoint.SquaredMagnitude();

            // Only update the Sphere if the point is outside the sphere:
            if (distSqr > math::Squared(sphere.m_radius))
            {
                const Type dist = std::sqrt(distSqr);
                const Type newRadius = (sphere.m_radius + dist) * static_cast<Type>(0.5f);
                const Type delta = (newRadius - sphere.m_radius) / dist;

                sphere.m_radius = newRadius;
                sphere.m_center += toPoint * delta;
            }
        }

        //----------------------------------------------------------------------------------------------------
        ///		@brief : Creates an approximate circle to encompass the points in the array by defining an AABB
        ///             to encompass the points. This should be a first pass when devising a full bounding circle.
        //----------------------------------------------------------------------------------------------------
        template <FloatingPointType Type>
        void ApproximateSphereFromDistantPoints2(TSphere2<Type>& circle, const TVector2<Type>* points,
            const size_t count)
        {
            // Find the most separated point pair defining the AABB.
            size_t iMin = 0;
            size_t iMax = 0;
            MostSeparatedPointsOnAABB2(points, count, iMin, iMax);

            // Set up the circle to just encompass these two points:
            circle.m_center = (points[iMin] + points[iMax]) * static_cast<Type>(0.5f);
            circle.m_radius = (points[iMax] - circle.m_center).Magnitude();
        }

        //----------------------------------------------------------------------------------------------------
        ///		@brief : Creates an approximate sphere to encompass the points in the array by defining an AABB
        ///             to encompass the points. This should be a first pass when devising a full bounding sphere.
        //----------------------------------------------------------------------------------------------------
        template <FloatingPointType Type>
        void ApproximateSphereFromDistantPoints3(TSphere3<Type>& sphere, const TVector3<Type>* points,
            const size_t count)
        {
            // Find the most separated point pair defining the AABB.
            size_t iMin = 0;
            size_t iMax = 0;
            MostSeparatedPointsOnAABB2(points, count, iMin, iMax);

            // Set up the circle to just encompass these two points:
            sphere.m_center = (points[iMin] + points[iMax]) * static_cast<Type>(0.5f);
            sphere.m_radius = (points[iMax] - sphere.m_center).Magnitude();
        }

        //----------------------------------------------------------------------------------------------------
        //		NOTES:
        //      pg 89-91 of "Real-Time Collision Detection"
        //
        ///		@brief : Compute a bounding circle that encompasses all points in the array. This is accomplished
        ///             in two passes: first get an approximation that encompasses the two most distant points,
        ///             then grow the circle to encompass all points.
        //----------------------------------------------------------------------------------------------------
        template <FloatingPointType Type>
        void RitterBoundingSphere2(TSphere2<Type>& circle, const TVector2<Type>* points, const size_t count)
        {
            // Get an approximate sphere that encompasses two most distant points. 
            ApproximateSphereFromDistantPoints2(circle, points, count);

            // Grow the Sphere to include all points.
            for (size_t i = 0; i < count; ++i)
            {
                GrowSphereToContainPoint2(circle, points[i]);
            }
        }

        //----------------------------------------------------------------------------------------------------
        //		NOTES:
        //      pg 89-91 of "Real-Time Collision Detection"
        //
        ///		@brief : Compute a bounding sphere that encompasses all points in the array. This is accomplished
        ///             in two passes: first get an approximation that encompasses the two most distant points,
        ///             then grow the sphere to encompass all points.
        //----------------------------------------------------------------------------------------------------
        template <FloatingPointType Type>
        void RitterBoundingSphere3(TSphere3<Type>& sphere, const TVector3<Type>* points, const size_t count)
        {
            // Get an approximate sphere that encompasses two most distant points. 
            ApproximateSphereFromDistantPoints3(sphere, points, count);

            // Grow the Sphere to include all points.
            for (size_t i = 0; i < count; ++i)
            {
                GrowSphereToContainPoint3(sphere, points[i]);
            }
        }
    }

    template <FloatingPointType Type>
    constexpr TSphere2<Type>::TSphere2(const TVector2<Type>& center, Type radius)
        : m_center(center)
        , m_radius(radius)
    {
        //
    }

    //----------------------------------------------------------------------------------------------------
    ///		@brief : Constructs a bounding circle to encompass all the points in the array. 
    //----------------------------------------------------------------------------------------------------
    template <FloatingPointType Type>
    TSphere2<Type>::TSphere2(const TVector2<Type>* points, const size_t count)
    {
        math::RitterBoundingSphere2(*this, points, count);
    }

    //----------------------------------------------------------------------------------------------------
    ///		@brief : Returns true if this circle and another are intersecting. Two circles intersect if the
    //          distance between their centers is less than the sum of their radii.
    //----------------------------------------------------------------------------------------------------
    template <FloatingPointType Type>
    constexpr bool TSphere2<Type>::Intersects(const TSphere2& other) const
    {
        const Type sqrDist = (m_center - other.m_center).SquaredMagnitude();
        return sqrDist < math::Squared(m_radius + other.m_radius);
    }

    //----------------------------------------------------------------------------------------------------
    ///		@brief : Tests whether a point is inside or on the bounds of the circle.
    //----------------------------------------------------------------------------------------------------
    template <FloatingPointType Type>
    constexpr bool TSphere2<Type>::ContainsPoint(const TVector2<Type>& point) const
    {
        return (point - m_center).SquaredMagnitude() <= math::Squared(m_radius);
    }

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
    ///		@brief : Constructs a bounding sphere to encompass all the points in the array. 
    //----------------------------------------------------------------------------------------------------
    template <FloatingPointType Type>
    TSphere3<Type>::TSphere3(const TVector3<Type>* points, const size_t count)
    {
        math::RitterBoundingSphere3(*this, points, count);
    }

    template <FloatingPointType Type>
    void TSphere3<Type>::EncapsulatePoint(const TVector3<Type>& point)
    {
        math::GrowSphereToContainPoint3(*this, point);
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
    ///		@brief : Returns true if this sphere and another are intersecting. Two spheres intersect if the
    //          distance between their centers is less than the sum of their radii.
    //----------------------------------------------------------------------------------------------------
    template <FloatingPointType Type>
    constexpr bool TSphere3<Type>::Intersects(const TSphere3& other) const
    {
        const Type sqrDist = (m_center - other.m_center).SquaredMagnitude();
        return sqrDist < math::Squared(m_radius + other.m_radius);
    }

    //----------------------------------------------------------------------------------------------------
    ///		@brief : Tests whether a point lies within the Sphere or on its surface. 
    //----------------------------------------------------------------------------------------------------
    template <FloatingPointType Type>
    constexpr bool TSphere3<Type>::ContainsPoint(const TVector3<Type>& point) const
    {
        // [Consider]: Probably need a tolerance check here. 
        return (point - m_center).SquaredMagnitude() <= math::Squared(m_radius);
    }
}