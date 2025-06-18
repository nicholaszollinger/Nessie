// RayCast.h
#pragma once
#include "BackFaceMode.h"
#include "Math/Mat4.h"
#include "Math/Vec3.h"

namespace nes
{
    struct RayCast
    {
        Vec3 m_origin;       /// Origin of the Ray.
        Vec3 m_direction;    /// Direction and length of the cast (anything beyond this length will not be reported as a hit).

        RayCast() = default;
        RayCast(const Vec3& origin, const Vec3& direction) : m_origin(origin), m_direction(direction) {}
        RayCast(const RayCast& other) = default;
        
        //----------------------------------------------------------------------------------------------------
        /// @brief : Get a point with fraction on the ray from m_origin to m_origin + m_direction * fraction.
        ///     (0 = start of the ray and 1 = end of the ray).
        //----------------------------------------------------------------------------------------------------
        Vec3 GetPointAlongRay(const float fraction) const
        {
            return m_origin + (m_direction * fraction);
        }

        RayCast Transformed(const Mat44& transform) const
        {
            const Vec3 rayOrigin = transform.TransformPoint(m_origin);
            const Vec3 direction(transform.TransformPoint(m_origin + m_direction) - rayOrigin);
            return RayCast(rayOrigin, direction);    
        }
        
        RayCast Translated(const Vec3& translation) const
        {
            return RayCast(translation + m_origin, m_direction);
        }
    };

    //----------------------------------------------------------------------------------------------------
    /// @brief : Settings to be passed with a ray cast. 
    //----------------------------------------------------------------------------------------------------
    struct RayCastSettings
    {
        /// How backfacing triangles should be treated. Should we report back facing hits for triangle based shapes, e.g.
        /// MeshShape/HeightFieldShape?
        EBackFaceMode   m_backfaceModeTriangles    = EBackFaceMode::IgnoreBackFaces;

        /// How back facing convex objects should be treated. Should we report back facing hits on convex shapes?
        EBackFaceMode   m_backfaceModeConvex       = EBackFaceMode::IgnoreBackFaces;

        /// If convex shapes should be treated as solid. When true, a ray starting inside a convex shape will generate a hit at fraction 0.
        bool            m_treatConvexAsSolid = true;

        //----------------------------------------------------------------------------------------------------
        /// @brief : Set the back face mode for all shapes. 
        //----------------------------------------------------------------------------------------------------
        void            SetBackFaceMode(const EBackFaceMode& backfaceMode) { m_backfaceModeTriangles = m_backfaceModeConvex = backfaceMode; }
    };
}