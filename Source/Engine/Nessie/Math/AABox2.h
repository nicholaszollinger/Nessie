// AABox2.h
#pragma once
#include "Vector2.h"
#include "Detail/TMatrix3x3.h"

namespace nes
{
    namespace math
    {
        template <FloatingPointType Type>
        void ExtremePointsAlongDirection2(const TVector2<Type>& direction, const TVector2<Type>* points, const size_t count, size_t& iMin, size_t& iMax);

        template <FloatingPointType Type>
        void MostSeparatedPointsOnAABB2(const TVector2<Type>* points, const size_t count, size_t &iMin, size_t& iMax);
    }
    
    //----------------------------------------------------------------------------------------------------
    /// @brief : Represents an Axis-aligned Bounding Box (AABB) in 2 dimensions. The AABB is stored in
    ///     Min-Max form.
    //----------------------------------------------------------------------------------------------------
    template <FloatingPointType Type>
    struct TAABox2
    {
        /// Maximum Extent value. If larger than this, operations will be subject to overflow. 
        static constexpr Type kMaxExtent = static_cast<Type>(math::kLargeFloat * 0.5f);
        
        TVector2<Type> m_min;
        TVector2<Type> m_max;

        constexpr TAABox2();
        constexpr TAABox2(const TVector2<Type>& min, const TVector2<Type>& max) : m_min(min), m_max(max) {}
        constexpr TAABox2(const TVector2<Type>& center, const Type width, const Type height);
        constexpr TAABox2(const TVector2<Type>* points, const size_t count);
        
        constexpr bool operator==(const TAABox2& other) const;
        constexpr bool operator!=(const TAABox2& other) const { return !(*this == other); }
        
        constexpr TVector2<Type>    Center() const;
        constexpr TVector2<Type>    Extents() const;
        constexpr TVector2<Type>    Size() const;
        constexpr TVector2<Type>    ClosestPointToPoint(const TVector2<Type>& queryPoint) const;
        constexpr Type              Area() const;
        constexpr Type              SquaredDistanceToPoint(const TVector2<Type>& queryPoint) const;
        Type                        DistanceToPoint(const TVector2<Type>& queryPoint) const;
        constexpr bool              HasValidDimensions() const;
        constexpr bool              Intersects(const TAABox2& other) const;

        void                        GrowToEncapsulate(const TAABox2& box);
        void                        GrowToEncapsulate(const TVector2<Type>& point);
        
        /// Returns an invalid Axis Aligned Bounding Box. The Min and Max are set so that no intersection is possible.
        static constexpr TAABox2    Invalid() { return TAABox2(TVector2<Type>(static_cast<Type>(math::kLargeFloat), TVector2<Type>(static_cast<Type>(-math::kLargeFloat)))); }
        static constexpr TAABox2    FromCenterAndExtents(const TVector2<Type>& center, const TVector2<Type>& extents);
        static void                 Transform(const TAABox2& original, const TMatrix3x3<Type>& transform, TAABox2& result);

        std::string                 ToString() const;
    };

    using AABox2f = TAABox2<float>;
    using AABox2d = TAABox2<double>;
    using AABox2D = TAABox2<NES_PRECISION_TYPE>;
}

namespace nes
{
    //----------------------------------------------------------------------------------------------------
    //		NOTES:
    //		
    ///		@brief : Determines the indices (imin and imax) of the points array of least and most, respectively,
    ///             distant points along the "direction" vector. 
    ///		@param direction : Direction whose extreme vertices we are evaluating.
    ///		@param points : Array of points to evaluate.
    ///		@param count : Number of points in the array.
    ///		@param iMin : Resulting index of the minimum point along the direction.
    ///		@param iMax : Resulting index of the maximum point along the direction.
    //----------------------------------------------------------------------------------------------------
    template <FloatingPointType Type>
    void math::ExtremePointsAlongDirection2(const TVector2<Type>& direction, const TVector2<Type>* points,
        const size_t count, size_t& iMin, size_t& iMax)
    {
        Type minimumProj = std::numeric_limits<Type>::max();
        Type maximumProj = std::numeric_limits<Type>::min();

        for (size_t i = 0; i < count; ++i)
        {
            // Project vector from origin to the point onto the direction vector.
            Type projection = TVector2<Type>::Dot(points[i], direction);

            // Keep track of the least distance point along direction vector.
            if (projection < minimumProj)
            {
                minimumProj = projection;
                iMin = i;
            }

            // Keep track of the most distance point along the direction vector
            if (projection > maximumProj)
            {
                maximumProj = projection;
                iMax = i;
            }
        }
    }
    
    //----------------------------------------------------------------------------------------------------
    //		NOTES:
    //      pg 89 of "Real-Time Collision Detection".
    //		
    ///		@brief : Compute indices to the two most separated points of the (up to) four points defining
    ///             the AABB encompassing the point set. Results stored in iMin and iMax.
    ///		@param points : The point set that the AABB encompasses.
    ///		@param count : Number of points in the array.
    ///		@param iMin : Result index of the minimal point.
    ///		@param iMax : Result index of the maximal point.
    //----------------------------------------------------------------------------------------------------
    template <FloatingPointType Type>
    void math::MostSeparatedPointsOnAABB2(const TVector2<Type>* points, const size_t count, size_t& iMin, size_t& iMax)
    {
        // Find the indices of the minimum and maximum points of the AABB
        size_t minIndices[2] { 0, 0 };
        size_t maxIndices[2] { 0, 0 };

        for (size_t i = 0; i < count; ++i)
        {
            for (size_t axis = 0; axis < 2; ++axis)
            {
                if (points[minIndices[axis]][axis] > points[i][axis])
                    minIndices[axis] = i;

                if (points[maxIndices[axis]][axis] < points[i][axis])
                    maxIndices[axis] = i;
            }
        }

        // Compute the distances along the axes to find which one spans the largest distance:
        const Type sqrDistX = TVector2<Type>::DistanceSquared(points[minIndices[0]], points[maxIndices[0]]);
        const Type sqrDistY = TVector2<Type>::DistanceSquared(points[minIndices[1]], points[maxIndices[1]]);
        
        // X-Axis is the largest:
        if (sqrDistX > sqrDistY)
        {
            iMin = minIndices[0];
            iMax = maxIndices[0];
            return;
        }
        
        // Y-Axis is largest
        iMin = minIndices[1];
        iMax = maxIndices[1];
    }

    template <FloatingPointType Type>
    constexpr TAABox2<Type>::TAABox2()
        : m_min(TVector2<Type>(static_cast<Type>(-0.5f)))
        , m_max(TVector2<Type>(static_cast<Type>(0.5f)))
    {
        //
    }

    //----------------------------------------------------------------------------------------------------
    ///		@brief : Constructs a 2D AABB from a center position and width and height.  
    //----------------------------------------------------------------------------------------------------
    template <FloatingPointType Type>
    constexpr TAABox2<Type>::TAABox2(const TVector2<Type>& center, const Type width, const Type height)
    {
        const TVector2<Type> extents = TVector2<Type>(width, height) * static_cast<Type>(0.5f);
        m_min = center - extents;
        m_max = center + extents;
    }

    //----------------------------------------------------------------------------------------------------
    ///		@brief : Constructs a 2D AABB to contain an array of points.
    //----------------------------------------------------------------------------------------------------
    template <FloatingPointType Type>
    constexpr TAABox2<Type>::TAABox2(const TVector2<Type>* points, const size_t count)
    {
        NES_ASSERT(points != nullptr);
        m_min = TVector2<Type>(math::kLargeFloat);
        m_max = TVector2<Type>(-math::kLargeFloat);
        
        for (size_t i = 0; i < count; ++i)
        {
            m_min.x = math::Min(points[i].x, m_min.x);
            m_min.y = math::Min(points[i].y, m_min.y);

            m_max.x = math::Max(points[i].x, m_max.x);
            m_max.y = math::Max(points[i].y, m_max.y);
        }
    }

    template <FloatingPointType Type>
    constexpr bool TAABox2<Type>::operator==(const TAABox2& other) const
    {
        return m_min == other.m_min && m_max == other.m_max;
    }

    //----------------------------------------------------------------------------------------------------
    /// @brief : Get the center point of the bounding box.
    //----------------------------------------------------------------------------------------------------
    template <FloatingPointType Type>
    constexpr TVector2<Type> TAABox2<Type>::Center() const
    {
        return static_cast<Type>(0.5f) * (m_min + m_max);
    }
    
    //----------------------------------------------------------------------------------------------------
    /// @brief : Get the extents of the bounding box (half of the size).
    //----------------------------------------------------------------------------------------------------
    template <FloatingPointType Type>
    constexpr TVector2<Type> TAABox2<Type>::Extents() const
    {
        return static_cast<Type>(0.5f) * (m_max - m_min);
    }

    //----------------------------------------------------------------------------------------------------
    ///		@brief : Returns the size of each dimension of the Box. (x == width and y == height). 
    //----------------------------------------------------------------------------------------------------
    template <FloatingPointType Type>
    constexpr TVector2<Type> TAABox2<Type>::Size() const
    {
        return (m_max - m_min);
    }

    //----------------------------------------------------------------------------------------------------
    ///		@brief : Get the Area of the Box. 
    //----------------------------------------------------------------------------------------------------
    template <FloatingPointType Type>
    constexpr Type TAABox2<Type>::Area() const
    {
        const auto size = Size();
        return size.x * size.y;
    }

    //----------------------------------------------------------------------------------------------------
    ///		@brief : Returns whether each extent dimension is within the range (0, kMaxExtent).
    //----------------------------------------------------------------------------------------------------
    template <FloatingPointType Type>
    constexpr bool TAABox2<Type>::HasValidDimensions() const
    {
        return m_min <= m_max;
    }

    //----------------------------------------------------------------------------------------------------
    ///		@brief : Returns true if the two Boxes intersect. 
    //----------------------------------------------------------------------------------------------------
    template <FloatingPointType Type>
    constexpr bool TAABox2<Type>::Intersects(const TAABox2<Type>& other) const
    {
        if (m_min > other.m_max || m_max < other.m_min)
            return false;
        
        return true;
    }

    //----------------------------------------------------------------------------------------------------
    ///		@brief : Returns the closest point on or in the Box from the query point.
    //----------------------------------------------------------------------------------------------------
    template <FloatingPointType Type>
    constexpr TVector2<Type> TAABox2<Type>::ClosestPointToPoint(const TVector2<Type>& queryPoint) const
    {
        return TVector2<Type>::Min(TVector2<Type>::Max(queryPoint, m_min), m_max);
    }

    //----------------------------------------------------------------------------------------------------
    ///		@brief : Returns the distance from the query point to the closest point on the box. 
    //----------------------------------------------------------------------------------------------------
    template <FloatingPointType Type>
    Type TAABox2<Type>::DistanceToPoint(const TVector2<Type>& queryPoint) const
    {
        return std::sqrt(SquaredDistanceToPoint(queryPoint));
    }

    //----------------------------------------------------------------------------------------------------
    ///		@brief : Returns the squared distance from the query point to the closest point on the box. 
    //----------------------------------------------------------------------------------------------------
    template <FloatingPointType Type>
    constexpr Type TAABox2<Type>::SquaredDistanceToPoint(const TVector2<Type>& queryPoint) const
    {
        return (ClosestPointToPoint(queryPoint) - queryPoint).SquaredMagnitude();
    }

    //----------------------------------------------------------------------------------------------------
    /// @brief : Grow this Axis Aligned Box to contain the other Box, if necessary. 
    //----------------------------------------------------------------------------------------------------
    template <FloatingPointType Type>
    void TAABox2<Type>::GrowToEncapsulate(const TAABox2& box)
    {
        m_min = TVector2<Type>::Min(m_min, box.m_min);
        m_max = TVector2<Type>::Max(m_max, box.m_max);
    }

    //----------------------------------------------------------------------------------------------------
    /// @brief : Grow this Axis Aligned Box to contain the point, if necessary. 
    //----------------------------------------------------------------------------------------------------
    template <FloatingPointType Type>
    void TAABox2<Type>::GrowToEncapsulate(const TVector2<Type>& point)
    {
        m_min = TVector2<Type>::Min(m_min, point);
        m_max = TVector2<Type>::Max(m_max, point);
    }
    
    //----------------------------------------------------------------------------------------------------
    ///		@brief : Constructs a 2D AABB from a center and half-extents. 
    ///		@param center : Center position of the Box.
    ///		@param extents : Half the length of each dimension. Can be thought of as radii. 
    //----------------------------------------------------------------------------------------------------
    template <FloatingPointType Type>
    constexpr TAABox2<Type> TAABox2<Type>::FromCenterAndExtents(const TVector2<Type>& center, const TVector2<Type>& extents)
    {
        TAABox2<Type> result;
        result.m_min = center - extents;
        result.m_max = center + extents;
        return result;
    }

    //----------------------------------------------------------------------------------------------------
    //		NOTES:
    //      pg 87 of Real-Time Collision Detection.
    //		
    ///		@brief : Transform the "original" Box by the transform matrix (includes translation).  
    ///		@param original : Original Box to transform.
    ///		@param transform : Transform Matrix.
    ///		@param result : Result of the transformation.
    //----------------------------------------------------------------------------------------------------
    template <FloatingPointType Type>
    void TAABox2<Type>::Transform(const TAABox2& original, const TMatrix3x3<Type>& transform, TAABox2& result)
    {
        // For each dimension:
        for (int i = 0; i < 2; ++i)
        {
            // Set to the initial Translation:
            result.m_min[i] = transform.m[2][i];
            result.m_max[i] = transform.m[2][i];

            // Form extents by summing smaller and larger terms respectively:
            for (int j = 0; j < 2; ++j)
            {
                const TVector3<Type> column = transform.GetColumn(j);
                const Type e = transform.m[i][j] * original.m_min[j]; 
                const Type f = transform.m[i][j] * original.m_max[j];

                if (e < f)
                {
                    result.m_min[i] += e;
                    result.m_max[i] += f;
                }
                
                else
                {
                    result.m_min[i] += f;
                    result.m_max[i] += e;
                }
            }
        }
    }

    template <FloatingPointType Type>
    std::string TAABox2<Type>::ToString() const
    {
        return CombineIntoString("(center=", Center(), ", size=", Size(), ")");
    }
}
