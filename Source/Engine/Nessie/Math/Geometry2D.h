// Geometry2D.h
#pragma once
#include "Matrix.h"
#include "Vector2.h"

namespace nes
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
        
        const TSquareMatrix<2, Type> mat(elements);
        return mat.CalculateDeterminant();
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

        const TSquareMatrix<3, Type> mat(elements);
        const Type determinant = mat.CalculateDeterminant();

        // If the determinant is 0, then the four points are co-circular.
        if (math::CheckEqualFloats<Type>(determinant, 0))
        {
            return 0.0;
        }

        const Type isCounterClockwise = Orient2D(a, b, c);
        return determinant * isCounterClockwise;
    }
}

namespace nes
{
    
}