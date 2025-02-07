// Geometry.h
#pragma once
#include "Matrix.h"
#include "Vector3.h"

namespace nes::math
{
    //----------------------------------------------------------------------------------------------------
    //		NOTES:
    //      pg 32 of "Real-Time Collision Detection"
    //		
    ///		@brief : The value returned represents twice the *signed* area of the triangle ABC.
    ///              (positive if counterclockwise; negative if clockwise).
    ///              - When the result is greater than 0, then point C lies to the left of the directed line A->B.
    ///              Equivalently, the triangle ABC is oriented counterclockwise.
    ///              - When the result is less than 0, then point C lies to the right of the directed line A->B
    ///              and the triangle ABC is oriented clockwise.
    ///              - When the result is equal to 0, then the three points are collinear.
    //----------------------------------------------------------------------------------------------------
    template <FloatingPointType Type>
    Type Orient2D(const TVector2<Type>& a, const TVector2<Type>& b, const TVector2<Type>& c)
    {
        const Type elements[4] =
        {
            a.x - c.x, a.y - c.y,
            b.x - c.x, b.y - c.y
        };
        
        const TMatrix2x2<Type> mat(elements);
        const Type det = mat.Determinant();
        return det;
    }

    //----------------------------------------------------------------------------------------------------
    //		NOTES:
    //      pg 33 of "Real-Time Collision Detection"
    //		
    ///		@brief : The value returned corresponds to six times the *signed* volume of the tetrahedron formed
    ///              by the four points.
    ///              - When the result is less than 0, D lies above the supporting plane of triangle ABC, in
    ///              the sense that ABC appears counterclockwise when viewed from D.
    ///              - When the result is greater than 0, D lies below the plane of ABC.
    ///              - When the result is equal to 0, then all points are coplanar.
    //----------------------------------------------------------------------------------------------------
    template <FloatingPointType Type>
    Type Orient3D(const TVector3<Type>& a, const TVector3<Type>& b, const TVector3<Type>& c, const TVector3<Type>& d)
    {
        const Type elements[9] =
        {
            a.x - d.x, a.y - d.y, a.z - d.z,
            b.x - d.x, b.y - d.y, b.z - d.z,
            c.x - d.x, c.y - d.y, c.z - d.z
        };
        
        const TMatrix3x3<Type> mat(elements);
        return mat.Determinant();
    }
    
    //----------------------------------------------------------------------------------------------------
    //		NOTES:
    //      pg 34 of "Real-Time Collision Detection"
    //		
    ///		@brief : Determines if the point D is on, in, or outside the circle that goes through the triangle
    ///             ABC.
    ///             - If the result is <0, then point D is inside the Circle.
    ///             - If the result is ==0, then all points are lie on the bounds of the circle.
    ///             - If the result is >0, then point D is outside the Circle.
    //----------------------------------------------------------------------------------------------------
    template <FloatingPointType Type>
    Type InCircle2D(const TVector2<Type>& a, const TVector2<Type>& b, const TVector2<Type>& c, const TVector2<Type>& d)
    {
        const auto aDiff = a - d;
        const auto bDiff = b - d;
        const auto cDiff = c - d;
        
        const Type elements[9] =
        {
            aDiff.x, aDiff.y, aDiff.SquaredMagnitude(),
            bDiff.x, bDiff.y, bDiff.SquaredMagnitude(),
            cDiff.x, cDiff.y, cDiff.SquaredMagnitude(),
        };

        const TMatrix3x3<Type> mat(elements);
        const Type determinant = mat.Determinant();

        // If the determinant is 0, then the four points are co-circular.
        if (math::CheckEqualFloats<Type>(determinant, 0))
        {
            return 0.0;
        }

        const Type isCounterClockwise = Orient2D(a, b, c);
        return determinant * isCounterClockwise;
    }

    //----------------------------------------------------------------------------------------------------
    ///		@brief : Returns whether the 3 points a, b, and c are all collinear. 
    //----------------------------------------------------------------------------------------------------
    template <FloatingPointType Type>
    bool PointsAreCollinear(const TVector2<Type>& a, const TVector2<Type>& b, const TVector2<Type>& c)
    {
        return math::CheckEqualFloats(Orient2D(a, b, c),0);
    }

    //----------------------------------------------------------------------------------------------------
    ///		@brief : Returns whether the 3 points a, b, and c are all coplanar. 
    //----------------------------------------------------------------------------------------------------
    template <FloatingPointType Type>
    bool PointsAreCoplanar(const TVector3<Type>& a, const TVector3<Type>& b, const TVector3<Type>& c)
    {
        return math::CheckEqualFloats(Orient3D(a, b, c), 0);
    }
}
