// Sphere.inl
#pragma once

namespace nes
{
    Sphere::Sphere(const Float3* points, const size_t count)
    {
        math::RitterBoundingSphere(*this, points, count);
    }

    Sphere::Sphere(const Vec3* points, const size_t count)
    {
        math::RitterBoundingSphere(*this, points, count);
    }
    
    bool Sphere::Overlaps(const Sphere& other) const
    {
        return (Vec3::LoadFloat3Unsafe(m_center) - Vec3::LoadFloat3Unsafe(other.m_center)).LengthSqr() <= math::Squared(m_radius + other.m_radius);
    }

    bool Sphere::Overlaps(const AABox& box) const
    {
        return box.GetSqrDistanceTo(GetCenter()) <= math::Squared(m_radius);
    }

    void Sphere::Encapsulate(const Vec3 point)
    {
        Vec3 center = GetCenter();
        const Vec3 toPoint = point - center;
        const float distSqr = toPoint.LengthSqr();

        // Only update the Sphere if the point is outside the sphere:
        if (distSqr > math::Squared(m_radius))
        {
            // It is further away than the radius, so we need to grow the sphere.
            // The diameter of the new sphere is radius + dist, so the new radius is half of that.
            const float dist = std::sqrt(distSqr);
            const float newRadius = (m_radius + dist) * (0.5f);

            // The center needs to shift by the newRadius - m_radius in the direction to the point.
            center += (newRadius - m_radius) / dist * toPoint;

            // Store the new sphere values.
            center.StoreFloat3(&m_center);
            m_radius = newRadius;
        }
    }

    float Sphere::Volume() const
    {
        // V = 4/3 * pi * r^3
        return (4.f / 3.f) * math::Pi<float>() * math::Cubed(m_radius);
    }

    float Sphere::SurfaceArea() const
    {
        // S = 4 * pi * r^2.
        return 4.f * math::Pi<float>() * math::Squared(m_radius);
    }

    namespace math
    {
        void ApproximateSphereFromDistantPoints(Sphere& sphere, const Float3* points, const size_t count)
        {
            // Find the most separated point pair defining the AABB.
            size_t iMin = 0;
            size_t iMax = 0;
            MostSeparatedPointsOnAABB(points, count, iMin, iMax);

            const Vec3 min = Vec3::LoadFloat3Unsafe(points[iMin]);
            const Vec3 max = Vec3::LoadFloat3Unsafe(points[iMax]);
            
            // Set up the sphere to just encompass these two points:
            sphere.SetCenter((min + max) * 0.5f);
            sphere.SetRadius((max - sphere.GetCenter()).Length());
        }
        
        void ApproximateSphereFromDistantPoints(Sphere& sphere, const Vec3* points, const size_t count)
        {
            // Find the most separated point pair defining the AABB.
            size_t iMin = 0;
            size_t iMax = 0;
            MostSeparatedPointsOnAABB(points, count, iMin, iMax);

            // Set up the sphere to just encompass these two points:
            sphere.SetCenter((points[iMin] + points[iMax]) * 0.5f);
            sphere.SetRadius((points[iMax] - sphere.GetCenter()).Length());
        }
        
        void RitterBoundingSphere(Sphere& sphere, const Float3* points, const size_t count)
        {
            // "Real-Time Collision Detection" (89-91)
            // Get an approximate sphere that encompasses the two most distant points. 
            ApproximateSphereFromDistantPoints(sphere, points, count);

            Vec3 center = sphere.GetCenter();
            float radius = sphere.GetRadius();
            
            // Grow the Sphere to include all points.
            for (size_t i = 0; i < count; ++i)
            {
                const Vec3 toPoint = Vec3::LoadFloat3Unsafe(points[i]) - center;
                const float distSqr = toPoint.LengthSqr();
                if (distSqr > math::Squared(radius))
                {
                    // It is further away than the radius, so we need to grow the sphere.
                    // The diameter of the new sphere is radius + dist, so the new radius is half of that.
                    const float dist = std::sqrt(distSqr);
                    const float newRadius = (radius + dist) * (0.5f);

                    // The center needs to shift by the newRadius - m_radius in the direction to the point.
                    center += (newRadius - radius) / dist * toPoint;
                    radius = newRadius;
                }
            }
            
            sphere.SetCenter(center);
            sphere.SetRadius(radius);
        }
        
        void RitterBoundingSphere(Sphere& sphere, const Vec3* points, const size_t count)
        {
            // "Real-Time Collision Detection" (89-91)
            // Get an approximate sphere that encompasses the two most distant points. 
            ApproximateSphereFromDistantPoints(sphere, points, count);

            Vec3 center = sphere.GetCenter();
            float radius = sphere.GetRadius();
            
            // Grow the Sphere to include all points.
            for (size_t i = 0; i < count; ++i)
            {
                const Vec3 toPoint = points[i] - center;
                const float distSqr = toPoint.LengthSqr();
                if (distSqr > math::Squared(radius))
                {
                    // It is further away than the radius, so we need to grow the sphere.
                    // The diameter of the new sphere is radius + dist, so the new radius is half of that.
                    const float dist = std::sqrt(distSqr);
                    const float newRadius = (radius + dist) * (0.5f);

                    // The center needs to shift by the newRadius - m_radius in the direction to the point.
                    center += (newRadius - radius) / dist * toPoint;
                    radius = newRadius;
                }
            }

            sphere.SetCenter(center);
            sphere.SetRadius(radius);
        }
    }
}