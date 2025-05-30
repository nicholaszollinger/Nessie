// RayCast.h
#pragma once
#include "BackFaceMode.h"
#include "Math/Matrix.h"
#include "Math/Vector3.h"

namespace nes
{
    struct RayCast
    {
        Vector3 m_origin;       /// Origin of the Ray.
        Vector3 m_direction;    /// Direction and length of the cast (anything beyond this length will not be reported as a hit).

        RayCast() = default;
        RayCast(const Vector3& origin, const Vector3& direction) : m_origin(origin), m_direction(direction) {}
        RayCast(const RayCast& other) = default;
        
        //----------------------------------------------------------------------------------------------------
        /// @brief : Get a point with fraction on the ray from m_origin to m_origin + m_direction * fraction.
        ///     (0 = start of the ray and 1 = end of the ray).
        //----------------------------------------------------------------------------------------------------
        Vector3 GetPointAlongRay(const float fraction) const
        {
            return m_origin + (m_direction * fraction);
        }

        RayCast Transformed(const Mat4& transform) const
        {
            const Vector3 rayOrigin = transform.TransformPoint(m_origin);
            const Vector3 direction(transform.TransformPoint(m_origin + m_direction) - rayOrigin);
            return RayCast(rayOrigin, direction);    
        }
        
        RayCast Translated(const Vector3& translation) const
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