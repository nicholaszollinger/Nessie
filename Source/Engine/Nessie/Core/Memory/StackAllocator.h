// StackAllocator.h
#pragma once
#include "Debug/Assert.h"

namespace nes
{
    //----------------------------------------------------------------------------------------------------
    // This should probably be moved to something like Memory.h or something.
    //		
    ///	@brief : Get the address of the closest aligned ptr.
    ///	@param pPtr : Start address that we are going to add some number of bytes to ensure an aligned address.
    ///	@param alignment : Alignment we have to adhere to. MUST BE A POWER OF TWO.
    ///	@returns : Aligned address. It could be the same address if it is already aligned.
    //----------------------------------------------------------------------------------------------------
    std::byte* GetAlignedPtr(std::byte* pPtr, const size_t alignment);
    
    //----------------------------------------------------------------------------------------------------
    ///	@brief : Allocates a chunk of memory that is managed like a stack. Memory is allocated on the top
    ///     and freed from the top.
    //----------------------------------------------------------------------------------------------------
    class StackAllocator
    {
        static constexpr size_t kDefaultAlignment = 16;

    public:
        using Marker = size_t;

    public:
        StackAllocator(const size_t stackSizeInBytes);
        StackAllocator(const StackAllocator&) = delete;
        StackAllocator(StackAllocator&& right) noexcept;
        StackAllocator& operator=(const StackAllocator&) = delete;
        StackAllocator& operator=(StackAllocator&& right) noexcept;
        ~StackAllocator();

        void*                   Allocate(const size_t size, const size_t alignment = kDefaultAlignment);

        //----------------------------------------------------------------------------------------------------
        ///	@brief : Free memory off the top stack.
        ///	@param pPtr : Ptr to the memory we want to free.
        ///	@param count : Size of the memory we want to free.
        //----------------------------------------------------------------------------------------------------
        void                    Free(std::byte* pPtr, const size_t count);

        //----------------------------------------------------------------------------------------------------
        /// @brief : Frees memory of the top of the stack until it hits a given marker. 
        //----------------------------------------------------------------------------------------------------
        void                    FreeToMarker(const Marker marker);

        //----------------------------------------------------------------------------------------------------
        /// @brief : Free all allocated memory.  
        //----------------------------------------------------------------------------------------------------
        void                    FreeAll() { m_pEnd = m_pBuffer; }

        //----------------------------------------------------------------------------------------------------
        ///	@brief : Returns a position in the stack that we can free to. This is useful if you want to save
        ///     keep memory before the current position and free it later.
        //----------------------------------------------------------------------------------------------------
        [[nodiscard]] Marker    PlaceMarker() const;

        //----------------------------------------------------------------------------------------------------
        /// @brief : Returns the number of bytes that have been allocated. 
        //----------------------------------------------------------------------------------------------------
        size_t                  Size() const                { return m_pEnd - m_pBuffer; }

        //----------------------------------------------------------------------------------------------------
        /// @brief : Returns the capacity of this allocator.
        //----------------------------------------------------------------------------------------------------
        size_t                  Capacity() const            { return m_pCapacity - m_pBuffer; }

        //----------------------------------------------------------------------------------------------------
        /// @brief : Returns how much space is left to use. 
        //----------------------------------------------------------------------------------------------------
        size_t                  RemainingCapacity() const   { return m_pCapacity - m_pEnd; }

        //----------------------------------------------------------------------------------------------------
        /// @brief : Returns true if there have been no allocations.
        //----------------------------------------------------------------------------------------------------
        bool                    IsEmpty() const             { return m_pBuffer == m_pEnd; }

        //----------------------------------------------------------------------------------------------------
        /// @brief : Returns true if the allocator is out of stack space.
        //----------------------------------------------------------------------------------------------------
        bool                    IsFull() const              { return m_pEnd == m_pCapacity; }

    private:
        std::byte*              m_pBuffer = nullptr; /// Pointer to the current 
        std::byte*              m_pEnd = nullptr;
        std::byte*              m_pCapacity = nullptr;
    };

    //----------------------------------------------------------------------------------------------------
    /// @brief : On construction, this will set a marker in the stack allocator. On destruction, it will free
    ///     memory up to the marker. Useful to use in a scope.
    //----------------------------------------------------------------------------------------------------
    class ScopedStackAllocator
    {
    public:
        ScopedStackAllocator(StackAllocator& allocator);
        ~ScopedStackAllocator();

        [[nodiscard]] StackAllocator&   GetAllocator() const { return m_allocator; }

    private:
        StackAllocator&                 m_allocator;
        StackAllocator::Marker          m_marker;
    };
}