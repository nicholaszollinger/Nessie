// AABox2.inl
#pragma once

namespace nes
{
    void AABox2::Reset()
    {
        m_min = Vec2(FLT_MAX);
        m_max = Vec2(-FLT_MAX);
    }

    void AABox2::Encapsulate(const Vec2& point)
    {
        m_min = Vec2::Min(m_min, point);
        m_max = Vec2::Max(m_max, point);
    }

    void AABox2::Encapsulate(const AABox2& box)
    {
        m_min = Vec2::Min(m_min, box.m_min);
        m_max = Vec2::Max(m_max, box.m_max);
    }

    AABox2 AABox2::Intersect(const AABox2& other) const
    {
        return AABox2(Vec2::Max(m_min, other.m_min), Vec2::Min(m_max, other.m_max));
    }

    void AABox2::EnsureMinimalEdgeLength(const float minEdgeLength)
    {
        const Vec2 minLength(minEdgeLength);
        // [TODO]: 
        //m_max = Vec2::Select(m_max, m_min + minLength, Vec2::Less(m_max - m_min, minLength));
    }

    void AABox2::ExpandBy(const Vec2 distance)
    {
        m_min -= distance;
        m_max += distance;
    }

    float AABox2::Area() const
    {
        const Vec2 size = Size();
        return size.x * size.y;
    }

    bool AABox2::Contains(const AABox2& other) const
    {
        if (m_min < other.m_max && m_max > other.m_min)
            return false;
        
        return true;
    }

    bool AABox2::Contains(const Vec2 point) const
    {
        if (m_min < point && m_max > point)
            return false;
        
        return true;
    }

    bool AABox2::Overlaps(const AABox2& other) const
    {
        if (m_min > other.m_max || m_max < other.m_min)
            return false;
        
        return true;
    }

    void AABox2::Translate(const Vec2 translation)
    {
        m_min += translation;
        m_max += translation;
    }

    AABox2 AABox2::Transformed(const Mat44& matrix) const
    {
        const Vec3 translation = matrix.GetColumn3(2);
        
        // Start with the translation
        Vec2 newMax;
        Vec2 newMin = newMax = Vec2(translation.x, translation.y);
        
        // Now find the extreme points by considering the product of the min and the max with each column of the matrix.
        for (int col = 0; col < 3; ++col)
        {
            const Vec3 c3 = matrix.GetColumn3(col);
            const Vec2 column = Vec2(c3.x, c3.y);

            const Vec2 a = column * m_min[col];
            const Vec2 b = column * m_max[col];

            newMin += Vec2::Min(a, b);
            newMax += Vec2::Max(a, b);
        }

        return AABox2(newMin, newMax);
    }

    AABox2 AABox2::Scaled(const Vec2 scale) const
    {
        return FromTwoPoints(m_min * scale, m_max * scale);
    }

    Vec2 AABox2::GetClosestPoint(const Vec2 queryPoint) const
    {
        return Vec2::Min(Vec2::Max(queryPoint, m_min), m_max);
    }

    float AABox2::DistanceSqrTo(const Vec2 queryPoint) const
    {
        return (GetClosestPoint(queryPoint) - queryPoint).LengthSqr();
    }

    namespace math
    {
        void MostSeparatedPointsOnAABB2(const Vec2* points, const size_t count, size_t& iMin, size_t& iMax)
        {
            // "Real-Time Collision Detection" (89). 
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
            const float sqrDistX = Vec2::DistanceSqr(points[minIndices[0]], points[maxIndices[0]]);
            const float sqrDistY = Vec2::DistanceSqr(points[minIndices[1]], points[maxIndices[1]]);
        
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

        void MostSeparatedPointsOnAABB2(const Float2* points, const size_t count, size_t& iMin, size_t& iMax)
        {
            // "Real-Time Collision Detection" (89). 
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
            const float sqrDistX = Vec2::DistanceSqr(Vec2(points[minIndices[0]]), Vec2(points[maxIndices[0]]));
            const float sqrDistY = Vec2::DistanceSqr(Vec2(points[minIndices[1]]), Vec2(points[maxIndices[1]]));
        
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
    }
}
