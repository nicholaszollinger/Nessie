// AABox.h
#pragma once
#include "Matrix.h"
#include "Vector3.h"

namespace nes
{
    namespace math
    {
        template <FloatingPointType Type>
        void ExtremePointsAlongDirection3(const TVector3<Type>& direction, const TVector3<Type>* points, const size_t count, size_t& iMin, size_t& iMax);

        template <FloatingPointType Type>
        void MostSeparatedPointsOnAABB3(const TVector2<Type>* points, const size_t count, size_t &iMin, size_t& iMax);
    }
    
    //----------------------------------------------------------------------------------------------------
    /// @brief : Represents an Axis-aligned Bounding Box (AABB) in 3 dimensions. The AABB is stored in
    ///     Min-Max form.
    //----------------------------------------------------------------------------------------------------
    template <FloatingPointType Type>
    struct TAABox3
    {
        /// Maximum Extent value. If larger than this, operations will be subject to overflow. 
        static constexpr Type kMaxExtent = static_cast<Type>(math::kLargeFloat * 0.5f);
        
        TVector3<Type> m_min;
        TVector3<Type> m_max;

        constexpr TAABox3();
        constexpr TAABox3(const TVector3<Type>& min, const TVector3<Type>& max) : m_min(min), m_max(max) {}
        constexpr TAABox3(const TVector3<Type>& center, const Type width, const Type height, const Type depth);
        constexpr TAABox3(const TVector3<Type>* points, const size_t count);
        
        constexpr bool operator==(const TAABox3& other) const;
        constexpr bool operator!=(const TAABox3& other) const { return !(*this == other); }
        
        constexpr TVector3<Type>    Center() const;
        constexpr TVector3<Type>    Extents() const;
        constexpr TVector3<Type>    Size() const;
        constexpr TVector3<Type>    ClosestPointToPoint(const TVector3<Type>& queryPoint) const;
        constexpr Type              Volume() const;
        constexpr Type              SquaredDistanceToPoint(const TVector3<Type>& queryPoint) const;
        Type                        DistanceToPoint(const TVector3<Type>& queryPoint) const;
        constexpr bool              HasValidDimensions() const;
        constexpr bool              Intersects(const TAABox3& other) const;
        constexpr bool              Contains(const TAABox3& other) const;
        constexpr bool              Contains(const Vector3& queryPoint) const;
        
        void                        GrowToEncapsulate(const TAABox3<Type>& box);
        void                        GrowToEncapsulate(const TVector3<Type>& point);

        /// Returns an invalid Axis Aligned Bounding Box. The Min and Max are set so that no intersection is possible.
        static constexpr TAABox3    Invalid() { return TAABox3(TVector3<Type>::Zero(), TVector3<Type>(static_cast<Type>(-math::kLargeFloat))); }
        static constexpr TAABox3    FromCenterAndExtents(const TVector3<Type>& center, const TVector3<Type>& extents);
        static void                 Transform(const TAABox3& original, const TMatrix4x4<Type>& transform, TAABox3& result);
        
        std::string                 ToString() const;
    };
    
    using AABox3f = TAABox3<float>;
    using AABox3d = TAABox3<double>;
    using AABox = TAABox3<NES_PRECISION_TYPE>;
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
    void math::ExtremePointsAlongDirection3(const TVector3<Type>& direction, const TVector3<Type>* points,
        const size_t count, size_t& iMin, size_t& iMax)
    {
        Type minimumProj = std::numeric_limits<Type>::max();
        Type maximumProj = std::numeric_limits<Type>::min();

        for (size_t i = 0; i < count; ++i)
        {
            // Project vector from origin to the point onto the direction vector.
            Type projection = TVector3<Type>::Dot(points[i], direction);

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
    ///		@brief : Compute indices to the two most separated points of the (up to) six points defining
    ///             the AABB encompassing the point set. Results stored in iMin and iMax.
    ///		@param points : The point set that the AABB encompasses.
    ///		@param count : Number of points in the array.
    ///		@param iMin : Result index of the minimal point.
    ///		@param iMax : Result index of the maximal point.
    //----------------------------------------------------------------------------------------------------
    template <FloatingPointType Type>
    void math::MostSeparatedPointsOnAABB3(const TVector2<Type>* points, const size_t count, size_t& iMin, size_t& iMax)
    {
        // Find the indices of the minimum and maximum points of the AABB
        size_t minIndices[3] { 0, 0, 0 };
        size_t maxIndices[3] { 0, 0, 0 };

        for (size_t i = 0; i < count; ++i)
        {
            for (size_t axis = 0; axis < 3; ++axis)
            {
                if (points[minIndices[axis]][axis] > points[i][axis])
                    minIndices[axis] = i;

                if (points[maxIndices[axis]][axis] < points[i][axis])
                    maxIndices[axis] = i;
            }
        }

        // Compute the distances along the axes to find which one spans the largest distance:
        const Type sqrDistX = TVector3<Type>::DistanceSquared(points[minIndices[0]], points[maxIndices[0]]);
        const Type sqrDistY = TVector3<Type>::DistanceSquared(points[minIndices[1]], points[maxIndices[1]]);
        const Type sqrDistZ = TVector3<Type>::DistanceSquared(points[minIndices[2]], points[maxIndices[2]]);

        // Assume X-Axis is largest
        iMin = minIndices[0];
        iMax = maxIndices[0];
        
        // Y-Axis is the largest:
        if (sqrDistY > sqrDistX && sqrDistY > sqrDistZ)
        {
            iMin = minIndices[1];
            iMax = maxIndices[1];
            return;
        }

        // Z-Axis is the largest:
        if (sqrDistZ > sqrDistX)
        {
            iMin = minIndices[2];
            iMax = maxIndices[2];
        }
    }

    template <FloatingPointType Type>
    constexpr TAABox3<Type>::TAABox3()
        : m_min(TVector3<Type>(static_cast<Type>(-0.5f)))
        , m_max(TVector3<Type>(static_cast<Type>(0.5f)))
    {
        //
    }

    //----------------------------------------------------------------------------------------------------
    ///		@brief : Constructs a 3D Axis Aligned Bounding Box from a center position and a width, height, and depth.  
    //----------------------------------------------------------------------------------------------------
    template <FloatingPointType Type>
    constexpr TAABox3<Type>::TAABox3(const TVector3<Type>& center, const Type width, const Type height, const Type depth)
    {
        const TVector3<Type> extents = TVector3<Type>(width, height, depth) * static_cast<Type>(0.5f);
        m_min = center - extents;
        m_max = center + extents;
    }

    //----------------------------------------------------------------------------------------------------
    ///		@brief : Constructs a 3D AABB from a series of points.  
    //----------------------------------------------------------------------------------------------------
    template <FloatingPointType Type>
    constexpr TAABox3<Type>::TAABox3(const TVector3<Type>* points, const size_t count)
    {
        m_min = TVector3<Type>(kMaxExtent);
        m_max = TVector3<Type>(-kMaxExtent);
        
        for (size_t i = 0; i < count; ++i)
        {
            m_min.x = math::Min(points[i].x, m_min.x);
            m_min.y = math::Min(points[i].y, m_min.y);
            m_min.z = math::Min(points[i].z, m_min.z);

            m_max.x = math::Max(points[i].x, m_max.x);
            m_max.y = math::Max(points[i].y, m_max.y);
            m_max.z = math::Max(points[i].z, m_max.z);
        }
    }

    template <FloatingPointType Type>
    constexpr bool TAABox3<Type>::operator==(const TAABox3& other) const
    {
        return m_min == other.m_min && m_max == other.m_max;
    }

    //----------------------------------------------------------------------------------------------------
    /// @brief : Get the center point of the bounding box.
    //----------------------------------------------------------------------------------------------------
    template <FloatingPointType Type>
    constexpr TVector3<Type> TAABox3<Type>::Center() const
    {
        return static_cast<Type>(0.5f) * (m_min + m_max);
    }

    //----------------------------------------------------------------------------------------------------
    /// @brief : Get the extents of the bounding box (half of the size). 
    //----------------------------------------------------------------------------------------------------
    template <FloatingPointType Type>
    constexpr TVector3<Type> TAABox3<Type>::Extents() const
    {
        return static_cast<Type>(0.5f) * (m_max - m_min);
    }
    //----------------------------------------------------------------------------------------------------
    ///		@brief : Returns the size of each dimension of the Box. (x == width, y == height, z == depth). 
    //----------------------------------------------------------------------------------------------------
    template <FloatingPointType Type>
    constexpr TVector3<Type> TAABox3<Type>::Size() const
    {
        return (m_max - m_min);
    }

    //----------------------------------------------------------------------------------------------------
    ///		@brief : Returns the Volume of the Box.
    //----------------------------------------------------------------------------------------------------
    template <FloatingPointType Type>
    constexpr Type TAABox3<Type>::Volume() const
    {
        const auto size = Size();
        return size.x * size.y * size.z;
    }

    //----------------------------------------------------------------------------------------------------
    ///		@brief : Returns whether each extent dimension is within the range (0, kMaxExtent).
    //----------------------------------------------------------------------------------------------------
    template <FloatingPointType Type>
    constexpr bool TAABox3<Type>::HasValidDimensions() const
    {
        return m_min <= m_max;
    }
    
    //----------------------------------------------------------------------------------------------------
    ///		@brief : Returns true if the two Boxes intersect. 
    //----------------------------------------------------------------------------------------------------
    template <FloatingPointType Type>
    constexpr bool TAABox3<Type>::Intersects(const TAABox3<Type>& other) const
    {
        if (m_min > other.m_max || m_max < other.m_min)
            return false;
        
        return true;
    }

    //----------------------------------------------------------------------------------------------------
    /// @brief : Returns true if the Box fully envelops the other.
    //----------------------------------------------------------------------------------------------------
    template <FloatingPointType Type>
    constexpr bool TAABox3<Type>::Contains(const TAABox3& other) const
    {
        return m_min <= other.m_min && m_max >= other.m_max;
    }

    //----------------------------------------------------------------------------------------------------
    /// @brief : Returns true if the point is inside the Box.
    //----------------------------------------------------------------------------------------------------
    template <FloatingPointType Type>
    constexpr bool TAABox3<Type>::Contains(const Vector3& queryPoint) const
    {
        return m_min <= queryPoint && m_max >= queryPoint;
    }

    //----------------------------------------------------------------------------------------------------
    ///		@brief : Returns the closest point on or in the Box from the query point.
    //----------------------------------------------------------------------------------------------------
    template <FloatingPointType Type>
    constexpr TVector3<Type> TAABox3<Type>::ClosestPointToPoint(const TVector3<Type>& queryPoint) const
    {
        return TVector3<Type>::Min(TVector3<Type>::Max(queryPoint, m_min), m_max);
    }

    //----------------------------------------------------------------------------------------------------
    ///		@brief : Returns the distance from the query point to the closest point on the box. 
    //----------------------------------------------------------------------------------------------------
    template <FloatingPointType Type>
    Type TAABox3<Type>::DistanceToPoint(const TVector3<Type>& queryPoint) const
    {
        return std::sqrt(SquaredDistanceToPoint(queryPoint));
    }

    //----------------------------------------------------------------------------------------------------
    ///		@brief : Returns the squared distance from the query point to the closest point on the box. 
    //----------------------------------------------------------------------------------------------------
    template <FloatingPointType Type>
    constexpr Type TAABox3<Type>::SquaredDistanceToPoint(const TVector3<Type>& queryPoint) const
    {
        return (ClosestPointToPoint(queryPoint) - queryPoint).SquaredMagnitude();
    }

    //----------------------------------------------------------------------------------------------------
    /// @brief : Construct an Axis Aligned Box from a center point and extents (half the size of each axis).
    //----------------------------------------------------------------------------------------------------
    template <FloatingPointType Type>
    constexpr TAABox3<Type> TAABox3<Type>::FromCenterAndExtents(const TVector3<Type>& center, const TVector3<Type>& extents)
    {
        TAABox3<Type> result;
        result.m_min = center - extents;
        result.m_max = center + extents;
        return result;
    }

    //----------------------------------------------------------------------------------------------------
    /// @brief : Increase the size of this AABox to fully contain the other box, if necessary.
    //----------------------------------------------------------------------------------------------------
    template <FloatingPointType Type>
    void TAABox3<Type>::GrowToEncapsulate(const TAABox3<Type>& box)
    {
        m_min = TVector3<Type>::Min(m_min, box.m_min);
        m_max = TVector3<Type>::Max(m_max, box.m_max);
    }

    //----------------------------------------------------------------------------------------------------
    /// @brief : Increase the size of this AABox to fully contain the point, if necessary. 
    //----------------------------------------------------------------------------------------------------
    template <FloatingPointType Type>
    void TAABox3<Type>::GrowToEncapsulate(const TVector3<Type>& point)
    {
        m_min = TVector3<Type>::Min(m_min, point);
        m_max = TVector3<Type>::Max(m_max, point);
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
    void TAABox3<Type>::Transform(const TAABox3& original, const TMatrix4x4<Type>& transform, TAABox3& result)
    {
        // For each dimension:
        for (int i = 0; i < 3; ++i)
        {
            // Set to the initial Translation:
            result.m_min[i] = transform.m[3][i];
            result.m_max[i] = transform.m[3][i];

            // Form extents by summing smaller and larger terms respectively:
            for (int j = 0; j < 3; ++j)
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
    std::string TAABox3<Type>::ToString() const
    {
        return CombineIntoString("(center=", Center(), ", size=", Size(), ")");
    }
}