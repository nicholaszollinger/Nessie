// MathTypes.h
// Forward declarations of Math Types.
#pragma once
#include "MathConfig.h"
#include "Nessie/Core/Concepts.h"

namespace nes
{
    class   Vec2;
    class   Vec3;
    class   Vec4;
    struct  Quat;
    struct  Rotation;
    class   Mat44;

    template <IntegralType Type> struct TIntVec2;
    using   IVec2 = TIntVec2<int>;
    using   UVec2 = TIntVec2<uint32>;

    template <IntegralType Type> struct TIntVec3;
    using   IVec3 = TIntVec3<int>;
    using   UVec3 = TIntVec3<uint32>;
    
    template <IntegralType Type> struct TIntVec4;
    using   IVec4 = TIntVec4<int>;
    using   UVec4 = TIntVec4<uint32>;

    struct  Vec4Reg;
    struct  UVec4Reg;
}