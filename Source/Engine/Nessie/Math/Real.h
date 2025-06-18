// Real.h
#pragma once
#include "MathTypes.h"
#include "Scalar2.h"
#include "Scalar3.h"
#include "Scalar4.h"

/// Defines Real types based on the current precision level (a Real is either float or double).  

namespace nes
{
#ifdef NES_DOUBLE_PRECISION
    using Real      = double;
    using Real2     = Double2;
    using Real3     = Double3;
    using Real4     = Double4;
    using RVec3     = DVec3;
    using RMat44    = DMat44;
    
    #define NES_RVECTOR_ALIGNMENT NES_DVECTOR_ALIGNMENT
    
#else
    using Real      = float;
    using Real2     = Float2;
    using Real3     = Float3;
    using Real4     = Float4;
    using RVec3     = Vec3;
    using RMat44    = Mat44;
    
    //----------------------------------------------------------------------------------------------------
    /// @brief : Vector alignment value based on the Real type (float or double). 
    //----------------------------------------------------------------------------------------------------
    #define NES_RVECTOR_ALIGNMENT NES_VECTOR_ALIGNMENT
    
#endif
}