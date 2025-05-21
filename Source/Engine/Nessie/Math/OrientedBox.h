// OBB.h
#pragma once
#include "AABox.h"

namespace nes
{
    namespace math
    {
        template <FloatingPointType Type>
        Type ComputeMinAreaRect(const TVector2<Type>* convexHullVertices, const size_t count, TVector2<Type>& center, TVector2<Type> orientation[2]);
    }
    
    //----------------------------------------------------------------------------------------------------
    //		NOTES:
    //      pg 101 of "Real Time Collision Detection".
    //		
    ///		@brief : An Oriented Bounding Box (OBB) is a rectangular block, much like an AABB (TBox2), but with an
    ///              arbitrary Orientation.
    //----------------------------------------------------------------------------------------------------
    template <FloatingPointType Type>
    struct TOrientedBox2
    {
        TMatrix2x2<Type> m_orientation;  // Describes the orientation of the Box. 
        TVector2<Type>   m_center;       // Box's Center.
        TVector2<Type>   m_extents;      // Positive half-width extents of the OBB along each axis.

        constexpr TOrientedBox2();
        constexpr TOrientedBox2(const TMatrix2x2<Type>& orientation, const TVector2<Type>& center, const TVector2<Type>& extents);

        constexpr TVector2<Type> ClosestPointToPoint(const TVector2<Type>& queryPoint) const;
        Type DistanceToPoint(const TVector2<Type>& queryPoint) const;
        constexpr Type SquaredDistanceToPoint(const TVector2<Type>& queryPoint) const;

        bool Intersects(const TOrientedBox2& other) const;
    };

    //----------------------------------------------------------------------------------------------------
    //		NOTES:
    //      pg 101 of "Real Time Collision Detection".
    //      - An option to save on storage. Instead of storing the full matrix representation, store two
    //        of the axes and compute the third with the cross product.
    //      - Unreal stores the OBB as a Quaternion Orientation, Vector position and vector extents. The book argues
    //        that storing the orientation as a Quaternion is expensive when doing intersection tests, because you
    //        have to convert to the matrix orientation anyway.
    //		
    ///		@brief : An Oriented Bounding Box (OBB) is a rectangular block, much like an AABB (TBox3), but with an
    ///              arbitrary Orientation.
    //----------------------------------------------------------------------------------------------------
    template <FloatingPointType Type>
    struct TOrientedBox3
    {
        TMatrix3x3<Type> m_orientation; // Describes the orientation of the Box. 
        TVector3<Type>   m_center;      // Box's Center.
        TVector3<Type>   m_halfExtents;     // Positive half-width extents of the OBB along each axis.

        constexpr TOrientedBox3();
        constexpr TOrientedBox3(const TMatrix3x3<Type>& orientation, const TVector3<Type>& center, const TVector3<Type>& extents);
        constexpr TOrientedBox3(const TMatrix4x4<Type>& orientation, const AABox& box);
        constexpr TOrientedBox3(const TMatrix4x4<Type>& orientation, const TVector3<Type>& halfExtents);

        constexpr TVector3<Type> ClosestPointToPoint(const TVector3<Type>& queryPoint) const;
        Type DistanceToPoint(const TVector3<Type>& queryPoint) const;
        constexpr Type SquaredDistanceToPoint(const TVector3<Type>& queryPoint) const;
        
        bool Intersects(const TOrientedBox3& other) const;
        bool Intersects(const AABox& other, float epsilon = 1.0e-6f) const;
    };

    using OBB2f = TOrientedBox2<float>;
    using OBB2d = TOrientedBox2<double>;
    using OBB2D = TOrientedBox2<NES_PRECISION_TYPE>;

    using OBB3f = TOrientedBox3<float>;
    using OBB3d = TOrientedBox3<double>;
    using OBB = TOrientedBox3<NES_PRECISION_TYPE>;
    using OrientedBox = OBB;
}

namespace nes
{
    //----------------------------------------------------------------------------------------------------
    //		NOTES:
    //      O(n^2).
    //		
    ///		@brief : Computes the center point, axis orientation, of the minimum area rectangle in the xy plane
    ///             containing the convex hull.
    ///		@param convexHullVertices : Array of vertices to encompass
    ///		@param count : The number of points in the array.
    ///		@param center : Resulting center of the minimum Rect.
    ///		@param orientation : The X and Y axes of the Rect. 
    ///		@returns : Minimum Area of the Rect.
    //----------------------------------------------------------------------------------------------------
    template <FloatingPointType Type>
    Type math::ComputeMinAreaRect(const TVector2<Type>* convexHullVertices, const size_t count, TVector2<Type>& center,
        TVector2<Type> orientation[2])
    {
        Type minArea = std::numeric_limits<Type>::max();

        // Loop through all edges; j trails i by 1, module count
        for (size_t i = 0, j = count - 1; i < count; j = i, ++i)
        {
            // Get the current edge, and normalize it.
            TVector2<Type> edge = (convexHullVertices[i] - convexHullVertices[j]).Normalized();

            // Get the perpendicular axis to that edge
            TVector2<Type> perp = TVector2<Type>::PerpendicularTo(edge);

            // Look through all points to get the maximum extents:
            Type min0{};
            Type min1{};
            Type max0{};
            Type max1{};
            for (size_t k = 0; k < count; ++k)
            {
                // Project points onto axes "edge" and "perp" and keep track of the minimum
                // and maximum values along both axes.
                TVector2<Type> d = convexHullVertices[k] - convexHullVertices[j];
                float dot = TVector2<Type>::Dot(d, edge);
                min0 = math::Min(dot, min0);
                max0 = math::Max(dot, max0);

                dot = TVector2<Type>::Dot(d, perp);
                min1 = math::Min(dot, min1);
                max1 = math::Max(dot, max1);
            }
            
            Type area = (max0 - min0) * (max1 - min1);

            // If the best so far, remember area, center and axes
            if (area < minArea)
            {
                minArea = area;
                center = convexHullVertices[j] + static_cast<Type>(0.5f) * ((min0 + max0) * edge + (min1 + max1) * perp);
                orientation[0] = edge;
                orientation[1] = perp;
            }
        }
        
        return minArea;
    }
    
    //----------------------------------------------------------------------------------------------------
    ///		@brief : The default constructor creates a unit box around the origin that is aligned with the
    ///         XY axes.
    //----------------------------------------------------------------------------------------------------
    template <FloatingPointType Type>
    constexpr TOrientedBox2<Type>::TOrientedBox2()
        : m_orientation()
        , m_center()
        , m_extents(TVector2<Type>::Unit())
    {
        //
    }
    
    template <FloatingPointType Type>
    constexpr TOrientedBox2<Type>::TOrientedBox2(const TMatrix2x2<Type>& orientation, const TVector2<Type>& center,
        const TVector2<Type>& extents)
        : m_orientation(orientation)
        , m_center(center)
        , m_extents(extents)
    {
        //
    }

    //----------------------------------------------------------------------------------------------------
    ///		@brief : Returns the closest point on or in the oriented box to the query point. 
    //----------------------------------------------------------------------------------------------------
    template <FloatingPointType Type>
    constexpr TVector2<Type> TOrientedBox2<Type>::ClosestPointToPoint(const TVector2<Type>& queryPoint) const
    {
        const TVector2<Type> toPoint = (queryPoint - m_center);

        // Start with the center, and make steps to the border from there.
        TVector2<Type> result = m_center;

        // For each Oriented Axis...
        for (int i = 0; i < 2; ++i)
        {
            // ...project the toPoint vector onto that axis to get the
            // distance along the axis of toPoint from the center.
            const TVector2<Type> axis = m_orientation[i];
            Type distance = TVector2<Type>::Dot(toPoint, axis); 

            // Clamp the distance to the extents
            distance = math::Clamp(distance, -m_extents[i], m_extents[i]);

            // Move the distance on that axis to get the final coordinate.  
            result += distance * axis; 
        }

        return result;
    }

    //----------------------------------------------------------------------------------------------------
    ///		@brief : Returns the distance from the query point to the closest point on the oriented box. 
    //----------------------------------------------------------------------------------------------------
    template <FloatingPointType Type>
    Type TOrientedBox2<Type>::DistanceToPoint(const TVector2<Type>& queryPoint) const
    {
        return std::sqrt(SquaredDistanceToPoint(queryPoint));
    }

    //----------------------------------------------------------------------------------------------------
    ///		@brief : Returns the squared distance from the query point to the closest point on the oriented box. 
    //----------------------------------------------------------------------------------------------------
    template <FloatingPointType Type>
    constexpr Type TOrientedBox2<Type>::SquaredDistanceToPoint(const TVector2<Type>& queryPoint) const
    {
        const TVector2<Type> closestPoint = ClosestPointToPoint(queryPoint);
        return (queryPoint - closestPoint).SquaredMagnitude();
    }

    //----------------------------------------------------------------------------------------------------
    //		NOTES:
    //      This is a "separating axis test". Two OBBs are separated if, with respect to some axis L, the sum
    //      of their projected radii is less than the distance between the projection of their center points.
    //		
    ///		@brief : Returns true if the two Oriented Boxes intersect.
    //----------------------------------------------------------------------------------------------------
    template <FloatingPointType Type>
    bool TOrientedBox2<Type>::Intersects(const TOrientedBox2& other) const
    {
        // Compute the Rotation Matrix expressing "other" in this Box's coordinate frame.
        TMatrix2x2<Type> orientation{};
        TMatrix2x2<Type> orientationAbs{};
        
        for (int i = 0; i < 2; ++i)
        {
            for (int j = 0; j < 2; ++j)
            {
                orientation[i][j] = TVector2<Type>::Dot(m_orientation.GetRow(i), other.m_orientation.GetColumn(j));

                // Compute common subexpressions. Add in an epsilon term to counteract arithmetic errors when two edges are
                // parallel and their cross product is (near) null.
                orientationAbs[i][j] = math::Abs(orientation[i][j]) + math::PrecisionDelta<Type>();  
            }
        }

        // Compute the translation vector
        TVector2<Type> translation = other.m_center - m_center;

        // Bring the translation into this coordinate frame:
        translation = TVector2<Type>(
            TVector2<Type>::Dot(translation, m_orientation.GetRow(0)),
            TVector2<Type>::Dot(translation, m_orientation.GetRow(1)));

        float radiusA;
        float radiusB;

        // Test to find a separating Axis "L". "R" == m_localOrientation matrix, "R[0]" == X Axis of the local orientation.
        // Test L = R[0]
        // Test L = R[1]
        for (int i = 0; i < 2; ++i)
        {
            radiusA = m_extents[i];
            radiusB = (other.m_extents[0] * orientationAbs[i][0])
                    + (other.m_extents[1] * orientationAbs[i][1]);

            if (math::Abs(translation[i]) > radiusA + radiusB)
                return false;
        }
        
        // Test L = other.R[0]
        // Test L = other.R[1]
        for (int i = 0; i < 2; ++i)
        {
            radiusA = (m_extents[0] * orientationAbs[i][0])
                    + (m_extents[1] * orientationAbs[i][1]);
            radiusB = other.m_extents[i];

            if (math::Abs((translation[0] * orientation[0][i]) + (translation[1] * orientation[1][i])) > radiusA + radiusB)
                return false;
        }

        // [TODO]: 
        // Is this everything???

        // There is no separating axis, so they must be intersecting.
        return true;
    }

    //----------------------------------------------------------------------------------------------------
    ///		@brief : The default constructor creates a unit box around the origin that is aligned with the
    ///         XY axes.
    //----------------------------------------------------------------------------------------------------
    template <FloatingPointType Type>
    constexpr TOrientedBox3<Type>::TOrientedBox3()
        : m_orientation()
        , m_center()
        , m_halfExtents(TVector3<Type>::Unit())
    {
        //
    }

    template <FloatingPointType Type>
    constexpr TOrientedBox3<Type>::TOrientedBox3(const TMatrix3x3<Type>& orientation, const TVector3<Type>& center,
        const TVector3<Type>& extents)
        : m_orientation(orientation)
        , m_center(center)
        , m_halfExtents(extents)
    {
        //
    }

    template <FloatingPointType Type>
    constexpr TOrientedBox3<Type>::TOrientedBox3(const TMatrix4x4<Type>& orientation, const AABox& box)
    {
        m_orientation = math::ToMat3(orientation.PreTranslated(box.Center()));
        m_center = box.Center();
        m_halfExtents = box.GetExtent();
    }

    template <FloatingPointType Type>
    constexpr TOrientedBox3<Type>::TOrientedBox3(const TMatrix4x4<Type>& orientation, const TVector3<Type>& halfExtents)
    {
        m_orientation = math::ToMat3(orientation);
        m_center = orientation.GetTranslation();
        m_halfExtents = halfExtents;
    }

    //----------------------------------------------------------------------------------------------------
    ///		@brief : Returns the closest point in or on the oriented box to the query point.
    //----------------------------------------------------------------------------------------------------
    template <FloatingPointType Type>
    constexpr TVector3<Type> TOrientedBox3<Type>::ClosestPointToPoint(const TVector3<Type>& queryPoint) const
    {
        const TVector3<Type> toPoint = (queryPoint - m_center);

        // Start with the center, and make steps to the border from there.
        TVector3<Type> result = m_center;

        // For each Oriented Axis...
        for (int i = 0; i < 3; ++i)
        {
            // ...project the toPoint vector onto that axis to get the
            // distance along the axis of toPoint from the center.
            const TVector3<Type> axis = m_orientation.GetAxis(i);
            Type distance = TVector3<Type>::Dot(toPoint, axis); 

            // Clamp the distance to the extents
            distance = math::Clamp(distance, -m_halfExtents[i], m_halfExtents[i]);

            // Move the distance on that axis to get the final coordinate.  
            result += distance * axis; 
        }

        return result;
    }

    //----------------------------------------------------------------------------------------------------
    ///		@brief : Returns the distance from the query point to the closest point on the oriented box.
    //----------------------------------------------------------------------------------------------------
    template <FloatingPointType Type>
    Type TOrientedBox3<Type>::DistanceToPoint(const TVector3<Type>& queryPoint) const
    {
        return std::sqrt(SquaredDistanceToPoint(queryPoint));
    }

    //----------------------------------------------------------------------------------------------------
    ///		@brief : Returns the squared distance from the query point to the closest point on the oriented box.
    //----------------------------------------------------------------------------------------------------
    template <FloatingPointType Type>
    constexpr Type TOrientedBox3<Type>::SquaredDistanceToPoint(const TVector3<Type>& queryPoint) const
    {
        const TVector3<Type> closestPoint = ClosestPointToPoint(queryPoint);
        return (queryPoint - closestPoint).SquaredMagnitude();
    }

    //----------------------------------------------------------------------------------------------------
    //		NOTES:
    //      This is a "separating axis test". Two OBBs are separated if, with respect to some axis L, the sum
    //      of their projected radii is less than the distance between the projection of their center points.
    //		
    ///		@brief : Returns true if the two Oriented Boxes intersect.
    //----------------------------------------------------------------------------------------------------
    template <FloatingPointType Type>
    bool TOrientedBox3<Type>::Intersects(const TOrientedBox3& other) const
    {
        // Compute the Rotation Matrix expressing "other" in this Box's coordinate frame.
        TMatrix3x3<Type> orientation{};
        TMatrix3x3<Type> orientationAbs{};
        
        for (int i = 0; i < 3; ++i)
        {
            for (int j = 0; j < 3; ++j)
            {
                orientation[i][j] = TVector3<Type>::Dot(m_orientation.GetRow(i), other.m_orientation.GetColumn(j));

                // Compute common subexpressions. Add in an epsilon term to counteract arithmetic errors when two edges are
                // parallel and their cross product is (near) null.
                orientationAbs[i][j] = math::Abs(orientation[i][j]) + math::PrecisionDelta<Type>();  
            }
        }

        // Compute the translation vector
        TVector3<Type> translation = other.m_center - m_center;

        // Bring the translation into this coordinate frame:
        translation = TVector3<Type>(
            TVector3<Type>::Dot(translation, m_orientation.GetRow(0)),
            TVector3<Type>::Dot(translation, m_orientation.GetRow(1)),
            TVector3<Type>::Dot(translation, m_orientation.GetRow(2)));

        float radiusA;
        float radiusB;

        // Test to find a separating axis:
        // Test Axes:
        // L = this.m_localOrientation[0],
        // L = this.m_localOrientation[1],
        // L = this.m_localOrientation[2] 
        for (int i = 0; i < 3; ++i)
        {
            radiusA = m_halfExtents[i];
            radiusB = (other.m_halfExtents[0] * orientationAbs[i][0])
                    + (other.m_halfExtents[1] * orientationAbs[i][1])
                    + (other.m_halfExtents[2] * orientationAbs[i][2]);

            // There *is* a separating Axis:
            if (math::Abs(translation[i]) > radiusA + radiusB)
                return false;
        }

        // Test Axes:
        // L = other.m_localOrientation[0],
        // L = other.m_localOrientation[1],
        // L = other.m_localOrientation[2]
        for (int i = 0; i < 3; ++i)
        {
            radiusA = (m_halfExtents[0] * orientationAbs[i][0])
                    + (m_halfExtents[1] * orientationAbs[i][1])
                    + (m_halfExtents[2] * orientationAbs[i][2]);
            radiusB = other.m_halfExtents[i];

            // There *is* a separating Axis:
            if (math::Abs((translation[0] * orientation[0][i]) + (translation[1] * orientation[1][i]) + (translation[2] * orientation[2][i])) > radiusA + radiusB)
                return false;
        }

        // Test Axis: L = R[0] x other.R[0]
        radiusA =       m_halfExtents[1] * orientationAbs[2][0] +       m_halfExtents[2] * orientationAbs[1][0];
        radiusB = other.m_halfExtents[1] * orientationAbs[0][2] + other.m_halfExtents[2] * orientationAbs[0][1];
        if (math::Abs(translation[2] * orientation[1][0] - translation[1] * orientation[2][0]) > radiusA + radiusB)
            return false;

        // Test Axis: L = R[0] x other.R[1]
        radiusA =       m_halfExtents[1] * orientationAbs[2][1] +       m_halfExtents[2] * orientationAbs[1][1];
        radiusB = other.m_halfExtents[0] * orientationAbs[0][2] + other.m_halfExtents[2] * orientationAbs[0][0];
        if (math::Abs(translation[2] * orientation[1][1] - translation[1] * orientation[2][1]) > radiusA + radiusB)
            return false;

        // Test Axis: L = R[0] x other.R[2]
        radiusA =       m_halfExtents[1] * orientationAbs[2][2] +       m_halfExtents[2] * orientationAbs[1][2];
        radiusB = other.m_halfExtents[0] * orientationAbs[0][1] + other.m_halfExtents[1] * orientationAbs[0][0];
        if (math::Abs(translation[2] * orientation[1][2] - translation[1] * orientation[2][2]) > radiusA + radiusB)
            return false;

        // Test Axis: L = R[1] x other.R[0]
        radiusA =       m_halfExtents[0] * orientationAbs[2][0] +       m_halfExtents[2] * orientationAbs[0][0];
        radiusB = other.m_halfExtents[1] * orientationAbs[1][2] + other.m_halfExtents[2] * orientationAbs[1][1];
        if (math::Abs(translation[0] * orientation[2][0] - translation[2] * orientation[0][0]) > radiusA + radiusB)
            return false;

        // Test Axis: L = R[1] x other.R[1]
        radiusA =       m_halfExtents[0] * orientationAbs[2][1] +       m_halfExtents[2] * orientationAbs[0][1];
        radiusB = other.m_halfExtents[0] * orientationAbs[1][2] + other.m_halfExtents[2] * orientationAbs[1][0];
        if (math::Abs(translation[0] * orientation[2][1] - translation[2] * orientation[0][1]) > radiusA + radiusB)
            return false;

        // Test Axis: L = R[1] x other.R[2]
        radiusA =       m_halfExtents[0] * orientationAbs[2][2] +       m_halfExtents[2] * orientationAbs[0][2];
        radiusB = other.m_halfExtents[0] * orientationAbs[1][1] + other.m_halfExtents[1] * orientationAbs[1][0];
        if (math::Abs(translation[0] * orientation[2][1] - translation[2] * orientation[0][1]) > radiusA + radiusB)
            return false;

        // Test Axis: L = R[2] x other.R[0]
        radiusA =       m_halfExtents[0] * orientationAbs[1][0] +       m_halfExtents[1] * orientationAbs[0][0];
        radiusB = other.m_halfExtents[1] * orientationAbs[2][2] + other.m_halfExtents[2] * orientationAbs[2][1];
        if (math::Abs(translation[1] * orientation[0][0] - translation[0] * orientation[1][0]) > radiusA + radiusB)
            return false;

        // Test Axis: L = R[2] x other.R[1]
        radiusA =       m_halfExtents[0] * orientationAbs[1][1] +       m_halfExtents[1] * orientationAbs[0][1];
        radiusB = other.m_halfExtents[0] * orientationAbs[2][2] + other.m_halfExtents[2] * orientationAbs[2][0];
        if (math::Abs(translation[1] * orientation[0][1] - translation[0] * orientation[1][1]) > radiusA + radiusB)
            return false;

        // Test Axis: L = R[2] x other.R[2]
        radiusA =       m_halfExtents[0] * orientationAbs[1][2] +       m_halfExtents[1] * orientationAbs[0][2];
        radiusB = other.m_halfExtents[0] * orientationAbs[2][1] + other.m_halfExtents[1] * orientationAbs[2][0];
        if (math::Abs(translation[1] * orientation[0][2] - translation[0] * orientation[1][2]) > radiusA + radiusB)
            return false;

        // There is no separating Axis found, so the OBBs must be intersecting
        return true;
    }

    template <FloatingPointType Type>
    bool TOrientedBox3<Type>::Intersects(const AABox& other, float epsilon) const
    {
        // Taken from: Real Time Collision Detection - Christer Ericson
        // Chapter 4.4.1, page 103-105.
        // Note that the code is swapped around: A is the AABB and B is the oriented box (this saves us from having to invert the orientation of the oriented box)

        // Convert AABox to center / extent representation
        Vector3 aCenter = other.Center();
        Vector3 aHalfExtents = other.GetExtent();

        // Get the rotation matrix expressing B in A's coordinate frame
        Mat4 rotation(m_orientation[0], m_orientation[1], m_orientation[2], m_center - aCenter);

        // Compute common subexpressions. Add in an epsilon term to
        // counteract arithmetic errors when tow edges are parallel and
        // their cross product is (near) null.
        Vector3 vEpsilon = Vector3::Replicate(epsilon);

        Vector3 absRot[3]
        {
            rotation.GetAxis(Axis::X).Abs() + vEpsilon,
            rotation.GetAxis(Axis::Y).Abs() + vEpsilon,
            rotation.GetAxis(Axis::Z).Abs() + vEpsilon,
        };

        // Test axes L = A0, L = A1, L = A2
        float radiusA, radiusB;
        
        // Test to find a separating axis:
        // Test axes L = A0, L = A1, L = A2
        for (int i = 0; i < 3; ++i)
        {
            radiusA = m_halfExtents[i];
            radiusB = (m_halfExtents[0] * absRot[0][i])
                    + (m_halfExtents[1] * absRot[1][i])
                    + (m_halfExtents[2] * absRot[2][i]);

            // There *is* a separating Axis:
            if (math::Abs(rotation[3][i]) > radiusA + radiusB)
                return false;
        }

        // Test axes L = B0, L = B1, L = B2
        for (int i = 0; i < 3; ++i)
        {
            radiusA = aHalfExtents[i];
            radiusB = (m_halfExtents[0] * absRot[0][i])
                    + (m_halfExtents[1] * absRot[1][i])
                    + (m_halfExtents[2] * absRot[2][i]);

            // There *is* a separating Axis:
            if (math::Abs(rotation[3][i]) > radiusA + radiusB)
                return false;
        }

        // Test axis L = A0 x B0
        radiusA = aHalfExtents[1] * absRot[0][2] + aHalfExtents[2] * absRot[0][1];
        radiusB = m_halfExtents[1] * absRot[2][0] + m_halfExtents[2] * absRot[1][0];
        if (math::Abs(rotation[3][2] * rotation[0][1] - rotation[3][1] * rotation[0][2]) > radiusA + radiusB)
            return false;

        // Test axis L = A0 x B1
        radiusA = aHalfExtents[1] * absRot[1][2] + aHalfExtents[2] * absRot[1][1];
        radiusB = m_halfExtents[0] * absRot[2][0] + m_halfExtents[2] * absRot[0][0];
        if (math::Abs(rotation[3][2] * rotation[1][1] - rotation[3][1] * rotation[1][2]) > radiusA + radiusB)
            return false;

        // Test axis L = A0 x B2
        radiusA = aHalfExtents[1] * absRot[2][2] + aHalfExtents[2] * absRot[2][1];
        radiusB = m_halfExtents[0] * absRot[1][0] + m_halfExtents[1] * absRot[0][0];
        if (math::Abs(rotation[3][2] * rotation[2][1] - rotation[3][1] * rotation[2][2]) > radiusA + radiusB)
            return false;

        // Test axis L = A1 x B0
        radiusA = aHalfExtents[0] * absRot[0][2] + aHalfExtents[2] * absRot[0][0];
        radiusB = m_halfExtents[1] * absRot[2][1] + m_halfExtents[2] * absRot[1][1];
        if (math::Abs(rotation[0][3] * rotation[0][2] - rotation[3][2] * rotation[0][0]) > radiusA + radiusB)
            return false;
        
        // Test axis L = A1 x B1
        radiusA = aHalfExtents[0] * absRot[1][2] + aHalfExtents[2] * absRot[1][0];
        radiusB = m_halfExtents[0] * absRot[2][1] + m_halfExtents[2] * absRot[0][1];
        if (math::Abs(rotation[3][0] * rotation[1][2] - rotation[3][2] * rotation[1][0]) > radiusA + radiusB)
            return false;

        // Test axis L = A1 x B2
        radiusA = aHalfExtents[0] * absRot[2][2] + aHalfExtents[2] * absRot[2][0];
        radiusB = m_halfExtents[0] * absRot[1][1] + m_halfExtents[1] * absRot[0][1];
        if (math::Abs(rotation[3][0] * rotation[2][2] - rotation[3][2] * rotation[2][0]) > radiusA + radiusB)
            return false;

        // Test axis L = A2 x B0
        radiusA = aHalfExtents[0] * absRot[0][1] + aHalfExtents[1] * absRot[0][0];
        radiusB = m_halfExtents[1] * absRot[2][2] + m_halfExtents[2] * absRot[1][2];
        if (math::Abs(rotation[3][1] * rotation[0][0] - rotation[3][0] * rotation[0][1]) > radiusA + radiusB)
            return false;

        // Test axis L = A2 x B1
        radiusA = aHalfExtents[0] * absRot[1][1] + aHalfExtents[1] * absRot[1][0];
        radiusB = m_halfExtents[0] * absRot[2][2] + m_halfExtents[2] * absRot[0][2];
        if (math::Abs(rotation[3][1] * rotation[1][0] - rotation[3][0] * rotation[1][1]) > radiusA + radiusB)
            return false;

        // Test axis L = A2 x B2
        radiusA = aHalfExtents[0] * absRot[2][1] + aHalfExtents[1] * absRot[2][0];
        radiusB = m_halfExtents[0] * absRot[1][2] + m_halfExtents[1] * absRot[0][2];
        if (math::Abs(rotation[3][1] * rotation[2][0] - rotation[3][0] * rotation[2][1]) > radiusA + radiusB)
            return false;

        // Since no separating axis is found, the OBB and AAB must be intersecting
        return true;
    }
}
