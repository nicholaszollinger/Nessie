// Memory.h
#pragma once
#include <vcruntime_new.h>
#include "Nessie/Core/Config.h"

//----------------------------------------------------------------------------------------------------
/// @brief : Macro to toggle recording allocations as they happen to print out each missed allocation
///     at the call to NES_DUMP_AND_DESTROY_LEAK_DETECTOR(). Very expensive.
//----------------------------------------------------------------------------------------------------
#define NES_ENABLE_ALLOCATION_TRACKING 0

//#define NES_DISABLE_CUSTOM_ALLOCATOR
namespace nes::memory::internal
{
    /// Allocation Functions
    /// - You should be using NES_ALLOC, NES_FREE, etc.
    void*   Allocate(size_t size);
    void*   Reallocate(void* pMemory, const size_t oldSize, const size_t newSize);
    void    Free(void* pMemory);
    void*   AlignedAllocate(size_t size, size_t alignment);
    void*   AlignedReallocate(void* pMemory, const size_t size, const size_t alignment);
    void    AlignedFree(void* pMemory);
}

#define NES_STACK_ALLOCATE(size) alloca(size)

#if !defined(NES_DISABLE_CUSTOM_ALLOCATOR) && defined(NES_DEBUG)

namespace nes::memory::internal
{
    /// Leak Detector management
    void    InitLeakDetector();
    void    DumpAndDestroyLeakDetector();
    
    /// Debug Allocation Functions
    void*   DebugAllocate(size_t size, const char* filename, int lineNum);
    void*   DebugReallocate(void* pMemory, size_t newSize, const char* filename, int lineNum);
    void    DebugFree(void* pMemory);
    void*   DebugAlignedAllocate(size_t size, size_t alignment, const char* filename, int lineNum);
    void*   DebugAlignedReallocate(void* pMemory, size_t size, size_t alignment, const char* filename, int lineNum);
    void    DebugAlignedFree(void* pMemory);
}

/// Debug new/delete operators
void*       operator new(size_t size, const char* filename, int lineNum);
void        operator delete(void* pMemory);
void        operator delete(void* pMemory, const char*, int);

/// Debug aligned new/delete operators
void*       operator new(size_t size, std::align_val_t alignment, const char* filename, int lineNum);
void        operator delete(void* pMemory, std::align_val_t alignment);
void        operator delete(void* pMemory, std::align_val_t alignment, const char*, int);

/// Debug array new/delete operators
void*       operator new[](size_t size, const char* filename, int lineNum);
void        operator delete[](void* pMemory);
void        operator delete[](void* pMemory, const char*, int);

//----------------------------------------------------------------------------------------------------
/// @brief : Initialize the Leak Detector. Must be called at the top of main(). 
//----------------------------------------------------------------------------------------------------
#define NES_INIT_LEAK_DETECTOR() nes::memory::internal::InitLeakDetector()

//----------------------------------------------------------------------------------------------------
/// @brief : Dump the records and destroy the leak detector. Must be called at the bottom of main(),
///     and NES_INIT_LEAK_DETECTOR() must be called at the top.
//----------------------------------------------------------------------------------------------------
#define NES_DUMP_AND_DESTROY_LEAK_DETECTOR() nes::memory::internal::DumpAndDestroyLeakDetector()

#define NES_NEW(type) new(__FILE__, __LINE__) type
#define NES_DELETE(ptr) delete ptr
#define NES_SAFE_DELETE(ptr) delete ptr; ptr = nullptr

#define NES_NEW_ARRAY(type, count) new(__FILE__, __LINE__) type[count]
#define NES_DELETE_ARRAY(ptr) delete[] ptr
#define NES_SAFE_DELETE_ARRAY(ptr) delete[] ptr; ptr = nullptr

#define NES_ALLOC(size) nes::memory::internal::DebugAllocate(size, __FILE__, __LINE__)
#define NES_REALLOC(ptr, size, alignment) nes::memory::internal::DebugReallocate(ptr, size, alignment, __FILE__, __LINE__)
#define NES_FREE(ptr) nes::memory::internal::DebugFree(ptr)
#define NES_ALIGNED_ALLOC(size, alignment) nes::memory::internal::DebugAlignedAllocate(size, alignment, __FILE__, __LINE__)
#define NES_ALIGNED_REALLOC(ptr, size, alignment) nes::memory::internal::DebugAlignedReallocate(ptr, size, alignment, __FILE__, __LINE__)
#define NES_ALIGNED_FREE(ptr) nes::memory::internal::DebugAlignedFree(ptr)


//----------------------------------------------------------------------------------------------------
/// @brief : Macro to override new and delete functions for a type.  
//----------------------------------------------------------------------------------------------------
#define NES_OVERRIDE_NEW_DELETE \
    NES_INLINE void* operator new (size_t size)                                                               { return NES_ALLOC(size); } \
    NES_INLINE void  operator delete (void* pPtr) noexcept                                                    { NES_FREE(pPtr);  } \
    NES_INLINE void* operator new[] (size_t size)                                                             { return NES_ALLOC(size); } \
    NES_INLINE void  operator delete[] (void* pPtr) noexcept                                                  { NES_FREE(pPtr); } \
    NES_INLINE void* operator new (size_t size, std::align_val_t alignment)                                   { return NES_ALIGNED_ALLOC(size, static_cast<size_t>(alignment)); } \
    NES_INLINE void  operator delete (void* pPtr, [[maybe_unused]] std::align_val_t alignment) noexcept       { NES_ALIGNED_FREE(pPtr); } \
    NES_INLINE void* operator new[] (size_t size, std::align_val_t alignment)                                 { return NES_ALIGNED_ALLOC(size, static_cast<size_t>(alignment)); } \
    NES_INLINE void  operator delete[] (void* pPtr, [[maybe_unused]] std::align_val_t alignment) noexcept     { NES_ALIGNED_FREE(pPtr); } \
    NES_INLINE void* operator new ([[maybe_unused]] size_t size, void* pPtr) noexcept                         { return pPtr; } \
    NES_INLINE void  operator delete ([[maybe_unused]] void *pPtr, [[maybe_unused]] void* pPlace) noexcept    { /* Do nothing */ } \
    NES_INLINE void* operator new[] ([[maybe_unused]] size_t size, void* pPtr) noexcept                       { return pPtr; } \
    NES_INLINE void  operator delete[] ([[maybe_unused]] void *pPtr, [[maybe_unused]] void* pPlace) noexcept  { /* Do nothing */ }

#else
#define NES_INIT_LEAK_DETECTOR() void(0)
#define NES_DUMP_AND_DESTROY_LEAK_DETECTOR() void(0)

#define NES_NEW(type) new type
#define NES_DELETE(ptr) delete ptr
#define NES_SAFE_DELETE(ptr) delete ptr; ptr = nullptr

#define NES_NEW_ARRAY(type, count) new type[count]
#define NES_DELETE_ARRAY(ptr) delete[] ptr
#define NES_SAFE_DELETE_ARRAY(ptr) delete[] ptr; ptr = nullptr

#define NES_ALLOC(size) nes::memory::internal::Allocate(size)
#define NES_REALLOC(pMemory, size, alignment) nes::memory::internal::Reallocate(pMemory, size, alignment)
#define NES_FREE(ptr) nes::memory::internal::Free(ptr)
#define NES_ALIGNED_ALLOC(size, alignment) nes::memory::internal::AlignedAllocate(size, alignment)
#define NES_ALIGNED_REALLOC(pMemory, size, alignment) nes::memory::internal::AlignedReallocate(pMemory, size, alignment)
#define NES_ALIGNED_FREE(ptr) nes::memory::internal::AlignedFree(ptr)

#define NES_OVERRIDE_NEW_DELETE

#endif