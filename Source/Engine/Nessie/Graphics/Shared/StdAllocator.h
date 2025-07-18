// StdAllocator.h
#pragma once
#include "Nessie/Debug/Assert.h"
#include "Nessie/Core/Memory/Memory.h"
#include "Nessie/Graphics/GraphicsCore.h"

#include <string>
#include <unordered_map>
#include <vector>

//-------------------------------------------------------------------------------------------------
// Under development. This was a part of NRI, but I will probably remove later.
//-------------------------------------------------------------------------------------------------

// NES_BEGIN_GRAPHICS_NAMESPACE
//
// namespace internal
// {
//     inline void* AlignedMalloc([[maybe_unused]] void* userArg, const size_t size, const size_t alignment)
//     {
//         return NES_ALIGNED_ALLOC(size, alignment);
//     }
//
//     inline void* AlignedRealloc([[maybe_unused]] void* userArg, void* pMemory, const size_t size, const size_t alignment)
//     {
//         return NES_ALIGNED_REALLOC(pMemory, size, alignment);
//     }
//
//     inline void AlignedFree([[maybe_unused]] void* userArg, void* memory)
//     {
//         NES_ALIGNED_FREE(memory);
//     }
// }
//
// //----------------------------------------------------------------------------------------------------
// /// @brief : If any of the allocation callbacks are missing, all will be set to the default implementations. 
// //----------------------------------------------------------------------------------------------------
// inline void CheckAndSetDefaultAllocator(DeviceAllocationCallbacks& callbacks)
// {
//     if (!callbacks.Allocate || !callbacks.Free || !callbacks.Reallocate)
//     {
//         callbacks.Allocate = internal::AlignedMalloc;
//         callbacks.Free = internal::AlignedFree;
//         callbacks.Reallocate = internal::AlignedRealloc;
//     }
// }
//
// //----------------------------------------------------------------------------------------------------
// /// @brief : STL Allocator wrapper for DeviceAllocationCallbacks.
// //----------------------------------------------------------------------------------------------------
// template <typename Type>
// class StdAllocator
// {
// public:
//     using value_type = Type;
//     using size_type = size_t;
//     using difference_type = std::ptrdiff_t;
//     using propagate_on_container_move_assignment = std::true_type;
//     using is_always_equal = std::false_type;
//
//     StdAllocator(const DeviceAllocationCallbacks& callbacks)
//         : m_callbacks(callbacks)
//     {
//         //
//     }
//     
//     StdAllocator(const StdAllocator& allocator)
//         : m_callbacks(allocator.GetCallbacks())
//     {
//         //
//     }
//
//     template <typename Other>
//     StdAllocator(const StdAllocator<Other>& allocator)
//         : m_callbacks(allocator.GetCallbacks())
//     {
//         //
//     }
//
//     StdAllocator& operator=(const StdAllocator& allocator)
//     {
//         m_callbacks = allocator.GetCallbacks();
//         return *this;
//     }
//
//     bool operator==(const StdAllocator& allocator) const { return m_callbacks == allocator.m_callbacks; }
//     bool operator!=(const StdAllocator& allocator) const { return m_callbacks != allocator.m_callbacks; }
//     
//
//     Type* allocate(const size_t n) noexcept
//     {
//         return static_cast<Type*>(m_callbacks.Allocate(m_callbacks.m_pUserArg, n * sizeof(Type),alignof(Type)));
//     }
//
//     void deallocate(Type* pMemory, const size_t) noexcept
//     {
//         m_callbacks.Free(m_callbacks.m_pUserArg, static_cast<void*>(pMemory));
//     }
//
//     const DeviceAllocationCallbacks& GetCallbacks() const { return m_callbacks; }
//
//     template <typename Other>
//     using other = StdAllocator<Other>;
//
// private:
//     const DeviceAllocationCallbacks& m_callbacks{};
// };
//
// //----------------------------------------------------------------------------------------------------
// //  STL Container types using the StdAllocator.
// //----------------------------------------------------------------------------------------------------
//
// template <typename Type>
// using Vector = std::vector<Type, StdAllocator<Type>>;
//
// template <typename Key, typename Value>
// using UnorderedMap = std::unordered_map<Key, Value, std::hash<Key>, std::equal_to<Key>, StdAllocator<std::pair<const Key, Value>>>;
//
// using String = std::basic_string<char, std::char_traits<char>, StdAllocator<char>>;
//
// static constexpr size_t kMaxStackAllocSize = 32ull * 1024;
//
// //----------------------------------------------------------------------------------------------------
// /// @brief : A scoped buffer that is allocated once on construction, and freed on destruction.
// /// Wraps DeviceAllocationCallbacks.
// /// If the required size is less than kMaxStackAllocSize, then the buffer will be allocated on the stack.
// /// Otherwise, it will defer to the heap.
// /// @note : Use NES_DEVICE_ALLOCATE_SCRATCH to create the Scratch buffer; don't construct directly. 
// //----------------------------------------------------------------------------------------------------
// template <typename Type>
// class Scratch
// {
// public:
//     Scratch(const DeviceAllocationCallbacks& allocator, Type* pMemory, const size_t num)
//         : m_allocator(allocator)
//         , m_pMemory(pMemory)
//         , m_num(num)
//     {
//         m_isHeap = (num * sizeof(Type) + alignof(Type)) > kMaxStackAllocSize;
//     }
//
//     ~Scratch()
//     {
//         if (m_isHeap)
//             m_allocator.Free(m_allocator.m_pUserArg, m_pMemory);
//     }
//
//     inline operator Type*() const
//     {
//         return m_pMemory;
//     }
//
//     inline Type& operator[](const size_t i) const
//     {
//         NES_ASSERT(i < m_num);
//         return m_pMemory[i];
//     }
//
// private:
//     const DeviceAllocationCallbacks& m_allocator;
//     Type* m_pMemory = nullptr;
//     size_t m_num = 0;
//     bool m_isHeap = false;
// };
//
// NES_END_GRAPHICS_NAMESPACE
//
// //----------------------------------------------------------------------------------------------------
// /// @brief : Allocate a scratch buffer using a device's Allocation Callbacks.
// ///	@param device : Device to use.
// ///	@param Type : Type to allocate.
// ///	@param numElements : Number of elements to allocate.
// //----------------------------------------------------------------------------------------------------
// #define NES_DEVICE_ALLOCATE_SCRATCH(device, Type, numElements)                  \
//     {(device).GetAllocationCallbacks(),                                         \
//         ((numElements) * sizeof(Type) + alignof(Type)) > kMaxStackAllocSize ?   \
//             static_cast<(Type*)>((device).GetAllocationCallbacks().Allocate((device).GetAllocationCallbacks().m_pUserArg, (numElements) * sizeof(Type) + alignof(Type))) \
//             : static_cast<(Type*)>(nes::AlignUp((numElements) ? static_cast<(Type*)>(NES_STACK_ALLOCATE((numElements) * sizeof(Type) + alignof(Type))) : nullptr)), alignof(Type), \
//         (numElements)}