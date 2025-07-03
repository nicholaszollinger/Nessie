// Sphere.h
#pragma once
#include "AABox.h"
#include "Nessie/Math/Math.h"

namespace nes
{
    //----------------------------------------------------------------------------------------------------
    /// @brief : A Sphere represented by a center point and radius. 
    //----------------------------------------------------------------------------------------------------
    class Sphere
    {
    public:
        inline constexpr    Sphere() = default;
        inline constexpr    Sphere(const Float3& center, const float radius) : m_center(center), m_radius(radius) {}
        inline              Sphere(const Vec3 center, const float radius)    : m_radius(radius) { center.StoreFloat3(&m_center); }
        inline              Sphere(const Vec3* points, const size_t count);
        inline              Sphere(const Float3* points, const size_t count);

        //----------------------------------------------------------------------------------------------------
        /// @brief : Get the center of the sphere. 
        //----------------------------------------------------------------------------------------------------
        inline Vec3         GetCenter() const                               { return Vec3::LoadFloat3Unsafe(m_center); }

        //----------------------------------------------------------------------------------------------------
        /// @brief : Set the center of the sphere. 
        //----------------------------------------------------------------------------------------------------
        void                SetCenter(const Vec3 center)                    { center.StoreFloat3(&m_center); }

        //----------------------------------------------------------------------------------------------------
        /// @brief : Get the radius of the Sphere. 
        //----------------------------------------------------------------------------------------------------
        inline float        GetRadius() const                               { return m_radius; }

        //----------------------------------------------------------------------------------------------------
        /// @brief : Set the radius of the sphere. 
        //----------------------------------------------------------------------------------------------------
        void                SetRadius(const float radius)                   { m_radius = radius; }

        //----------------------------------------------------------------------------------------------------
        /// @brief : Test if two spheres intersect.
        //----------------------------------------------------------------------------------------------------
        inline bool         Overlaps(const Sphere& other) const;

        //----------------------------------------------------------------------------------------------------
        /// @brief : Test if this sphere intersects a box.
        //----------------------------------------------------------------------------------------------------
        inline bool         Overlaps(const AABox& box) const;

        //----------------------------------------------------------------------------------------------------
        /// @brief : Grow the sphere (if necessary) to contain the point. 
        //----------------------------------------------------------------------------------------------------
        inline void         Encapsulate(const Vec3 point);

        //----------------------------------------------------------------------------------------------------
        /// @brief : Get the volume of the sphere. 
        //----------------------------------------------------------------------------------------------------
        inline float        Volume() const;

        //----------------------------------------------------------------------------------------------------
        /// @brief : Get the surface area of the sphere. 
        //----------------------------------------------------------------------------------------------------
        inline float        SurfaceArea() const;

    private:
        Float3              m_center; /// Center stored as 3 floats, rather than Vec3 (which is 4 floats). 
        float               m_radius;
    };

    // [TODO]: Move these, and have the definitions in a cpp file.
    namespace math
    {
        //----------------------------------------------------------------------------------------------------
        ///	@brief : Creates an approximate sphere to encompass the points in the array by defining an AABB
        ///     to encompass the points. This should be a first pass when devising a full bounding sphere.
        //----------------------------------------------------------------------------------------------------
        inline void         ApproximateSphereFromDistantPoints(Sphere& sphere, const Float3* points, const size_t count);

        //----------------------------------------------------------------------------------------------------
        ///	@brief : Creates an approximate sphere to encompass the points in the array by defining an AABB
        ///     to encompass the points. This should be a first pass when devising a full bounding sphere.
        //----------------------------------------------------------------------------------------------------
        inline void         ApproximateSphereFromDistantPoints(Sphere& sphere, const Vec3* points, const size_t count);

        //----------------------------------------------------------------------------------------------------
        ///	@brief : Compute a bounding sphere that encompasses all points in the array. This is done
        ///     in two passes: first get an approximation that encompasses the two most distant points,
        ///     then grow the sphere to encompass all points.
        //----------------------------------------------------------------------------------------------------
        inline void         RitterBoundingSphere(Sphere& sphere, const Vec3* points, const size_t count);

        //----------------------------------------------------------------------------------------------------
        ///	@brief : Compute a bounding sphere that encompasses all points in the array. This is done
        ///     in two passes: first get an approximation that encompasses the two most distant points,
        ///     then grow the sphere to encompass all points.
        //----------------------------------------------------------------------------------------------------
        inline void         RitterBoundingSphere(Sphere& sphere, const Float3* points, const size_t count);
    }
}

#include "Sphere.inl"