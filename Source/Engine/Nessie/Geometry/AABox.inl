// AABox.inl
#pragma once

namespace nes
{
    void AABox::Reset()
    {
        m_min = Vec3::Replicate(FLT_MAX);
        m_max = Vec3::Replicate(-FLT_MAX);
    }

    void AABox::Encapsulate(const Vec3 position)
    {
        m_min = Vec3::Min(m_min, position);
        m_max = Vec3::Max(m_max, position);
    }

    void AABox::Encapsulate(const AABox& box)
    {
        m_min = Vec3::Min(m_min, box.m_min);
        m_max = Vec3::Max(m_max, box.m_max);
    }

    AABox AABox::Intersect(const AABox& other) const
    {
        return AABox(Vec3::Max(m_min, other.m_min), Vec3::Min(m_max, other.m_max));
    }

    void AABox::EnsureMinimalEdgeLength(const float minEdgeLength)
    {
        const Vec3 minLength = Vec3::Replicate(minEdgeLength);
        m_max = Vec3::Select(m_max, m_min + minLength, Vec3::Less(m_max - m_min, minLength));
    }

    void AABox::ExpandBy(const Vec3 distance)
    {
        m_min -= distance;
        m_max += distance;
    }

    float AABox::SurfaceArea() const
    {
        const Vec3 extent = m_max - m_min;
        return 2.f * (extent.x * extent.y + extent.x * extent.z + extent.y * extent.z);
    }

    float AABox::Volume() const
    {
        const Vec3 extent = m_max - m_min;
        return extent.x * extent.y * extent.z;
    }

    bool AABox::Contains(const AABox& other) const
    {
        return UVec4Reg::And(Vec3::LessOrEqual(m_min, other.m_min), Vec3::GreaterOrEqual(m_max, other.m_max)).TestAllXYZTrue();
    }

    bool AABox::Contains(const Vec3 point) const
    {
        return UVec4Reg::And(Vec3::LessOrEqual(m_min, point), Vec3::GreaterOrEqual(m_max, point)).TestAllXYZTrue();
    }

    bool AABox::Overlaps(const AABox& other) const
    {
        return !UVec4Reg::Or(Vec3::Greater(m_min, other.m_max), Vec3::Less(m_max, other.m_min)).TestAnyXYZTrue();
    }

    bool AABox::Overlaps(const Plane& plane) const
    {
        const Vec3 normal = plane.GetNormal();
        const float distNormal = plane.SignedDistanceTo(GetSupport(normal));
        const float distMinNormal = plane.SignedDistanceTo(GetSupport(-normal));

        // If both support points are on the same side of the plane, then there is no overlap.
        return distNormal * distMinNormal <= 0.f; 
    }

    void AABox::Translate(const Vec3 translation)
    {
        m_min += translation;
        m_max += translation;
    }

    AABox AABox::Transformed(const Mat44& matrix) const
    {
        // Start with the translation
        Vec3 newMax;
        Vec3 newMin = newMax = matrix.GetTranslation();

        // Now find the extreme points by considering the product of the min and the max with each column of the matrix.
        for (int col = 0; col < 3; ++col)
        {
            Vec3 column = matrix.GetColumn3(col);

            Vec3 a = column * m_min[col];
            Vec3 b = column * m_max[col];

            newMin += Vec3::Min(a, b);
            newMax += Vec3::Max(a, b);
        }

        return AABox(newMin, newMax);
    }

    AABox AABox::Scaled(const Vec3 scale) const
    {
        return AABox::FromTwoPoints(m_min * scale, m_max * scale);
    }

    Vec3 AABox::ClosestPointTo(const Vec3 point) const
    {
        return Vec3::Min(Vec3::Max(point, m_min), m_max);
    }

    float AABox::GetSqrDistanceTo(const Vec3 point) const
    {
        return (ClosestPointTo(point) - point).LengthSqr();
    }

    Vec3 AABox::GetSupport(const Vec3 direction) const
    {
        return Vec3::Select(m_max, m_min, Vec3::Less(direction, Vec3::Zero()));
    }
    
    template <typename VertexArray>
    void AABox::GetSupportingFace(const Vec3& direction, VertexArray& outVertices) const
    {
        outVertices.resize(4);
        
        int axis = direction.Abs().MaxComponentIndex();
        if (direction[axis] < 0.0f)
        {
            switch (axis)
            {
                case 0:
                {
                    outVertices[0] = Vec3(m_max.x, m_min.y, m_min.z);
                    outVertices[1] = Vec3(m_max.x, m_max.y, m_min.z);
                    outVertices[2] = Vec3(m_max.x, m_max.y, m_max.z);
                    outVertices[3] = Vec3(m_max.x, m_min.y, m_max.z);
                    break;
                }
                    
                case 1:
                {
                    outVertices[0] = Vec3(m_min.x, m_max.y, m_min.z);
                    outVertices[1] = Vec3(m_min.x, m_max.y, m_max.z);
                    outVertices[2] = Vec3(m_max.x, m_max.y, m_max.z);
                    outVertices[3] = Vec3(m_max.x, m_max.y, m_min.z);
                    break;
                }
        
                case 2:
                {
                    outVertices[0] = Vec3(m_min.x, m_min.y, m_max.z);
                    outVertices[1] = Vec3(m_max.x, m_min.y, m_max.z);
                    outVertices[2] = Vec3(m_max.x, m_max.y, m_max.z);
                    outVertices[3] = Vec3(m_min.x, m_max.y, m_max.z);
                    break;
                }

                default: NES_ASSERT(false); break;
            }
        }
        else
        {
            switch (axis)
            {
                case 0:
                {
                    outVertices[0] = Vec3(m_min.x, m_min.y, m_min.z);
                    outVertices[1] = Vec3(m_min.x, m_min.y, m_max.z);
                    outVertices[2] = Vec3(m_min.x, m_max.y, m_max.z);
                    outVertices[3] = Vec3(m_min.x, m_max.y, m_min.z);
                    break;
                }
        
                case 1:
                {
                    outVertices[0] = Vec3(m_min.x, m_min.y, m_min.z);
                    outVertices[1] = Vec3(m_max.x, m_min.y, m_min.z);
                    outVertices[2] = Vec3(m_max.x, m_min.y, m_max.z);
                    outVertices[3] = Vec3(m_min.x, m_min.y, m_max.z);
                    break;
                }
        
                case 2:
                {
                    outVertices[0] = Vec3(m_min.x, m_min.y, m_min.z);
                    outVertices[1] = Vec3(m_min.x, m_max.y, m_min.z);
                    outVertices[2] = Vec3(m_max.x, m_max.y, m_min.z);
                    outVertices[3] = Vec3(m_max.x, m_min.y, m_min.z);
                    break;
                }

                default: NES_ASSERT(false); break;
            }
        }
    }

    namespace math
    {
        inline void MostSeparatedPointsOnAABB(const Vec3* points, const size_t count, size_t& iMin, size_t& iMax)
        {
            // "Real-Time Collision Detection" (89). 
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
            const float sqrDistX = Vec3::DistanceSqr(points[minIndices[0]], points[maxIndices[0]]);
            const float sqrDistY = Vec3::DistanceSqr(points[minIndices[1]], points[maxIndices[1]]);
            const float sqrDistZ = Vec3::DistanceSqr(points[minIndices[2]], points[maxIndices[2]]);
            
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

        inline void MostSeparatedPointsOnAABB(const Float3* points, const size_t count, size_t& iMin, size_t& iMax)
        {
            // "Real-Time Collision Detection" (89). 
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
            const float sqrDistX = Vec3::DistanceSqr(Vec3::LoadFloat3Unsafe(points[minIndices[0]]), Vec3::LoadFloat3Unsafe(points[maxIndices[0]]));
            const float sqrDistY = Vec3::DistanceSqr(Vec3::LoadFloat3Unsafe(points[minIndices[1]]), Vec3::LoadFloat3Unsafe(points[maxIndices[1]]));
            const float sqrDistZ = Vec3::DistanceSqr(Vec3::LoadFloat3Unsafe(points[minIndices[2]]), Vec3::LoadFloat3Unsafe(points[maxIndices[2]]));
            
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
    }
}