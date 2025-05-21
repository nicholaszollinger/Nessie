// VectorRegister.h
#pragma once
#include "MathConfig.h"
#include "SIMD/VectorRegisterF.h"

namespace nes
{
    //----------------------------------------------------------------------------------------------------
    // [TODO]: Allow support for Double Precision
    //		
    /// @brief : Register class for operating on 4 floating-point values in single instruction, if supported
    ///     SIMD instruction sets are available.
    //----------------------------------------------------------------------------------------------------
    using VectorRegister = NES_IF_SINGLE_PRECISION_ELSE(nes::VectorRegisterF, void);
    static_assert(!std::same_as<VectorRegister, void>, "Invalid Vector Register Type!!!");
}