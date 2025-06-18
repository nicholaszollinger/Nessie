// AABox2.h
#pragma once
#include "Math/Vec2.h"
#include "Math/Mat4.h"

namespace nes
{
    //----------------------------------------------------------------------------------------------------
    /// @brief : Represents an Axis-aligned Bounding Box (AABB) in 2 dimensions. The AABB is stored in
    ///     Min-Max form.
    //----------------------------------------------------------------------------------------------------
    struct AABox2
    {
        /// Maximum Extent value. If larger than this, operations will be subject to overflow. 
        static constexpr float      kMaxExtent = math::kLargeFloat * 0.5f;
        Vec2                        m_min; /// Minimum point of the box.
        Vec2                        m_max; /// Maximum point of the box.

        /// Constructors
                                    AABox2()                                        : m_min(Vec2(FLT_MAX)), m_max(Vec2(-FLT_MAX)) {}
                                    AABox2(const Vec2& min, const Vec2& max)        : m_min(min), m_max(max) {}
                                    AABox2(const Vec2& center, const float radius)  : m_min(center - Vec2(radius)), m_max(center + Vec2(radius)) {}

        /// Operators
        bool                        operator==(const AABox2& other) const       { return m_min == other.m_min && m_max == other.m_max; }
        bool                        operator!=(const AABox2& other) const       { return m_min != other.m_min || m_max != other.m_max; } 

        //----------------------------------------------------------------------------------------------------
        /// @brief : Returns true if the min point is less than the max point. 
        //----------------------------------------------------------------------------------------------------
        NES_INLINE bool             IsValid() const                             { return m_min <= m_max;  } 

        //----------------------------------------------------------------------------------------------------
        /// @brief : Reset the bounding box to the empty state, where the Max is set to -FLT_MAX, and the Min
        ///     is set to FLT_MAX, making any intersection with the box impossible.
        //----------------------------------------------------------------------------------------------------
        NES_INLINE void             Reset();
        
        //----------------------------------------------------------------------------------------------------
        /// @brief : Grow the Box (if necessary) so that it contains the given point. 
        //----------------------------------------------------------------------------------------------------
        NES_INLINE void             Encapsulate(const Vec2& point);

        //----------------------------------------------------------------------------------------------------
        /// @brief : Grow the Box (if necessary) so that it contains the given box.
        //----------------------------------------------------------------------------------------------------
        NES_INLINE void             Encapsulate(const AABox2& box);

        //----------------------------------------------------------------------------------------------------
        /// @brief : Intersect this bounding box with the other. Returns the intersection. 
        //----------------------------------------------------------------------------------------------------
        NES_INLINE AABox2           Intersect(const AABox2& other) const;

        //----------------------------------------------------------------------------------------------------
        /// @brief : Make sure that each edge of the bounding box is at least minEdgeLength long. 
        //----------------------------------------------------------------------------------------------------
        NES_INLINE void             EnsureMinimalEdgeLength(const float minEdgeLength);

        //----------------------------------------------------------------------------------------------------
        /// @brief : Widen the box on both sides by the given distance.
        //----------------------------------------------------------------------------------------------------
        NES_INLINE void             ExpandBy(const Vec2 distance);

        //----------------------------------------------------------------------------------------------------
        /// @brief : Get the center of the box. 
        //----------------------------------------------------------------------------------------------------
        NES_INLINE Vec2             Center() const                              { return (m_min + m_max) * 0.5f; }

        //----------------------------------------------------------------------------------------------------
        /// @brief : Get the extent of the box (half of the size). 
        //----------------------------------------------------------------------------------------------------
        NES_INLINE Vec2             Extent() const                              { return (m_max - m_min) * 0.5f; }

        //----------------------------------------------------------------------------------------------------
        /// @brief : Get the size of the box (x == width, y == height). 
        //----------------------------------------------------------------------------------------------------
        NES_INLINE Vec2             Size() const                                { return (m_max - m_min); }
        
        //----------------------------------------------------------------------------------------------------
        /// @brief : Get the area of the bounding box.
        //----------------------------------------------------------------------------------------------------
        NES_INLINE float            Area() const;

        //----------------------------------------------------------------------------------------------------
        /// @brief : Returns true if the other box is inside this box.
        //----------------------------------------------------------------------------------------------------
        NES_INLINE bool             Contains(const AABox2& other) const;

        //----------------------------------------------------------------------------------------------------
        /// @brief : Returns true if the point is inside the box.
        //----------------------------------------------------------------------------------------------------
        NES_INLINE bool             Contains(const Vec2 point) const;

        //----------------------------------------------------------------------------------------------------
        /// @brief : Returns true if the two boxes intersect. 
        //----------------------------------------------------------------------------------------------------
        NES_INLINE bool             Overlaps(const AABox2& other) const;

        //----------------------------------------------------------------------------------------------------
        /// @brief : Translate the bounding box by the given translation. 
        //----------------------------------------------------------------------------------------------------
        NES_INLINE void             Translate(const Vec2 translation);

        //----------------------------------------------------------------------------------------------------
        /// @brief : Transform the bounding box by the given matrix.
        //----------------------------------------------------------------------------------------------------
        NES_INLINE AABox2           Transformed(const Mat44& matrix) const;

        //----------------------------------------------------------------------------------------------------
        /// @brief :Scale this bounding box. This can handle non-uniform and negative scaling. 
        //----------------------------------------------------------------------------------------------------
        NES_INLINE AABox2           Scaled(const Vec2 scale) const;

        //----------------------------------------------------------------------------------------------------
        /// @brief : Get the closest point on or inside this box to the given point.
        //----------------------------------------------------------------------------------------------------
        NES_INLINE Vec2             GetClosestPoint(const Vec2 queryPoint) const;

        //----------------------------------------------------------------------------------------------------
        /// @brief : Get the distance between the point and this box. This will return 0 if the point
        ///     is inside this box.
        //----------------------------------------------------------------------------------------------------
        NES_INLINE float            DistanceTo(const Vec2 queryPoint) const { return std::sqrt(DistanceSqrTo(queryPoint)); }

        //----------------------------------------------------------------------------------------------------
        /// @brief : Get the squared distance between the point and this box. This will return 0 if the point
        ///     is inside this box.
        //----------------------------------------------------------------------------------------------------
        NES_INLINE float            DistanceSqrTo(const Vec2 queryPoint) const;

        //----------------------------------------------------------------------------------------------------
        /// @brief : Construct a Box from 2 points.
        //----------------------------------------------------------------------------------------------------
        static NES_INLINE AABox2    FromTwoPoints(const Vec2 a, const Vec2 b)   { return AABox2(Vec2::Min(a, b), Vec2::Max(a, b)); }

        //----------------------------------------------------------------------------------------------------
        /// @brief : Returns an invalid AABox. The Min and Max are set so that no intersection is possible. 
        //----------------------------------------------------------------------------------------------------
        static NES_INLINE AABox2    Invalid()                                   { return AABox2(Vec2(math::kLargeFloat), Vec2(-math::kLargeFloat)); }

        //----------------------------------------------------------------------------------------------------
        /// @brief : Get a bounding box of size FLT_MAX. 
        //----------------------------------------------------------------------------------------------------
        static NES_INLINE AABox2    Biggest()                                   { return AABox2(Vec2(-0.5f * FLT_MAX), Vec2(0.5f * FLT_MAX)); }        
    };

    namespace math
    {
        //----------------------------------------------------------------------------------------------------
        /// @brief : Compute indices to the two most separated points of the (up to) 4 points defining
        ///    the AABB encompassing the point set. Results stored in iMin and iMax.
        /// @param points : The point set that the AABB encompasses.
        /// @param count : Number of points in the array.
        /// @param iMin : Result index of the minimal point.
        /// @param iMax : Result index of the maximal point.
        //----------------------------------------------------------------------------------------------------
        void MostSeparatedPointsOnAABB(const Vec2* points, const size_t count, size_t& iMin, size_t& iMax);

        //----------------------------------------------------------------------------------------------------
        /// @brief : Compute indices to the two most separated points of the (up to) 4 points defining
        ///    the AABB encompassing the point set. Results stored in iMin and iMax.
        /// @param points : The point set that the AABB encompasses.
        /// @param count : Number of points in the array.
        /// @param iMin : Result index of the minimal point.
        /// @param iMax : Result index of the maximal point.
        //----------------------------------------------------------------------------------------------------
        void MostSeparatedPointsOnAABB(const Float2* points, const size_t count, size_t& iMin, size_t& iMax);
    }
}

namespace nes
{
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
    // void math::ExtremePointsAlongDirection2(const Vec2& direction, const Vec2* points,
    //     const size_t count, size_t& iMin, size_t& iMax)
    // {
    //     float minimumProj = std::numeric_limits<Type>::max;
    //     float maximumProj = std::numeric_limits<Type>::min();
    //
    //     for (size_t i = 0; i < count; ++i)
    //     {
    //         // Project vector from origin to the point onto the direction vector.
    //         float projection = Vec2::Dot(points[i], direction);
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

    // //----------------------------------------------------------------------------------------------------
    // ///		@brief : Constructs a 2D AABB to contain an array of points.
    // //----------------------------------------------------------------------------------------------------
    // template <FloatingPointType Type>
    // constexpr AABox2::AABox2(const Vec2* points, const size_t count)
    // {
    //     NES_ASSERT(points != nullptr);
    //     m_min = Vec2(math::kLargeFloat);
    //     m_max = Vec2(-math::kLargeFloat);
    //     
    //     for (size_t i = 0; i < count; ++i)
    //     {
    //         m_min.x = math::Min(points[i].x, m_min.x);
    //         m_min.y = math::Min(points[i].y, m_min.y);
    //
    //         m_max.x = math::Max(points[i].x, m_max.x);
    //         m_max.y = math::Max(points[i].y, m_max.y);
    //     }
    // }
}

#include "AABox2.inl"