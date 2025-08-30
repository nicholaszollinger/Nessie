// Atomics.h
#pragma once
#include <atomic>
#include "Nessie/Core/Concepts.h"

namespace nes
{
    //----------------------------------------------------------------------------------------------------
    /// @brief : Atomically computes the min(atomic, value) and stores it in the atomic. Returns true if the
    ///     value was actually updated.
    ///	@param atomic : Atomic value that will attempt to be updated.
    ///	@param value : Value to compare and potentially store.
    //----------------------------------------------------------------------------------------------------
    template <ScalarType Type>
    bool AtomicMin(std::atomic<Type>& atomic, const Type value, const std::memory_order memOrder = std::memory_order::seq_cst)
    {
        Type currentValue = atomic.load(std::memory_order_relaxed);
        while (currentValue > value)
        {
            if (atomic.compare_exchange_weak(currentValue, value, memOrder))
                return true;
        }

        return false;
    }

    //----------------------------------------------------------------------------------------------------
    /// @brief : Atomically computes the max(atomic, value) and stores it in the atomic. Returns true if the
    ///     value was actually updated.
    ///	@param atomic : Atomic value that will attempt to be updated.
    ///	@param value : Value to compare and potentially store.
    //----------------------------------------------------------------------------------------------------
    template <ScalarType Type>
    bool AtomicMax(std::atomic<Type>& atomic, const Type value, const std::memory_order memOrder = std::memory_order::seq_cst)
    {
        Type currentValue = atomic.load(std::memory_order_relaxed);
        while (currentValue < value)
        {
            if (atomic.compare_exchange_weak(currentValue, value, memOrder))
                return true;
        }

        return false;
    }
}