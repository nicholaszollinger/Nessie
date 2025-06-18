// Circle.inl
#pragma once

namespace nes
{
    void Circle::Encapsulate(const Vec2 point)
    {
        const Vec2 toPoint = point - m_center;
        const float distSqr = toPoint.LengthSqr();
        
        // Only update the Circle if the point is outside the sphere:
        if (distSqr > math::Squared(m_radius))
        {
            // It is further away than the radius, so we need to grow the circle.
            // The diameter of the new sphere is radius + dist, so the new radius is half of that.
            const float dist = std::sqrt(distSqr);
            const float newRadius = (m_radius + dist) * (0.5f);

            // The center needs to shift by the newRadius - m_radius in the direction to the point.
            m_center += (newRadius - m_radius) / dist * toPoint;
            m_radius = newRadius;
        }
    }
    
    bool Circle::Overlaps(const Circle& other) const
    {
        const float sqrDist = (m_center - other.m_center).LengthSqr();
        return sqrDist < math::Squared(m_radius + other.m_radius);
    }
    
    bool Circle::Contains(const Vec2 point) const
    {
        return (point - m_center).LengthSqr() < math::Squared(m_radius);
    }

    namespace math
    {
        void ApproximateCircleFromDistancePoints(Circle& circle, const Float2* points, const size_t count)
        {
            // Find the most separated point pair defining the AABB.
            size_t iMin = 0;
            size_t iMax = 0;
            MostSeparatedPointsOnAABB(points, count, iMin, iMax);

            const Vec2 min = Vec2(points[iMin]);
            const Vec2 max = Vec2(points[iMax]);
            
            // Set up the circle to just encompass these two points:
            circle.m_center = (min + max) * 0.5f;
            circle.m_radius = (max - circle.m_center).Length();
        }

        void ApproximateCircleFromDistancePoints(Circle& circle, const Vec2* points, const size_t count)
        {
            // Find the most separated point pair defining the AABB.
            size_t iMin = 0;
            size_t iMax = 0;
            MostSeparatedPointsOnAABB(points, count, iMin, iMax);

            // Set up the sphere to just encompass these two points:
            circle.m_center = ((points[iMin] + points[iMax]) * 0.5f);
            circle.m_radius = ((points[iMax] - circle.m_center).Length());
        }

        void RitterBoundingCircle(Circle& circle, const Float2* points, const size_t count)
        {
            // "Real-Time Collision Detection" (89-91)
            // Get an approximate sphere that encompasses the two most distant points. 
            ApproximateCircleFromDistancePoints(circle, points, count);

            // Grow the Sphere to include all points.
            for (size_t i = 0; i < count; ++i)
            {
                const Vec2 toPoint = Vec2(points[i]) - circle.m_center;
                const float distSqr = toPoint.LengthSqr();
                
                // Only update the Circle if the point is outside the sphere:
                if (distSqr > math::Squared(circle.m_radius))
                {
                    // It is further away than the radius, so we need to grow the circle.
                    // The diameter of the new sphere is radius + dist, so the new radius is half of that.
                    const float dist = std::sqrt(distSqr);
                    const float newRadius = (circle.m_radius + dist) * (0.5f);

                    // The center needs to shift by the newRadius - m_radius in the direction to the point.
                    circle.m_center += (newRadius - circle.m_radius) / dist * toPoint;
                    circle.m_radius = newRadius;
                }
            }
        }

        void RitterBoundingCircle(Circle& circle, const Vec2* points, const size_t count)
        {
            // "Real-Time Collision Detection" (89-91)
            // Get an approximate sphere that encompasses the two most distant points. 
            ApproximateCircleFromDistancePoints(circle, points, count);

            // Grow the Sphere to include all points.
            for (size_t i = 0; i < count; ++i)
            {
                const Vec2 toPoint = Vec2(points[i]) - circle.m_center;
                const float distSqr = toPoint.LengthSqr();
                
                // Only update the Circle if the point is outside the sphere:
                if (distSqr > math::Squared(circle.m_radius))
                {
                    // It is further away than the radius, so we need to grow the circle.
                    // The diameter of the new sphere is radius + dist, so the new radius is half of that.
                    const float dist = std::sqrt(distSqr);
                    const float newRadius = (circle.m_radius + dist) * (0.5f);

                    // The center needs to shift by the newRadius - m_radius in the direction to the point.
                    circle.m_center += (newRadius - circle.m_radius) / dist * toPoint;
                    circle.m_radius = newRadius;
                }
            }
        }
    }
}
