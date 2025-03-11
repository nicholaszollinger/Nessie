// Matrix.h
#pragma once
#include "Vector3.h"
#include "Detail/TMatrix4x4.h"

namespace nes
{
    using Matrix2x2f = TMatrix2x2<float>;
    using Matrix2x2d = TMatrix2x2<double>;
    using Matrix2x2 = TMatrix2x2<NES_MATH_DEFAULT_REAL_TYPE>;
    using Mat2 = Matrix2x2;
    
    using Matrix3x3f = TMatrix3x3<float>;
    using Matrix3x3d = TMatrix3x3<double>;
    using Matrix3x3 = TMatrix3x3<NES_MATH_DEFAULT_REAL_TYPE>;
    using Mat3 = Matrix3x3;
    
    using Matrix4x4f = TMatrix4x4<float>;
    using Matrix4x4d = TMatrix4x4<double>;
    using Matrix4x4 = TMatrix4x4<NES_MATH_DEFAULT_REAL_TYPE>;
    using Mat4 = Matrix4x4;
}
