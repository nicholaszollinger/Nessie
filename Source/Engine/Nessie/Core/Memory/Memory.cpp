// Memory.cpp
#include "Memory.h"
#include "Debug/Assert.h"

#if defined(NES_PLATFORM_WINDOWS)
#include <crtdbg.h>
#endif

namespace nes::memory::internal
{
    void* Allocate(const size_t size)
    {
        NES_ASSERT(size > 0);
        return malloc(size);
    }

    void Free(void* pMemory)
    {
        free(pMemory);
    }

    void* AlignedAllocate(const size_t size, const size_t alignment)
    {
        NES_ASSERT(size > 0 && alignment > 0);

#if defined(NES_PLATFORM_WINDOWS)
        return _aligned_malloc(size, alignment);
#else
#error "AlignedAllocate() not implemented for platform".
#endif
    }

    void AlignedFree(void* pMemory)
    {
#if defined(NES_PLATFORM_WINDOWS)
        return _aligned_free(pMemory);
#else
#error "AlignedFree() not implemented for platform".
#endif
    }
}

#ifdef NES_DEBUG
namespace nes::memory::internal
{
    void InitLeakDetector()
    {
#if defined(NES_PLATFORM_WINDOWS)
        _CrtSetDbgFlag(_CRTDBG_ALLOC_MEM_DF | _CRTDBG_LEAK_CHECK_DF);
#endif
    }

    void DumpAndDestroyLeakDetector()
    {
        // [TODO]: This would spit out more specific logs based on tracked memory allocation.
    }

    void* DebugAllocate(size_t size, const char* filename, int lineNum)
    {
#if defined(NES_PLATFORM_WINDOWS)
        return _malloc_dbg(size, 1, filename, lineNum);
#else
#error "DebugAllocate() not implemented for platform".
#endif
    }

    void DebugFree(void* pMemory)
    {
#if defined(NES_PLATFORM_WINDOWS)
        return _free_dbg(pMemory, 1);
#else
#error "DebugFree() not implemented for platform".
#endif
    }

    void* DebugAlignedAllocate(size_t size, size_t alignment, const char* filename, int lineNum)
    {
#if defined(NES_PLATFORM_WINDOWS)
        return _aligned_malloc_dbg(size, alignment, filename, lineNum);
#else
#error "DebugAlignedAllocate() not implemented for platform".
#endif
    }

    void DebugAlignedFree(void* pMemory)
    {
#if defined(NES_PLATFORM_WINDOWS)
        return _aligned_free_dbg(pMemory);
#else
#error "DebugAlignedFree() not implemented for platform".
#endif
    }
}

void* operator new(const size_t size, const char* filename, const int lineNum)
{
    return nes::memory::internal::DebugAllocate(size, filename, lineNum);
}

void operator delete(void* pMemory, const char*, int)
{
    return nes::memory::internal::DebugFree(pMemory);
}

void* operator new(const size_t size, std::align_val_t alignment, const char* filename, const int lineNum)
{
    return nes::memory::internal::DebugAlignedAllocate(size, static_cast<size_t>(alignment), filename, lineNum);
}

void operator delete(void* pMemory, [[maybe_unused]] std::align_val_t alignment, const char*, int)
{
    return nes::memory::internal::DebugAlignedFree(pMemory);
}

void* operator new[](const size_t size, const char* filename, const int lineNum)
{
    return nes::memory::internal::DebugAllocate(size, filename, lineNum);
}

void operator delete[](void* pMemory, const char*, int)
{
    return nes::memory::internal::DebugFree(pMemory);
}
#endif