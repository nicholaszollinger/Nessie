// AABox.h
#pragma once
#include "Plane.h"
#include "Math/Vec4.h"
#include "Math/Mat44.h"

namespace nes
{
    // [TODO]: Encapsulate a triangle.
    // [TODO]: Encapsulate a vertex list.
    // [TODO]: Double vec/matrix support.
    
    //----------------------------------------------------------------------------------------------------
    /// @brief : Represents an Axis-aligned Bounding Box (AABB) in 3 dimensions. The AABB is stored in
    ///     Min-Max form.
    //----------------------------------------------------------------------------------------------------
    struct AABox
    {
        Vec3 m_min; /// Minimum point of the box.
        Vec3 m_max; /// Maximum point of the box.

        /// Constructors
        AABox()                                         : m_min(Vec3::Replicate(FLT_MAX)), m_max(Vec3::Replicate(-FLT_MAX)) {}
        AABox(const Vec3 min, const Vec3 max)           : m_min(min), m_max(max) {}
        AABox(const Vec3 center, const float radius)    : m_min(center - Vec3::Replicate(radius)), m_max(center + Vec3::Replicate(radius)) {}

        /// Operators
        bool                    operator==(const AABox& other) const            { return m_min == other.m_min && m_max == other.m_max; }
        bool                    operator!=(const AABox& other) const            { return m_min != other.m_min || m_max != other.m_max; }

        //----------------------------------------------------------------------------------------------------
        /// @brief : Returns true if the min point is less than the max point. 
        //----------------------------------------------------------------------------------------------------
        NES_INLINE bool         IsValid() const                                 { return m_min <= m_max; }
        
        //----------------------------------------------------------------------------------------------------
        /// @brief : Reset the bounding box to the empty state, where the Max is set to -FLT_MAX and the Min
        ///     is set to FLT_MAX, making any intersection with the box impossible.
        //----------------------------------------------------------------------------------------------------
        NES_INLINE void         Reset();

        //----------------------------------------------------------------------------------------------------
        /// @brief : Grow the Box (if necessary) so that it contains the given point. 
        //----------------------------------------------------------------------------------------------------
        NES_INLINE void         Encapsulate(const Vec3 position);

        //----------------------------------------------------------------------------------------------------
        /// @brief : Grow the Box (if necessary) so that it contains the given box.
        //----------------------------------------------------------------------------------------------------
        NES_INLINE void         Encapsulate(const AABox& box);

        //----------------------------------------------------------------------------------------------------
        /// @brief : Intersect this bounding box with the other. Returns the intersection. 
        //----------------------------------------------------------------------------------------------------
        NES_INLINE AABox        Intersect(const AABox& other) const;

        //----------------------------------------------------------------------------------------------------
        /// @brief : Make sure that each edge of the bounding box is at least minEdgeLength long. 
        //----------------------------------------------------------------------------------------------------
        NES_INLINE void         EnsureMinimalEdgeLength(const float minEdgeLength);

        //----------------------------------------------------------------------------------------------------
        /// @brief : Widen the box on both sides by the given distance.
        //----------------------------------------------------------------------------------------------------
        NES_INLINE void         ExpandBy(const Vec3 distance);

        //----------------------------------------------------------------------------------------------------
        /// @brief : Get the center of the box. 
        //----------------------------------------------------------------------------------------------------
        NES_INLINE Vec3         Center() const                                  { return (m_min + m_max) * 0.5f; }

        //----------------------------------------------------------------------------------------------------
        /// @brief : Get the extent of the box (half of the size). 
        //----------------------------------------------------------------------------------------------------
        NES_INLINE Vec3         Extent() const                                  { return (m_max - m_min) * 0.5f; }

        //----------------------------------------------------------------------------------------------------
        /// @brief : Get the size of the box (x == width, y == height, z == depth). 
        //----------------------------------------------------------------------------------------------------
        NES_INLINE Vec3         Size() const                                    { return m_max - m_min; }

        //----------------------------------------------------------------------------------------------------
        /// @brief : Get the surface area of the bounding box.
        //----------------------------------------------------------------------------------------------------
        NES_INLINE float        SurfaceArea() const;

        //----------------------------------------------------------------------------------------------------
        /// @brief : Get the volume of the bounding box. 
        //----------------------------------------------------------------------------------------------------
        NES_INLINE float        Volume() const;

        //----------------------------------------------------------------------------------------------------
        /// @brief : Returns true if the other box is inside this box.
        //----------------------------------------------------------------------------------------------------
        NES_INLINE bool         Contains(const AABox& other) const;

        //----------------------------------------------------------------------------------------------------
        /// @brief : Returns true if the point is inside the box.
        //----------------------------------------------------------------------------------------------------
        NES_INLINE bool         Contains(const Vec3 point) const;

        //----------------------------------------------------------------------------------------------------
        /// @brief : Returns true if the two boxes intersect. 
        //----------------------------------------------------------------------------------------------------
        NES_INLINE bool         Overlaps(const AABox& other) const;

        //----------------------------------------------------------------------------------------------------
        /// @brief : Returns true if the box and plane intersect. 
        //----------------------------------------------------------------------------------------------------
        NES_INLINE bool         Overlaps(const Plane& plane) const;

        //----------------------------------------------------------------------------------------------------
        /// @brief : Translate the bounding box by the given translation. 
        //----------------------------------------------------------------------------------------------------
        NES_INLINE void         Translate(const Vec3 translation);

        //----------------------------------------------------------------------------------------------------
        /// @brief : Transform the bounding box by the given matrix.
        //----------------------------------------------------------------------------------------------------
        NES_INLINE AABox        Transformed(const Mat44& matrix) const;

        //----------------------------------------------------------------------------------------------------
        /// @brief :Scale this bounding box. This can handle non-uniform and negative scaling. 
        //----------------------------------------------------------------------------------------------------
        NES_INLINE AABox        Scaled(const Vec3 scale) const;

        //----------------------------------------------------------------------------------------------------
        /// @brief : Get the closest point on or inside this box to the given point.
        //----------------------------------------------------------------------------------------------------
        NES_INLINE Vec3         ClosestPointTo(const Vec3 point) const;

        //----------------------------------------------------------------------------------------------------
        /// @brief : Get the squared distance between the point and this box. This will return 0 if the point
        ///     is inside this box.
        //----------------------------------------------------------------------------------------------------
        NES_INLINE float        GetSqrDistanceTo(const Vec3 point) const;

        //----------------------------------------------------------------------------------------------------
        /// @brief : Calculate the support vector for this convex shape.
        //----------------------------------------------------------------------------------------------------
        NES_INLINE Vec3         GetSupport(const Vec3 direction) const;

        //----------------------------------------------------------------------------------------------------
        /// @brief : Get the vertices of the face that faces the direction the most.
        //----------------------------------------------------------------------------------------------------
        template <typename VertexArray>
        void                    GetSupportingFace(const Vec3& direction, VertexArray& outVertices) const;
        
        //----------------------------------------------------------------------------------------------------
        /// @brief : Construct a Box from 2 points.
        //----------------------------------------------------------------------------------------------------
        static NES_INLINE AABox FromTwoPoints(const Vec3 point1, const Vec3 point2)     { return AABox(Vec3::Min(point1, point2), Vec3::Max(point1, point2)); }
        
        //----------------------------------------------------------------------------------------------------
        /// @brief : Get a bounding box of size FLT_MAX. 
        //----------------------------------------------------------------------------------------------------
        static NES_INLINE AABox Biggest()                                               { return AABox(Vec3::Replicate(-0.5f * FLT_MAX), Vec3::Replicate(0.5f * FLT_MAX)); }

        //----------------------------------------------------------------------------------------------------
        /// @brief : Returns an invalid AABox. The Min and Max are set so that no intersection is possible. 
        //----------------------------------------------------------------------------------------------------
        static NES_INLINE AABox Invalid()                                               { return AABox(Vec3::Replicate(FLT_MAX), Vec3::Replicate(-FLT_MAX)); }
    };

    namespace math
    {
        //----------------------------------------------------------------------------------------------------
        /// @brief : Compute indices to the two most separated points of the (up to) six points defining
        ///    the AABB encompassing the point set. Results stored in iMin and iMax.
        /// @param points : The point set that the AABB encompasses.
        /// @param count : Number of points in the array.
        /// @param iMin : Result index of the minimal point.
        /// @param iMax : Result index of the maximal point.
        //----------------------------------------------------------------------------------------------------
        inline void             MostSeparatedPointsOnAABB(const Vec3* points, const size_t count, size_t& iMin, size_t& iMax);

        //----------------------------------------------------------------------------------------------------
        /// @brief : Compute indices to the two most separated points of the (up to) six points defining
        ///    the AABB encompassing the point set. Results stored in iMin and iMax.
        /// @param points : The point set that the AABB encompasses.
        /// @param count : Number of points in the array.
        /// @param iMin : Result index of the minimal point.
        /// @param iMax : Result index of the maximal point.
        //----------------------------------------------------------------------------------------------------
        inline void             MostSeparatedPointsOnAABB(const Float3* points, const size_t count, size_t& iMin, size_t& iMax);
    }
    
}

#include "AABox.inl"

// [TODO]: 
// //----------------------------------------------------------------------------------------------------
// //		NOTES:
// //		
// ///		@brief : Determines the indices (imin and imax) of the points array of least and most, respectively,
// ///             distant points along the "direction" vector. 
// ///		@param direction : Direction whose extreme vertices we are evaluating.
// ///		@param points : Array of points to evaluate.
// ///		@param count : Number of points in the array.
// ///		@param iMin : Resulting index of the minimum point along the direction.
// ///		@param iMax : Resulting index of the maximum point along the direction.
// //----------------------------------------------------------------------------------------------------
// template <FloatingPointType Type>
// void math::ExtremePointsAlongDirection3(const TVector3<Type>& direction, const TVector3<Type>* points,
//     const size_t count, size_t& iMin, size_t& iMax)
// {
//     Type minimumProj = std::numeric_limits<Type>::max();
//     Type maximumProj = std::numeric_limits<Type>::min();
//
//     for (size_t i = 0; i < count; ++i)
//     {
//         // Project vector from origin to the point onto the direction vector.
//         Type projection = TVector3<Type>::Dot(points[i], direction);
//
//         // Keep track of the least distance point along direction vector.
//         if (projection < minimumProj)
//         {
//             minimumProj = projection;
//             iMin = i;
//         }
//
//         // Keep track of the most distance point along the direction vector
//         if (projection > maximumProj)
//         {
//             maximumProj = projection;
//             iMax = i;
//         }
//     }
// }