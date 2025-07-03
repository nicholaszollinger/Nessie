// STLStackAllocator.h
#pragma once
#include "Nessie/Core/Memory/StackAllocator.h"

namespace nes
{
    //----------------------------------------------------------------------------------------------------
    /// @brief : Wrapper around StackAllocator to make it compatible with std containers. 
    //----------------------------------------------------------------------------------------------------
    template <typename Type>
    class STLStackAllocator
    {
        StackAllocator& m_allocator;
        
    public:
        using value_type = Type;
        using pointer = Type*;
        using const_pointer = const Type*;
        using reference = Type&;
        using const_reference = const Type&;
        using size_type = std::size_t;
        using difference_type = std::ptrdiff_t;
        using is_always_equal = std::false_type; /// The allocator is not stateless.

        /// Converting to an allocator of another type.
        template <typename OtherType>
        struct rebind
        {
            using other = STLStackAllocator<OtherType>;  
        };
    
    public:
        inline          STLStackAllocator(StackAllocator& stackAllocator) : m_allocator(stackAllocator) {}

        /// Construct from another typed allocator.
        template <typename OtherType>
        explicit        STLStackAllocator(const STLStackAllocator<OtherType>& other) : m_allocator(other.GetAllocator()) {}

        bool            operator==(const STLStackAllocator& other) const { return &m_allocator == &other.m_allocator; }
        bool            operator!=(const STLStackAllocator& other) const { return &m_allocator != &other.m_allocator; }

    public:
        //----------------------------------------------------------------------------------------------------
        /// @brief : STL Allocate.
        //----------------------------------------------------------------------------------------------------
        inline pointer  allocate(size_type size)
        {
            return pointer(m_allocator.Allocate(size * sizeof(value_type)));
        }

        //----------------------------------------------------------------------------------------------------
        /// @brief : STL Deallocate.
        //----------------------------------------------------------------------------------------------------
        inline void     deallocate(pointer pPtr, const size_type size)
        {
            m_allocator.Free(reinterpret_cast<std::byte*>(pPtr), size * sizeof(value_type));
        }

        //----------------------------------------------------------------------------------------------------
        /// @brief : Get the underlying Stack Allocator used by wrapper. 
        //----------------------------------------------------------------------------------------------------
        [[nodiscard]] StackAllocator& GetAllocator() const { return m_allocator; }
    };
}