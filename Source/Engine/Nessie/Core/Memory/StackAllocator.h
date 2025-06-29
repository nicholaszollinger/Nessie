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
    ///     and freed from the top; the allocations must be freed in the reverse order that they are allocated in.
    ///     On construction, the buffer is allocated. On destruction, the buffer is freed. 
    //----------------------------------------------------------------------------------------------------
    class StackAllocator
    {
    public:
        using Marker = size_t;

    public:
        StackAllocator(const size_t stackSizeInBytes);
        StackAllocator(const StackAllocator&) = delete;
        StackAllocator(StackAllocator&& right) noexcept = default;
        StackAllocator& operator=(const StackAllocator&) = delete;
        StackAllocator& operator=(StackAllocator&& right) noexcept = default;
        ~StackAllocator();
        
        //----------------------------------------------------------------------------------------------------
        /// @brief : Allocate 'size' bytes of memory. The returned address must be NES_RVECTOR_BYTE aligned.
        //----------------------------------------------------------------------------------------------------
        void*                   Allocate(const size_t size);
        
        //----------------------------------------------------------------------------------------------------
        ///	@brief : Free memory off the top stack. Memory *must* be freed in the reverse order that it was allocated.
        ///	@param pAddress : Ptr to the memory we want to free.
        ///	@param size : Size of the memory we want to free, in bytes
        //----------------------------------------------------------------------------------------------------
        void                    Free(void* pAddress, const size_t size);

        //----------------------------------------------------------------------------------------------------
        /// @brief : Free memory off the stack to a specific marker.
        ///	@param marker : Marker to free memory to.
        //----------------------------------------------------------------------------------------------------
        void                    FreeToMarker(const Marker marker);

        //----------------------------------------------------------------------------------------------------
        /// @brief : Free all allocated memory.
        //----------------------------------------------------------------------------------------------------
        void                    FreeAll()                   { m_top = 0; }

        //----------------------------------------------------------------------------------------------------
        ///	@brief : Returns a position in the stack that we can free to. This is useful if you want to save
        ///     keep memory before the current position and free it later.
        //----------------------------------------------------------------------------------------------------
        [[nodiscard]] Marker    PlaceMarker() const;

        //----------------------------------------------------------------------------------------------------
        /// @brief : Returns the number of bytes that have been allocated. 
        //----------------------------------------------------------------------------------------------------
        size_t                  Size() const                { return m_top; }

        //----------------------------------------------------------------------------------------------------
        /// @brief : Returns the capacity of this allocator.
        //----------------------------------------------------------------------------------------------------
        size_t                  Capacity() const            { return m_capacity; }

        //----------------------------------------------------------------------------------------------------
        /// @brief : Returns how much space is left to use. 
        //----------------------------------------------------------------------------------------------------
        size_t                  RemainingCapacity() const   { return m_capacity - m_top; }

        //----------------------------------------------------------------------------------------------------
        /// @brief : Returns true if there have been no allocations.
        //----------------------------------------------------------------------------------------------------
        bool                    IsEmpty() const             { return m_top == 0; }

        //----------------------------------------------------------------------------------------------------
        /// @brief : Returns true if the allocator is out of stack space.
        //----------------------------------------------------------------------------------------------------
        bool                    IsFull() const              { return m_top == m_capacity; }

    private:
        std::byte*              m_pBase = nullptr;  /// Base address of the memory block.
        size_t                  m_top = 0;          /// End of the current allocated area.
        size_t                  m_capacity;         /// Size of the memory block, in bytes.
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