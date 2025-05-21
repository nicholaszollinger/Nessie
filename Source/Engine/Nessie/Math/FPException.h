// FPException.h
#pragma once

#include "FPControlWord.h"

namespace nes
{
#if defined(NES_USE_SSE)
    //----------------------------------------------------------------------------------------------------
    /// @brief : Enable floating point divide by zero exception, overflow exceptions and exceptions on invalid numbers
    //----------------------------------------------------------------------------------------------------
    class FPExceptionsEnable : public FPControlWord<0, _MM_MASK_DIV_ZERO | _MM_MASK_INVALID | _MM_MASK_OVERFLOW> { };

    //----------------------------------------------------------------------------------------------------
    /// @brief : Disable invalid floating point value exceptions
    //----------------------------------------------------------------------------------------------------
    class FPExceptionDisableInvalid : public FPControlWord<_MM_MASK_INVALID, _MM_MASK_INVALID> { };

    //----------------------------------------------------------------------------------------------------
    /// @brief : Disable division by zero floating point exceptions
    //----------------------------------------------------------------------------------------------------
    class FPExceptionDisableDivideByZero : public FPControlWord<_MM_MASK_DIV_ZERO, _MM_MASK_DIV_ZERO> { };

    //----------------------------------------------------------------------------------------------------
    /// @brief : Disable floating point overflow exceptions
    //----------------------------------------------------------------------------------------------------
    class FPExceptionDisableOverflow : public FPControlWord<_MM_MASK_OVERFLOW, _MM_MASK_OVERFLOW> { };
#else
    class FPExceptionsEnable { };
    class FPExceptionDisableInvalid { };
    class FPExceptionDisableDivByZero { };
    class FPExceptionDisableOverflow { };
#endif
}