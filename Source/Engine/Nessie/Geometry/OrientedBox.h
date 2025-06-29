// OBB.h
#pragma once
#include "AABox.h"
#include "Math/Mat44.h"

namespace nes
{
    //----------------------------------------------------------------------------------------------------
    ///	@brief : An Oriented Bounding Box (OBB) is a 3D box, but with an arbitrary orientation.
    //----------------------------------------------------------------------------------------------------
    struct OrientedBox
    {
        Mat44   m_orientation; /// Transform that positions and rotates the local space, axis-aligned box into world space
        Vec3    m_halfExtents; /// Half-extents (half the size of the edge) of the local space axis-aligned box.

        /// Constructors
        OrientedBox() = default;
        OrientedBox(const Mat44& orientation, const Vec3& halfExtents) : m_orientation(orientation), m_halfExtents(halfExtents) {}
        OrientedBox(const Mat44& orientation, const AABox& box);

        //----------------------------------------------------------------------------------------------------
        /// @brief : Get the center of the oriented box.
        //----------------------------------------------------------------------------------------------------
        Vec3    Center() const;
        
        //----------------------------------------------------------------------------------------------------
        /// @brief : Test if this intersects with an axis-aligned box (AABox).
        //----------------------------------------------------------------------------------------------------
        bool    Overlaps(const AABox& box, const float tolerance = 1.0e-6f) const;

        //----------------------------------------------------------------------------------------------------
        /// @brief : Test if two oriented bounding boxes intersect each other.
        //----------------------------------------------------------------------------------------------------
        bool    Overlaps(const OrientedBox& box, const float tolerance = 1.0e-6f) const;

        //----------------------------------------------------------------------------------------------------
        ///	@brief : Returns the closest point on or in the oriented box to the query point. 
        //----------------------------------------------------------------------------------------------------
        Vec3    ClosestPointTo(const Vec3& point) const;

        //----------------------------------------------------------------------------------------------------
        ///	@brief : Returns the distance from the query point to the closest point on the oriented box. 
        //----------------------------------------------------------------------------------------------------
        float   DistanceToPoint(const Vec3& point) const;

        //----------------------------------------------------------------------------------------------------
        /// @brief : Returns the squared distance from the query point to the closest point on the oriented box.  
        //----------------------------------------------------------------------------------------------------
        float   DistanceSqrToPoint(const Vec3& point) const;
    };

    using OBB = OrientedBox;
    
    // namespace math
    // {
    //     template <FloatingPointType Type>
    //     Type ComputeMinAreaRect(const TVector2<Type>* convexHullVertices, const size_t count, TVector2<Type>& center, TVector2<Type> orientation[2]);
    // }
}

namespace nes
{
    // //----------------------------------------------------------------------------------------------------
    // //		NOTES:
    // //      O(n^2).
    // //		
    // ///		@brief : Computes the center point, axis orientation, of the minimum area rectangle in the xy plane
    // ///             containing the convex hull.
    // ///		@param convexHullVertices : Array of vertices to encompass
    // ///		@param count : The number of points in the array.
    // ///		@param center : Resulting center of the minimum Rect.
    // ///		@param orientation : The X and Y axes of the Rect. 
    // ///		@returns : Minimum Area of the Rect.
    // //----------------------------------------------------------------------------------------------------
    // template <FloatingPointType Type>
    // Type math::ComputeMinAreaRect(const TVector2<Type>* convexHullVertices, const size_t count, TVector2<Type>& center,
    //     TVector2<Type> orientation[2])
    // {
    //     Type minArea = std::numeric_limits<Type>::max();
    //
    //     // Loop through all edges; j trails i by 1, module count
    //     for (size_t i = 0, j = count - 1; i < count; j = i, ++i)
    //     {
    //         // Get the current edge, and normalize it.
    //         TVector2<Type> edge = (convexHullVertices[i] - convexHullVertices[j]).Normalized();
    //
    //         // Get the perpendicular axis to that edge
    //         TVector2<Type> perp = TVector2<Type>::PerpendicularTo(edge);
    //
    //         // Look through all points to get the maximum extents:
    //         Type min0{};
    //         Type min1{};
    //         Type max0{};
    //         Type max1{};
    //         for (size_t k = 0; k < count; ++k)
    //         {
    //             // Project points onto axes "edge" and "perp" and keep track of the minimum
    //             // and maximum values along both axes.
    //             TVector2<Type> d = convexHullVertices[k] - convexHullVertices[j];
    //             float dot = TVector2<Type>::Dot(d, edge);
    //             min0 = math::Min(dot, min0);
    //             max0 = math::Max(dot, max0);
    //
    //             dot = TVector2<Type>::Dot(d, perp);
    //             min1 = math::Min(dot, min1);
    //             max1 = math::Max(dot, max1);
    //         }
    //         
    //         Type area = (max0 - min0) * (max1 - min1);
    //
    //         // If the best so far, remember area, center and axes
    //         if (area < minArea)
    //         {
    //             minArea = area;
    //             center = convexHullVertices[j] + static_cast<Type>(0.5f) * ((min0 + max0) * edge + (min1 + max1) * perp);
    //             orientation[0] = edge;
    //             orientation[1] = perp;
    //         }
    //     }
    //     
    //     return minArea;
    // }
}
