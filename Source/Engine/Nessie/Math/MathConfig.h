// MathConfig.h
#pragma once
#include "Core/Config.h"

/// Uncomment to set NES_PRECISION_TYPE to "double". This is used across all default math types.
/// Otherwise, the precision type is "float".
// #define NES_DOUBLE_PRECISION

#ifdef NES_DOUBLE_PRECISION
    #define NES_IF_SINGLE_PRECISION(...)
    #define NES_IF_SINGLE_PRECISION_ELSE(singleArg, doubleArg) doubleArg
    #define NES_IF_DOUBLE_PRECISION(...) __VA_ARGS__
    #define NES_PRECISION_TYPE double

#else
    #define NES_IF_SINGLE_PRECISION(...) __VA_ARGS__
    #define NES_IF_SINGLE_PRECISION_ELSE(singleArg, doubleArg) singleArg
    #define NES_IF_DOUBLE_PRECISION(...)
    #define NES_PRECISION_TYPE float
#endif

#if defined(NES_COMPILER_MSVC)
    //----------------------------------------------------------------------------------------------------
    /// @brief : Macro to disable floating-point-contraction, which is an instruction such as
    ///     Fused-Multiply-Add (FMA) that combines two separate floating point instructions into a single
    ///     instruction. Use of these instructions can affect floating-point precision, because instead of
    ///     rounding after each operation, the processor may round only once after both operations.
    /// @note Must be bookended with NES_PRECISE_MATH_END for the end of the scope that you are using it.
    //----------------------------------------------------------------------------------------------------
    #define NES_PRECISE_MATH_BEGIN                 \
        __pragma(float_control(precise, on, push)) \
        __pragma(fp_contract(off))

    //----------------------------------------------------------------------------------------------------
    /// @brief : Macro that re-enables floating-point-contraction, which is an instruction such as
    ///     Fused-Multiply-Add (FMA) that combines two separate floating point instructions into a single
    ///     instruction. Use of these instructions can affect floating-point precision, because instead of
    ///     rounding after each operation, the processor may round only once after both operations.
    /// @note Must be preceded with NES_PRECISE_MATH_BEGIN at the beginning of the scope that you are using it.
    //----------------------------------------------------------------------------------------------------
    #define NES_PRECISE_MATH_END        \
        __pragma(fp_contract(on))       \
        __pragma(float_control(pop))

#else
    #define NES_PRECISE_MATH_BEGIN
    #define NES_PRECISE_MATH_END
#endif

#ifdef NES_FLOATING_POINT_EXCEPTIONS_ENABLED
    #define NES_IF_FLOATING_POINT_EXCEPTIONS_ENABLED(...) __VA_ARGS__
#else
    #define NES_IF_FLOATING_POINT_EXCEPTIONS_ENABLED(...)
#endif

//----------------------------------------------------------------------------------------------------
/// @brief : If true, the near and far clip planes in view matrices will be clamped to [0, 1]. Otherwise,
///     they will be normalized to [-1, 1].
//----------------------------------------------------------------------------------------------------
#define NES_CLIP_VIEW_ZERO_TO_ONE 1