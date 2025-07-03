// RayCast.h
#pragma once
#include "BackFaceMode.h"
#include "Nessie/Math/Math.h"

namespace nes
{
    template <typename VecType, typename MatType, typename RayCastType>
    struct TRayCast
    {
        VecType m_origin;       /// Origin of the Ray.
        Vec3 m_direction;       /// Direction and length of the cast (anything beyond this length will not be reported as a hit).

        TRayCast() = default;
        TRayCast(const VecType& origin, const Vec3 direction) : m_origin(origin), m_direction(direction) {}
        TRayCast(const TRayCast& other) = default;
        
        //----------------------------------------------------------------------------------------------------
        /// @brief : Get a point with fraction on the ray from m_origin to m_origin + m_direction * fraction.
        ///     (0 = start of the ray and 1 = end of the ray).
        //----------------------------------------------------------------------------------------------------
        inline VecType GetPointAlongRay(const float fraction) const
        {
            return m_origin + (m_direction * fraction);
        }

        RayCastType Transformed(const MatType& transform) const
        {
            const VecType rayOrigin = transform * m_origin;
            const Vec3 direction(transform * (m_origin + m_direction) - rayOrigin);
            return RayCastType(rayOrigin, direction);
        }
        
        RayCastType Translated(const VecType& translation) const
        {
            return RayCastType(translation + m_origin, m_direction);
        }
    };

    //----------------------------------------------------------------------------------------------------
    /// @brief : Single precision RayCast type. 
    //----------------------------------------------------------------------------------------------------
    struct RayCast : public TRayCast<Vec3, Mat44, RayCast>
    {
        using TRayCast<Vec3, Mat44, RayCast>::TRayCast;
    };

    //----------------------------------------------------------------------------------------------------
    /// @brief : RayCast type whose precision is based on Real. 
    //----------------------------------------------------------------------------------------------------
    struct RRayCast : public TRayCast<RVec3, Mat44, RRayCast>
    {
        using TRayCast<RVec3, Mat44, RRayCast>::TRayCast;

        //----------------------------------------------------------------------------------------------------
        /// @brief : Explicit cast form RayCast. Converts from single to double precision. 
        //----------------------------------------------------------------------------------------------------
        explicit RRayCast(const RayCast& ray) : RRayCast(RVec3(ray.m_origin), ray.m_direction) {}

        //----------------------------------------------------------------------------------------------------
        /// @brief : Convert to RayCast, which implies casting from double precision to single precision. 
        //----------------------------------------------------------------------------------------------------
        explicit operator RayCast() const { return RayCast(Vec3(m_origin), m_direction); }
    };

    //----------------------------------------------------------------------------------------------------
    /// @brief : Settings to be passed with a ray cast. 
    //----------------------------------------------------------------------------------------------------
    struct RayCastSettings
    {
        /// How backfacing triangles should be treated. Should we report back facing hits for triangle based shapes, e.g.
        /// MeshShape/HeightFieldShape?
        EBackFaceMode   m_backfaceModeTriangles     = EBackFaceMode::IgnoreBackFaces;

        /// How back facing convex objects should be treated. Should we report back facing hits on convex shapes?
        EBackFaceMode   m_backfaceModeConvex        = EBackFaceMode::IgnoreBackFaces;

        /// If convex shapes should be treated as solid. When true, a ray starting inside a convex shape will generate a hit at fraction 0.
        bool            m_treatConvexAsSolid        = true;

        //----------------------------------------------------------------------------------------------------
        /// @brief : Set the back face mode for all shapes. 
        //----------------------------------------------------------------------------------------------------
        void            SetBackFaceMode(const EBackFaceMode& backfaceMode) { m_backfaceModeTriangles = m_backfaceModeConvex = backfaceMode; }
    };
}