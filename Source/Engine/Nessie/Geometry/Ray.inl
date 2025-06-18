// Ray.inl
#pragma once

namespace nes
{
    Vec2 Ray2::PositionAlongRay(const float distance) const
    {
        if (distance <= 0.f)
            return m_origin;
        
        return m_origin + (m_direction * distance);
    }
    
    float Ray2::DistanceSqrToPoint(const Vec2 point) const
    {
        const float projectedDistance = Vec2::Dot(m_direction, point - m_origin);

        // If the projected point is behind the origin, return the squared distance
        // to the origin point.
        if (projectedDistance < 0)
        {
            return Vec2::DistanceSqr(m_origin, point);
        }

        // Otherwise, get the distance squared from the projected position along the ray to
        // the query point.
        const Vec2 projectedPoint = m_origin + (m_direction * projectedDistance);
        return Vec2::DistanceSqr(point, projectedPoint);
    }
    
    Vec2 Ray2::ClosestPoint(const Vec2 point) const
    {
        const float projectedDistance = Vec2::Dot(m_direction, point - m_origin);

        // If the projected point is behind the ray, return the origin.
        if (projectedDistance < 0)
            return m_origin;

        // Otherwise, return the point that is the projected distance along the ray.
        return m_origin + (m_direction * projectedDistance);
    }
    
    Vec3 Ray::PositionAlongRay(const float distance)
    {
        if (distance <= 0)
            return m_origin;
        
        return m_origin + (m_direction * distance);
    }
    
    float Ray::DistanceSqrToPoint(const Vec3 point) const
    {
        const float projectedDistance = Vec3::Dot(m_direction, point - m_origin);

        // If the projected point is behind the origin, return the squared distance
        // to the origin point.
        if (projectedDistance < 0)
        {
            return Vec3::DistanceSqr(m_origin, point);
        }

        // Otherwise, get the distance squared from the projected position along the ray to
        // the query point.
        const Vec3 projectedPoint = m_origin + (m_direction * projectedDistance);
        return Vec3::DistanceSqr(point, projectedPoint);
    }
    
    Vec3 Ray::ClosestPoint(const Vec3 point) const
    {
        const float projectedDistance = Vec3::Dot(m_direction, point - m_origin);

        // If the projected point is behind the ray, return the origin.
        if (projectedDistance < 0)
            return m_origin;

        // Otherwise return the point that is the projected distance along the ray.
        return m_origin + (m_direction * projectedDistance);
    }
    
    Ray Ray::Transformed(const Mat44& transform) const
    {
        Vec3 rayOrigin = transform.TransformPoint(m_origin);
        Vec3 direction(transform.TransformPoint(m_origin + m_direction) - rayOrigin);
        return Ray(rayOrigin, direction);
    }
}