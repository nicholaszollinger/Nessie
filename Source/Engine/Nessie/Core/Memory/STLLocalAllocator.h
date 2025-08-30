// STLLocalAllocator.h
#pragma once
#include "Nessie/Core/Memory/STLAllocator.h"
#include "Nessie/Math/Generic.h"

namespace nes
{
#if !defined(NES_DISABLE_CUSTOM_ALLOCATOR)

    //----------------------------------------------------------------------------------------------------
    /// @brief : STL Allocator that keeps N elements in a local buffer before falling back to
    ///     regular allocations.
    //----------------------------------------------------------------------------------------------------
    template <typename Type, size_t N>
    class STLLocalAllocator : private STLAllocator<Type>
    {
        using Base = STLAllocator<Type>;

    public:
        using value_type = Type;
        using pointer = Type*;
        using const_pointer = const Type*;
        using reference = Type&;
        using const_reference = const Type&;
        using size_type = size_t;
        using difference_type = ptrdiff_t;
        using is_always_equal = std::false_type; /// The allocator is not stateless (has a local buffer).

        /// We cannot copy, move or swap allocators.
        using propagate_on_container_copy_assignment = std::false_type;
        using propagate_on_container_move_assignment = std::false_type;
        using propagate_on_container_swap = std::false_type;

        /// Always implements a reallocate function, as we can often reallocate in place.
        static constexpr bool has_reallocate = true;
        
        STLLocalAllocator() = default;
        STLLocalAllocator(const STLLocalAllocator&) = delete;
        STLLocalAllocator(STLLocalAllocator&&) noexcept = delete;
        STLLocalAllocator& operator=(const STLLocalAllocator&) = delete;
        STLLocalAllocator& operator=(STLLocalAllocator&&) noexcept = delete;

        //----------------------------------------------------------------------------------------------------
        /// @brief : Constructor used when rebinding to another type. This expects the allocator to use the
        ///     original memory pool from the first allocator, but in our case, we cannot use the local buffer
        ///     from the original allocator as it has a different size and alignment rules. To solve this,
        ///     we make this allocator fall back to the heap immediately.
        //----------------------------------------------------------------------------------------------------
        template <typename Type2>
        explicit STLLocalAllocator(const STLLocalAllocator<Type2, N>&) : m_numElementsUsed(N) {}

        //----------------------------------------------------------------------------------------------------
        /// @brief : Check if pPointer is in the local buffer.
        //----------------------------------------------------------------------------------------------------
        inline bool is_local(const_pointer pPointer) const
        {
            const ptrdiff_t diff = pPointer - reinterpret_cast<const_pointer>(m_elements);
            return diff >= 0 && diff < static_cast<ptrdiff_t>(N);
        }

        //----------------------------------------------------------------------------------------------------
        /// @brief : Allocate count number of elements. This will allocate in the local buffer first, before
        ///     falling back to the heap.
        //----------------------------------------------------------------------------------------------------
        inline pointer allocate(const size_type count)
        {
            // If we allocate more than we have, fall back to the heap.
            if (m_numElementsUsed + count > N)
                return Base::allocate(count);

            // Allocate from the local buffer.
            pointer result = reinterpret_cast<pointer>(m_elements) + m_numElementsUsed;
            m_numElementsUsed += count;
            return result;
        }

        //----------------------------------------------------------------------------------------------------
        /// @brief : Reallocate memory.
        //----------------------------------------------------------------------------------------------------
        inline pointer reallocate(pointer pOldPointer, const size_type oldSize, const size_type newSize)
        {
            NES_ASSERT(newSize > 0); // Reallocating to size zero is implementation-dependent, so we don't allow it.

            // If there was no previous allocation, we can go through the regular allocate function.
            if (pOldPointer == nullptr)
                return allocate(newSize);

            // If the pointer is outside our local buffer, fall back to the heap.
            if (!is_local(pOldPointer))
            {
                if constexpr (STLAllocatorHasReallocate<Base>)
                    return Base::reallocate(pOldPointer, oldSize, newSize);
                else
                    return ReallocateImpl(pOldPointer, oldSize, newSize);
            }

            // If we happen to have space left, we only need to update our bookkeeping.
            pointer pBasePtr = reinterpret_cast<pointer>(m_elements) + m_numElementsUsed - oldSize;
            if (pOldPointer == pBasePtr && m_numElementsUsed - oldSize + newSize <= N)
            {
                m_numElementsUsed += newSize - oldSize;
                return pBasePtr;
            }

            // We can't reallocate in place, fall back to the heap.
            return ReallocateImpl(pOldPointer, oldSize, newSize);
        }

        //----------------------------------------------------------------------------------------------------
        /// @brief : Deallocate memory.
        //----------------------------------------------------------------------------------------------------
        inline void deallocate(pointer pPointer, const size_type count)
        {
            // If the pointer is not in our local buffer, fall back to the heap.
            if (!is_local(pPointer))
                return Base::deallocate(pPointer, count);

            // Else, we can only reclaim memory if it was the last allocation.
            if (pPointer == reinterpret_cast<pointer>(m_elements) + m_numElementsUsed - count)
                m_numElementsUsed -= count;
        }

        inline bool operator==(const STLLocalAllocator& other) const { return this == &other; }
        inline bool operator!=(const STLLocalAllocator& other) const { return this != &other; }

        //----------------------------------------------------------------------------------------------------
        /// @brief : Converting to an allocator of another type. 
        //----------------------------------------------------------------------------------------------------
        template <typename Type2>
        struct rebind
        {
            using other = STLLocalAllocator<Type2, N>;
        };

    private:
        //----------------------------------------------------------------------------------------------------
        /// @brief : Implements reallocate when the base class doesn't or when we go from the local buffer
        ///     to the heap.
        //----------------------------------------------------------------------------------------------------
        inline pointer ReallocateImpl(pointer pOldPointer, const size_type oldSize, const size_type newSize)
        {
            pointer pNewPointer = Base::allocate(newSize);
            const size_type n = math::Min(oldSize, newSize);
            if constexpr (std::is_trivially_copyable_v<Type>)
            {
                // Can use mem copy
                memcpy(pNewPointer, pOldPointer, n * sizeof(Type));
            }
            else
            {
                // Need to actually move the elements.
                for (size_t i = 0; i < n; ++i)
                {
                    new (pNewPointer + i) Type(std::move(pOldPointer[i]));

                    if constexpr (!std::is_trivially_destructible_v<Type>)
                        pOldPointer[i].~Type();
                }
            }

            deallocate(pOldPointer, oldSize);
            return pNewPointer;
        }
        
        alignas(Type) uint8     m_elements[N * sizeof(Type)]; // Uninitialized local buffer of elements.
        size_type               m_numElementsUsed = 0;
    };

#else
    template <typename Type, size_t N>
    using STLLocalAllocator = std::allocator<Type>;
#endif
}