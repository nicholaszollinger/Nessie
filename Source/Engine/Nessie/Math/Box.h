﻿// Box.h
#pragma once
#include "Matrix.h"
#include "Vector3.h"

namespace nes
{
    namespace math
    {
        template <FloatingPointType Type>
        void ExtremePointsAlongDirection2(const TVector2<Type>& direction, const TVector2<Type>* points, const size_t count, size_t& iMin, size_t& iMax);

        template <FloatingPointType Type>
        void ExtremePointsAlongDirection3(const TVector3<Type>& direction, const TVector3<Type>* points, const size_t count, size_t& iMin, size_t& iMax);

        template <FloatingPointType Type>
        void MostSeparatedPointsOnAABB2(const TVector2<Type>* points, const size_t count, size_t &iMin, size_t& iMax);

        template <FloatingPointType Type>
        void MostSeparatedPointsOnAABB3(const TVector2<Type>* points, const size_t count, size_t &iMin, size_t& iMax);
    }
    
    //----------------------------------------------------------------------------------------------------
    //		NOTES:
    //      Unreal Engine stores their Box2D as a min and max point.
    //		
    ///		@brief : Represents an Axis-aligned Bounding Box (AABB) in 2 dimensions. The AABB is stored in
    ///             Center-Radius form.
    //----------------------------------------------------------------------------------------------------
    template <FloatingPointType Type>
    struct TBox2
    {
        TVector2<Type> m_center{};        // Center point of the AABB.
        TVector2<Type> m_halfExtents{};   // Radius or half-width extents.

        constexpr TBox2() = default;
        constexpr TBox2(const TVector2<Type>& center, const TVector2<Type>& halfExtents);
        constexpr TBox2(const TVector2<Type>& center, const Type width, const Type height);
        constexpr TBox2(const TVector2<Type>* points, const size_t count);
        
        constexpr bool operator==(const TBox2& other) const;
        constexpr bool operator!=(const TBox2& other) const { return !(*this == other); }
        
        constexpr TVector2<Type> Min() const;
        constexpr TVector2<Type> Max() const;
        constexpr TVector2<Type> Size() const;
        constexpr Type Area() const;

        constexpr bool HasValidDimensions() const;
        constexpr bool Intersects(const TBox2& other) const;
        constexpr TVector2<Type> ClosestPointToPoint(const TVector2<Type>& queryPoint) const;
        Type DistanceToPoint(const TVector2<Type>& queryPoint) const;
        constexpr Type SquaredDistanceToPoint(const TVector2<Type>& queryPoint) const;
        
        static void Transform(const TBox2& original, const TMatrix3x3<Type>& transform, TBox2& result);

        std::string ToString() const;
    };

    //----------------------------------------------------------------------------------------------------
    //		NOTES:
    //      Unreal Engine stores their Box as a min and max point. As per my book, center-radius form
    //      seems like the better storage approach.
    //		
    ///		@brief : Represents an Axis-aligned Bounding Box (AABB) in 3 dimensions. The AABB is stored in
    ///             Center-Radius form.
    //----------------------------------------------------------------------------------------------------
    template <FloatingPointType Type>
    struct TBox3
    {
        TVector3<Type> m_center{};        // Center point of the AABB.
        TVector3<Type> m_halfExtents{};   // Radius or half-width extents.

        constexpr TBox3() = default;
        constexpr TBox3(const TVector3<Type>& center, const TVector3<Type>& halfExtents);
        constexpr TBox3(const TVector3<Type>* points, const size_t count);
        
        constexpr bool operator==(const TBox3& other) const;
        constexpr bool operator!=(const TBox3& other) const { return !(*this == other); }
        
        constexpr TVector3<Type> Min() const;
        constexpr TVector3<Type> Max() const;
        constexpr TVector3<Type> Size() const;
        constexpr Type Volume() const;

        constexpr bool HasValidDimensions() const;
        constexpr bool Intersects(const TBox3<Type>& other) const;
        constexpr TVector3<Type> ClosestPointToPoint(const TVector3<Type>& queryPoint) const;
        Type DistanceToPoint(const TVector3<Type>& queryPoint) const;
        constexpr Type SquaredDistanceToPoint(const TVector3<Type>& queryPoint) const;

        static void Transform(const TBox3& original, const TMatrix4x4<Type>& transform, TBox3& result);

        std::string ToString() const;
    };
    
    using Box2f = TBox2<float>;
    using Box2d = TBox2<double>;
    using Box2D = TBox2<NES_MATH_DEFAULT_REAL_TYPE>;
    
    using Box3f = TBox3<float>;
    using Box3d = TBox3<double>;
    using Box = TBox3<NES_MATH_DEFAULT_REAL_TYPE>;
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

    //----------------------------------------------------------------------------------------------------
    ///		@brief : Constructs a 2D AABB from a center and half-extents. 
    ///		@param center : Center position of the Box.
    ///		@param halfExtents : Half the length of each dimension. Can be thought of as radii. 
    //----------------------------------------------------------------------------------------------------
    template <FloatingPointType Type>
    constexpr TBox2<Type>::TBox2(const TVector2<Type>& center, const TVector2<Type>& halfExtents)
        : m_center(center)
        , m_halfExtents(halfExtents)
    {
        //
    }

    //----------------------------------------------------------------------------------------------------
    ///		@brief : Constructs a 2D AABB from a center position and width and height.  
    //----------------------------------------------------------------------------------------------------
    template <FloatingPointType Type>
    constexpr TBox2<Type>::TBox2(const TVector2<Type>& center, const Type width, const Type height)
        : m_center(center)
        , m_halfExtents(width * static_cast<Type>(0.5f), height * static_cast<Type>(0.5f))
    {
        //
    }

    //----------------------------------------------------------------------------------------------------
    ///		@brief : Constructs a 2D AABB to contain an array of points.
    //----------------------------------------------------------------------------------------------------
    template <FloatingPointType Type>
    constexpr TBox2<Type>::TBox2(const TVector2<Type>* points, const size_t count)
    {
        NES_ASSERT(points != nullptr);
        size_t iMin = std::numeric_limits<size_t>::max();        
        size_t iMax = std::numeric_limits<size_t>::max();
        
        math::ExtremePointsAlongDirection2(TVector2<Type>::GetRightVector(), points, count, iMin, iMax);
        m_halfExtents.x = (points[iMax].x - points[iMin].x) * static_cast<Type>(0.5f);
        m_center.x = points[iMin].x + m_halfExtents.x;

        math::ExtremePointsAlongDirection2(TVector2<Type>::GetUpVector(), points, count, iMin, iMax);
        m_halfExtents.y = (points[iMax].y - points[iMin].y) * static_cast<Type>(0.5f);
        m_center.y = points[iMin].y + m_halfExtents.y;
    }

    template <FloatingPointType Type>
    constexpr bool TBox2<Type>::operator==(const TBox2& other) const
    {
        return m_center == other.m_center && m_halfExtents == other.m_halfExtents;
    }

    //----------------------------------------------------------------------------------------------------
    ///		@brief : Returns the minimum point of the Box. 
    //----------------------------------------------------------------------------------------------------
    template <FloatingPointType Type>
    constexpr TVector2<Type> TBox2<Type>::Min() const
    {
        return m_center - m_halfExtents;
    }

    //----------------------------------------------------------------------------------------------------
    ///		@brief : Returns the maximum point of the Box. 
    //----------------------------------------------------------------------------------------------------
    template <FloatingPointType Type>
    constexpr TVector2<Type> TBox2<Type>::Max() const
    {
        return m_center + m_halfExtents;
    }

    //----------------------------------------------------------------------------------------------------
    ///		@brief : Returns the size of each dimension of the Box. (x == width and y == height). 
    //----------------------------------------------------------------------------------------------------
    template <FloatingPointType Type>
    constexpr TVector2<Type> TBox2<Type>::Size() const
    {
        return m_halfExtents * static_cast<Type>(2.0f);
    }

    //----------------------------------------------------------------------------------------------------
    ///		@brief : Get the Area of the Box. 
    //----------------------------------------------------------------------------------------------------
    template <FloatingPointType Type>
    constexpr Type TBox2<Type>::Area() const
    {
        const auto size = Size();
        return size.x * size.y;
    }

    //----------------------------------------------------------------------------------------------------
    ///		@brief : Returns whether each extent dimension is greater than 0.
    //----------------------------------------------------------------------------------------------------
    template <FloatingPointType Type>
    constexpr bool TBox2<Type>::HasValidDimensions() const
    {
        return m_halfExtents.x > static_cast<Type>(0.f) && m_halfExtents.y > static_cast<Type>(0.f);
    }

    //----------------------------------------------------------------------------------------------------
    ///		@brief : Returns true if the two Boxes intersect. 
    //----------------------------------------------------------------------------------------------------
    template <FloatingPointType Type>
    constexpr bool TBox2<Type>::Intersects(const TBox2<Type>& other) const
    {
        if (math::Abs<Type>(m_center[0] - other.m_center[0]) > (m_halfExtents[0] + other.m_halfExtents[0])
            || math::Abs<Type>(m_center[1] - other.m_center[1]) > (m_halfExtents[1] + other.m_halfExtents[1]))
            return false;

        return true;
    }

    //----------------------------------------------------------------------------------------------------
    ///		@brief : Returns the closest point on or in the Box from the query point.
    //----------------------------------------------------------------------------------------------------
    template <FloatingPointType Type>
    constexpr TVector2<Type> TBox2<Type>::ClosestPointToPoint(const TVector2<Type>& queryPoint) const
    {
        TVector2<Type> result{};
        const TVector2<Type> min = Min();
        const TVector2<Type> max = Max();
        
        // For each coordinate axis, if the point coordinate value is outside the box,
        // clamp to the box, otherwise keep as is.
        for (int axis = 0; axis < 2; ++axis)
        {
            result[axis] = math::Clamp(queryPoint[axis], min[axis], max[axis]);
        }

        return result;
    }

    //----------------------------------------------------------------------------------------------------
    ///		@brief : Returns the distance from the query point to the closest point on the box. 
    //----------------------------------------------------------------------------------------------------
    template <FloatingPointType Type>
    Type TBox2<Type>::DistanceToPoint(const TVector2<Type>& queryPoint) const
    {
        return std::sqrt(SquaredDistanceToPoint(queryPoint));
    }

    //----------------------------------------------------------------------------------------------------
    ///		@brief : Returns the squared distance from the query point to the closest point on the box. 
    //----------------------------------------------------------------------------------------------------
    template <FloatingPointType Type>
    constexpr Type TBox2<Type>::SquaredDistanceToPoint(const TVector2<Type>& queryPoint) const
    {
        Type sqrDist{};
        const TVector2<Type> min = Min();
        const TVector2<Type> max = Max();

        // For each axis, add any excess distance outside the extents. 
        for (int axis = 0; axis < 2; ++axis)
        {
            const float value = queryPoint[axis];
            if (value < min[axis])
            {
                sqrDist += math::Squared(min[axis] - value);
            }

            if (value > max[axis])
            {
                sqrDist += math::Squared(value - max[axis]);
            }
        }

        return sqrDist;
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
    void TBox2<Type>::Transform(const TBox2& original, const TMatrix3x3<Type>& transform, TBox2& result)
    {
        // For each dimension:
        for (int i = 0; i < 2; ++i)
        {
            // Set to the initial Translation:
            result.m_center[i] = transform.m[2][i];
            result.m_halfExtents[i] = 0;
            
            for (int j = 0; j < 2; ++j)
            {
                result.m_center[i] += transform.m[i][j] * original.m_center[j];
                result.m_halfExtents[i] += math::Abs(transform.m[i][j]) * original.m_halfExtents[j];
            }
        }
    }

    template <FloatingPointType Type>
    std::string TBox2<Type>::ToString() const
    {
        return CombineIntoString("(center=", m_center, ", size=", Size(), ")");
    }

    //----------------------------------------------------------------------------------------------------
    ///		@brief : Constructs a 3D AABB from a center and half-extents. 
    ///		@param center : Center position of the Box.
    ///		@param halfExtents : Half the length of each dimension. Can be thought of as radii. 
    //----------------------------------------------------------------------------------------------------
    template <FloatingPointType Type>
    constexpr TBox3<Type>::TBox3(const TVector3<Type>& center, const TVector3<Type>& halfExtents)
        : m_center(center)
        , m_halfExtents(halfExtents)
    {
        //
    }

    //----------------------------------------------------------------------------------------------------
    ///		@brief : Constructs a 3D AABB from a center position and width and height.  
    //----------------------------------------------------------------------------------------------------
    template <FloatingPointType Type>
    constexpr TBox3<Type>::TBox3(const TVector3<Type>* points, const size_t count)
    {
        size_t iMin = std::numeric_limits<size_t>::max();        
        size_t iMax = std::numeric_limits<size_t>::max();
        
        math::ExtremePointsAlongDirection2(TVector2<Type>::GetRightVector(), points, count, iMin, iMax);
        m_halfExtents.x = (points[iMax].x - points[iMin].x) * static_cast<Type>(0.5f);
        m_center.x = points[iMin].x + m_halfExtents.x;

        math::ExtremePointsAlongDirection2(TVector2<Type>::GetUpVector(), points, count, iMin, iMax);
        m_halfExtents.y = (points[iMax].y - points[iMin].y) * static_cast<Type>(0.5f);
        m_center.y = points[iMin].y + m_halfExtents.y;

        math::ExtremePointsAlongDirection2(TVector2<Type>::GetForwardVector(), points, count, iMin, iMax);
        m_halfExtents.z = (points[iMax].z - points[iMin].z) * static_cast<Type>(0.5f);
        m_center.z = points[iMin].z + m_halfExtents.z;
    }

    template <FloatingPointType Type>
    constexpr bool TBox3<Type>::operator==(const TBox3& other) const
    {
        return m_center == other.m_center && m_halfExtents == other.m_halfExtents;
    }

    //----------------------------------------------------------------------------------------------------
    ///		@brief : Returns the minimum point of the Box. 
    //----------------------------------------------------------------------------------------------------
    template <FloatingPointType Type>
    constexpr TVector3<Type> TBox3<Type>::Min() const
    {
        return m_center - m_halfExtents;
    }

    //----------------------------------------------------------------------------------------------------
    ///		@brief : Returns the maximum point of the Box. 
    //----------------------------------------------------------------------------------------------------
    template <FloatingPointType Type>
    constexpr TVector3<Type> TBox3<Type>::Max() const
    {
        return m_center + m_halfExtents;
    }

    //----------------------------------------------------------------------------------------------------
    ///		@brief : Returns the size of each dimension of the Box. (x == width, y == height, z == depth). 
    //----------------------------------------------------------------------------------------------------
    template <FloatingPointType Type>
    constexpr TVector3<Type> TBox3<Type>::Size() const
    {
        return m_halfExtents * static_cast<Type>(2.0f);
    }

    //----------------------------------------------------------------------------------------------------
    ///		@brief : Returns the Volume of the Box.
    //----------------------------------------------------------------------------------------------------
    template <FloatingPointType Type>
    constexpr Type TBox3<Type>::Volume() const
    {
        const auto size = Size();
        return size.x * size.y * size.z;
    }

    //----------------------------------------------------------------------------------------------------
    ///		@brief : Returns whether each extent dimension is greater than 0.
    //----------------------------------------------------------------------------------------------------
    template <FloatingPointType Type>
    constexpr bool TBox3<Type>::HasValidDimensions() const
    {
        return m_halfExtents.x > static_cast<Type>(0.f)
            && m_halfExtents.y > static_cast<Type>(0.f)
            && m_halfExtents.z > static_cast<Type>(0.f);
    }
    
    //----------------------------------------------------------------------------------------------------
    ///		@brief : Returns true if the two Boxes intersect. 
    //----------------------------------------------------------------------------------------------------
    template <FloatingPointType Type>
    constexpr bool TBox3<Type>::Intersects(const TBox3<Type>& other) const
    {
        if (math::Abs<Type>(m_center[0] - other.m_center[0]) > (m_halfExtents[0] + other.m_halfExtents[0])
            || math::Abs<Type>(m_center[1] - other.m_center[1]) > (m_halfExtents[1] + other.m_halfExtents[1])
            || math::Abs<Type>(m_center[2] - other.m_center[2]) > (m_halfExtents[2] + other.m_halfExtents[2]))
            return false;

        return true;
    }

    //----------------------------------------------------------------------------------------------------
    ///		@brief : Returns the closest point on or in the Box from the query point.
    //----------------------------------------------------------------------------------------------------
    template <FloatingPointType Type>
    constexpr TVector3<Type> TBox3<Type>::ClosestPointToPoint(const TVector3<Type>& queryPoint) const
    {
        TVector3<Type> result{};
        const TVector3<Type> min = Min();
        const TVector3<Type> max = Max();
        
        // For each coordinate axis, if the point coordinate value is outside the box,
        // clamp to the box, otherwise keep as is.
        for (int axis = 0; axis < 3; ++axis)
        {
            result[axis] = math::Clamp(queryPoint[axis], min[axis], max[axis]);
        }

        return result;
    }

    //----------------------------------------------------------------------------------------------------
    ///		@brief : Returns the distance from the query point to the closest point on the box. 
    //----------------------------------------------------------------------------------------------------
    template <FloatingPointType Type>
    Type TBox3<Type>::DistanceToPoint(const TVector3<Type>& queryPoint) const
    {
        return std::sqrt(SquaredDistanceToPoint(queryPoint));
    }

    //----------------------------------------------------------------------------------------------------
    ///		@brief : Returns the squared distance from the query point to the closest point on the box. 
    //----------------------------------------------------------------------------------------------------
    template <FloatingPointType Type>
    constexpr Type TBox3<Type>::SquaredDistanceToPoint(const TVector3<Type>& queryPoint) const
    {
        Type sqrDist{};
        const TVector3<Type> min = Min();
        const TVector3<Type> max = Max();

        // For each axis, add any excess distance outside the extents. 
        for (int axis = 0; axis < 3; ++axis)
        {
            const float value = queryPoint[axis];
            if (value < min[axis])
            {
                sqrDist += math::Squared(min[axis] - value);
            }

            if (value > max[axis])
            {
                sqrDist += math::Squared(value - max[axis]);
            }
        }

        return sqrDist;
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
    void TBox3<Type>::Transform(const TBox3& original, const TMatrix4x4<Type>& transform, TBox3& result)
    {
        // For each dimension:
        for (int i = 0; i < 3; ++i)
        {
            // Set to the initial Translation:
            result.m_center[i] = transform.m[3][i];
            result.m_halfExtents[i] = 0;
            
            for (int j = 0; j < 3; ++j)
            {
                result.m_center[i] += transform.m[i][j] * original.m_center[j];
                result.m_halfExtents[i] += math::Abs(transform.m[i][j]) * original.m_halfExtents[j];
            }
        }
    }

    template <FloatingPointType Type>
    std::string TBox3<Type>::ToString() const
    {
        return CombineIntoString("(center=", m_center, ", size=", Size(), ")");
    }
}