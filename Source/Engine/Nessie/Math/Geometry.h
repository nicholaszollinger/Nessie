// Geometry.h
#pragma once
#include "Matrix.h"
#include "Vector3.h"

namespace nes
{
    //----------------------------------------------------------------------------------------------------
    //		NOTES:
    //      pg 33 of "Real-Time Collision Detection"
    //		
    ///		@brief : The value returned corresponds to siz times the *signed* volume of the tetrahedron formed
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
        
        const TSquareMatrix<3, Type> mat(elements);
        return mat.CalculateDeterminant();
    }

    // //----------------------------------------------------------------------------------------------------
    // //		NOTES:
    // //       I put this on pause, because it needs a 5x4 matrix which I currently don't support...  
    // //		
    // ///		@brief : 
    // ///		@returns : 
    // //----------------------------------------------------------------------------------------------------
    // template <FloatingPointType Type>
    // Type InSphere3D(const Vector3<Type>& a, const Vector3<Type>& b, const Vector3<Type>& c, const Vector3<Type>& d,
    //     const Vector3<Type>& e)
    // {
    //     const auto aDiff = a - e;
    //     const auto bDiff = b - e;
    //     const auto cDiff = c - e;
    //     const auto dDiff = d - e;
    //     
    //     const Type elements[9] =
    //         {
    //         aDiff.x, aDiff.y, aDiff.SquaredMagnitude(),
    //         bDiff.x, bDiff.y, bDiff.SquaredMagnitude(),
    //         cDiff.x, cDiff.y, cDiff.SquaredMagnitude(),
    //     };
    //
    //     const SquareMatrix<3, Type> mat(elements);
    //     const Type determinant = mat.CalculateDeterminant();
    // }
}
