// AllocationCallbacks.cpp
#include "AllocationCallbacks.h"

namespace nes
{
    static void* AlignedMalloc([[maybe_unused]] void* userArg, const size_t size, const size_t alignment)
    {
        return NES_ALIGNED_ALLOC(size, alignment);
    }

    static void* AlignedRealloc([[maybe_unused]] void* userArg, void* pMemory, const size_t size, const size_t alignment)
    {
        return NES_ALIGNED_REALLOC(pMemory, size, alignment);
    }

    static void AlignedFree([[maybe_unused]] void* userArg, void* pMemory)
    {
        NES_ALIGNED_FREE(pMemory);
    }

    static void* Malloc(void*, const size_t size, const size_t)
    {
        return NES_ALLOC(size);
    }

    static void* Realloc(void*, void* pMemory, const size_t size, const size_t)
    {
        return NES_REALLOC(pMemory, size);
    }

    static void Free(void*, void* pMemory)
    {
        NES_FREE(pMemory);
    }

    AllocationCallbacks::AllocationCallbacks()
        : m_alloc(AlignedMalloc)
        , m_free(AlignedFree)
        , m_realloc(AlignedRealloc)
        , m_pUserData(nullptr)
    {
        //
    }

    AllocationCallbacks::AllocationCallbacks(const AllocateFunction& alloc, const FreeFunction& free, const ReallocationFunction& realloc)
        : m_alloc(alloc)
        , m_free(free)
        , m_realloc(realloc)
        , m_pUserData(nullptr)
    {
        EnsureValidCallbacksOrReset();
    }

    void AllocationCallbacks::EnsureValidCallbacksOrReset()
    {
        // If any are invalid, set to default.
        if (m_alloc == nullptr || m_free == nullptr || m_realloc == nullptr)
        {
            *this = AllocationCallbacks();
        }
    }

    AllocationCallbacks AllocationCallbacks::GetDefaultCallbacks()
    {
        static AllocationCallbacks defaultCallbacks(nes::Malloc, nes::Free, nes::Realloc);
        return defaultCallbacks;
    }

    AllocationCallbacks AllocationCallbacks::GetDefaultAlignedCallbacks()
    {
        return {}; // Defaults to aligned allocations.
    }

    AllocationCallbacks& AllocationCallbacks::SetCallbacks(const AllocateFunction& alloc, const FreeFunction& free, const ReallocationFunction& realloc)
    {
        m_alloc = alloc;
        m_free = free;
        m_realloc = realloc;

        EnsureValidCallbacksOrReset();
        return *this;
    }

    AllocationCallbacks& AllocationCallbacks::SetUserData(void* pUserData)
    {
        m_pUserData = pUserData;
        return *this;
    }

    void* AllocationCallbacks::Allocate(const size_t size, const size_t alignment) const
    {
        return m_alloc(m_pUserData, size, alignment);
    }

    void AllocationCallbacks::Free(void* pMemory) const
    {
        m_free(m_pUserData, pMemory);
    }

    void* AllocationCallbacks::Reallocate(void* pOriginal, const size_t size, const size_t alignment) const
    {
        return m_realloc(m_pUserData, pOriginal, size, alignment);
    }
}