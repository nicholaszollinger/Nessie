// VectorConversions.h
#pragma once
#include "Vector4.h"

namespace nes::math
{
    template <ScalarType Type>
    constexpr TVector2<Type> XY(const TVector3<Type>& vec)
    {
        return TVector2<Type>(vec.x, vec.y);
    }

    template <ScalarType Type>
    constexpr TVector3<Type> XYZ(const TVector4<Type>& vec)
    {
        return TVector3<Type>(vec.x, vec.y, vec.z);
    }
}