// FPControlWord.h
#pragma once
#include "Core/Config.h"

namespace nes
{
#if defined (NES_USE_SSE)
    //----------------------------------------------------------------------------------------------------
    /// @brief : Helper class that need to be put on the stack to update the state of the floating point
    ///     control work. This state is kept per thread.
    //----------------------------------------------------------------------------------------------------
    template <unsigned Value, unsigned Mask>
    class FPControlWord
    {
        unsigned m_previousState;
    public:
        FPControlWord()
        {
            m_previousState = _mm_getcsr();
            _mm_setcsr((m_previousState & ~Mask) | Value);
        }

        ~FPControlWord()
        {
            _mm_setcsr((_mm_getcsr() & ~Mask) | (m_previousState & Mask));
        }
    };
#else
#error "Unsupported CPU Architecture!"
#endif
}