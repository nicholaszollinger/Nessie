// STLAllocator.h
#pragma once
#include <type_traits>
#include "Core/Memory/Memory.h"
#include "Debug/Assert.h"

namespace nes
{
#if !defined(NES_DISABLE_CUSTOM_ALLOCATOR)

    template <typename Type>
    concept TypeNeedsAlignedAllocate = alignof(Type) > (NES_CPU_ADDRESS_BITS == 32? 8 : 16);

    template <typename Type>
    concept TypeAllowsSTLReallocate = std::is_trivially_copyable_v<Type> && TypeNeedsAlignedAllocate<Type>;
    
    //----------------------------------------------------------------------------------------------------
    /// @brief : STL Allocator that forwards to our allocation functions. 
    ///	@tparam Type : Type that will be allocated. 
    //----------------------------------------------------------------------------------------------------
    template <typename Type>
    class STLAllocator
    {
    public:
        using value_type = Type;
        using pointer = Type*;
        using const_pointer = const Type*;
        using reference = Type&;
        using const_reference = const Type&;
        using size_type = size_t;
        using difference_type = ptrdiff_t;
        using is_always_equal = std::true_type;                         /// Allocator is stateless
        using propagate_on_container_move_assigment = std::true_type;   /// Supports moving.

        static constexpr bool has_reallocate = TypeAllowsSTLReallocate<Type>;

        /// Constructor
        inline STLAllocator() = default;

        /// Constructor from another allocator
        template <typename Type2>
        inline STLAllocator(const STLAllocator<Type2>&) {}

        //----------------------------------------------------------------------------------------------------
        /// @brief : Allocate memory.
        ///	@param count : Number of elements to allocate.
        //----------------------------------------------------------------------------------------------------
        inline pointer  allocate(const size_type count)
        {
            if constexpr (TypeNeedsAlignedAllocate<Type>)
                return static_cast<pointer>(memory::internal::AlignedAllocate(count * sizeof(value_type), alignof(value_type)));
            else
                return static_cast<pointer>(memory::internal::Allocate(count * sizeof(value_type)));
        }

        //----------------------------------------------------------------------------------------------------
        /// @brief : Reallocate Memory. Only available if the type is trivially copyable and doesn't need to
        ///     be aligned.
        //----------------------------------------------------------------------------------------------------
        template <bool HasReallocateV = has_reallocate, typename = std::enable_if_t<HasReallocateV>>
        inline pointer  reallocate(pointer pOldPointer, const size_type oldSize, size_t newSize)
        {
            NES_ASSERT(newSize > 0); // Reallocating to size zero is implementation-dependent, so we don't allow it.
            return pointer(memory::internal::Reallocate(pOldPointer, oldSize * sizeof(value_type), newSize * sizeof(value_type)));
        }

        //----------------------------------------------------------------------------------------------------
        /// @brief : Free memory.
        //----------------------------------------------------------------------------------------------------
        inline void     deallocate(void* pPointer, size_type)
        {
            if constexpr (TypeNeedsAlignedAllocate<Type>)
                memory::internal::AlignedFree(pPointer);
            else
                memory::internal::Free(pPointer);
        }

        //----------------------------------------------------------------------------------------------------
        /// @brief : Allocators are stateless so assumed to be equal.
        //----------------------------------------------------------------------------------------------------
        inline bool operator==(const STLAllocator&) const { return true; }
        inline bool operator!=(const STLAllocator&) const { return false; }

        // Converting to an allocator for another type.
        template <typename Type2>
        struct rebind
        {
            using other = STLAllocator<Type2>;
        };
    };

    template <typename Type>
    concept AllocatorHasReallocate = Type::has_reallocate;
    
#else
    template <typename Type> using STLAllocator = std::allocator<Type>;
#endif
    
}
