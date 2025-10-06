// Frustum.h
#pragma once
#include "Nessie/Geometry/AABox.h"
#include "Nessie/Geometry/Plane.h"

#undef near
#undef far

namespace nes
{
    //----------------------------------------------------------------------------------------------------
    /// @brief : A camera Frustum is made of 6 planes (left, right, top, bottom, near and far) pointing
    ///     inwards.
    //----------------------------------------------------------------------------------------------------
    class Frustum
    {
    public:
        Frustum() = default;

        //----------------------------------------------------------------------------------------------------
        /// @brief : Construct the frustum from a position, forward, up field of view x and y and near and far plane.
        /// @note : The up parameter does not need to be perpendicular to the forward but cannot be collinear.
        //----------------------------------------------------------------------------------------------------
        inline Frustum(const Vec3 position, const Vec3 forward, const Vec3 up, const float fovX, const float fovY, const float near, const float far)
        {
            const Vec3 right = forward.Cross(up).Normalized();
            // Calculate the real up vector (up doesn't need to be perpendicular to forward).
            const Vec3 realUp = right.Cross(forward).Normalized();

            // Near Plane
            m_planes[0] = Plane::FromPointAndNormal(position + near * forward, forward);

            // Top and bottom planes.
            m_planes[1] = Plane::FromPointAndNormal(position, Mat44::MakeRotation(right, 0.5f * fovY) * -realUp);
            m_planes[2] = Plane::FromPointAndNormal(position, Mat44::MakeRotation(right, -0.5f * fovY) * realUp);

            // Left and right planes
            m_planes[3] = Plane::FromPointAndNormal(position, Mat44::MakeRotation(realUp, 0.5f * fovX) * right);
            m_planes[4] = Plane::FromPointAndNormal(position, Mat44::MakeRotation(realUp, -0.5f * fovX) * -right);
            
            // Far Plane. This is last so that we can do an overlap test without testing the far plane.
            m_planes[5] = Plane::FromPointAndNormal(position + far * forward, -forward);
        }

        //----------------------------------------------------------------------------------------------------
        /// @brief : Test if this frustum overlaps with an axis aligned box.
        /// @note : This is a conservative estimate and can return true if the frustum doesn't overlap with the box.
        ///     This is because we only test the plane axis as a separating axis and skip checking the cross-products
        ///     of the edges of the frustum.
        //----------------------------------------------------------------------------------------------------
        inline bool Overlaps(const AABox& box) const
        {
            for (const Plane& plane : m_planes)
            {
                // Get support point (the maximum extent) in the direction of our normal.
                const Vec3 support = box.GetSupport(plane.GetNormal());

                // If this is behind the plane, the box is not inside the frustum
                if (plane.SignedDistanceTo(support) < 0.f)
                    return false;
            }

            return true;
        }

        //----------------------------------------------------------------------------------------------------
        /// @brief : Test if this frustum overlaps with an axis aligned box. This version does not check the far plane.
        /// @note : This is a conservative estimate and can return true if the frustum doesn't overlap with the box.
        ///     This is because we only test the plane axis as a separating axis and skip checking the cross-products
        ///     of the edges of the frustum.
        //----------------------------------------------------------------------------------------------------
        inline bool OverlapsInfinite(const AABox& box) const
        {
            for (size_t i = 0; i < 5; ++i)
            {
                // Get support point (the maximum extent) in the direction of our normal.
                const Vec3 support = box.GetSupport(m_planes[i].GetNormal());

                // If this is behind the plane, the box is not inside the frustum
                if (m_planes[i].SignedDistanceTo(support) < 0.f)
                    return false;
            }

            return true;
        }

        inline void GetBounds(nes::Vec3& outMinBounds, nes::Vec3& outMaxBounds) const
        {
            outMinBounds = nes::Vec3(FLT_MAX);
            outMaxBounds = nes::Vec3(-FLT_MAX);
            
            using PlaneCombo = std::array<int, 3>;
            static constexpr std::array<PlaneCombo, 8> kCornerCombos = 
            {
                PlaneCombo{0, 3, 1}, // Near-Left-Top
                PlaneCombo{0, 3, 2}, // Near-Left-Bottom
                PlaneCombo{0, 4, 1}, // Near-Right-Top
                PlaneCombo{0, 4, 2}, // Near-Right-Bottom
                PlaneCombo{5, 3, 1}, // Far-Left-Top
                PlaneCombo{5, 3, 2}, // Far-Left-Bottom
                PlaneCombo{5, 4, 1}, // Far-Right-Top
                PlaneCombo{5, 4, 2}, // Far-Right-Bottom
            };

            for (const auto& combo : kCornerCombos)
            {
                nes::Vec3 corner;
                if (Plane::IntersectPlanes(m_planes[combo[0]], m_planes[combo[1]], m_planes[combo[2]], corner))
                {
                    outMinBounds = nes::Vec3::Min(outMinBounds, corner);
                    outMaxBounds = nes::Vec3::Max(outMaxBounds, corner);
                }
            }
        }
        
    private:
        Plane       m_planes[6];    /// The planes forming the frustum.
    };    
}