// StackAllocator.cpp

#include "StackAllocator.h"
#include <BleachNew.h>

namespace nes
{
    //----------------------------------------------------------------------------------------------------
    //		NOTES:
    //      This should probably be moved to something like GenericMemory.ixx or something.
    //		
    ///		@brief : Get the address of the closest aligned ptr.
    ///		@param pPtr : Start address that we are going to add some number of bytes to ensure an aligned address.
    ///		@param alignment : Alignment we have to adhere to. MUST BE A POWER OF TWO.
    ///		@returns : Aligned address. It could be the same address if it is already aligned.
    //----------------------------------------------------------------------------------------------------
    std::byte* GetAlignedPtr(std::byte* pPtr, const size_t alignment)
    {
        NES_ASSERTV(alignment > 0, "Alignment of Zero makes no sense!");

        // Unsigned integer that is the numeric size to hold the ptr.
        // We are treating the address as a number.
        const uintptr_t address = reinterpret_cast<uintptr_t>(pPtr);

        // If And-ing the mask and a value we want to align to endure that the
        // address is divisible by a power of 2.
        // Example: For an address of: 0100 (4), the mask will equal 0011 (3). 
        const size_t mask = alignment - 1;
        NES_ASSERTV((alignment & mask) == 0, "Alignment must be a power of two!");

        // Compute the aligned address.
        // Example: Address = 0101 (5), Mask = 0011 (3), ~Mask = 1100 (12).
        //          Address + Mask = 0101 + 0011 = 1000 (8).
        //          Address + Mask & ~Mask = 1000 & 1100 = 1000 (8). <- Aligned address. (It's a power of two!)
        const uintptr_t alignedAddress = (address + mask) & ~mask;

        // Return the aligned address.
        return reinterpret_cast<std::byte*>(alignedAddress);
    }

    StackAllocator::StackAllocator(const size_t stackSizeInBytes)
        : m_pBuffer(BLEACH_NEW_ARRAY(std::byte, stackSizeInBytes))
        , m_pEnd(nullptr)
        , m_pCapacity(nullptr)
    {
        m_pEnd = m_pBuffer;
        m_pCapacity = m_pBuffer + stackSizeInBytes;
    }

    StackAllocator::StackAllocator(StackAllocator&& right) noexcept
        : m_pBuffer(right.m_pBuffer)
        , m_pEnd(right.m_pEnd)
        , m_pCapacity(right.m_pCapacity)
    {
        right.m_pBuffer = nullptr;
        right.m_pEnd = nullptr;
        right.m_pCapacity = nullptr;
    }

    StackAllocator& StackAllocator::operator=(StackAllocator&& right) noexcept
    {
        if (this != &right)
        {
            m_pBuffer = right.m_pBuffer;
            m_pEnd = right.m_pEnd;
            m_pCapacity = right.m_pCapacity;

            right.m_pBuffer = nullptr;
            right.m_pEnd = nullptr;
            right.m_pCapacity = nullptr;
        }

        return *this;
    }

    StackAllocator::~StackAllocator()
    {
        BLEACH_DELETE_ARRAY(m_pBuffer);
        m_pBuffer = nullptr;
        m_pEnd = nullptr;
        m_pCapacity = nullptr;
    }

    void* StackAllocator::Alloc(const size_t size, const size_t alignment)
    {
        NES_ASSERTV(size > 0, "Size must be greater than zero!");

        // Get the aligned address.
        std::byte* pAlignedAddress = GetAlignedPtr(m_pEnd, alignment);

        // Calculate the size of the padding.
        std::byte* pNewEnd = pAlignedAddress + size;

        // Check if we have enough space.
        if (pNewEnd > m_pCapacity)
        {
            NES_CRITICAL("Attempted to Allocate memory, but the StackAllocator is full!");
        }

        // Set our new end:
        m_pEnd = pNewEnd;

        // Return the pointer to the allocated memory.
        return pAlignedAddress;
    }

    //----------------------------------------------------------------------------------------------------
    //		NOTES:
    //		
    ///		@brief : Free memory off the top stack.
    ///		@param pPtr : Ptr to the memory we want to free.
    ///		@param count : Size of the memory we want to free.
    //----------------------------------------------------------------------------------------------------
    void StackAllocator::Free(std::byte*& pPtr, const size_t count)
    {
        NES_ASSERTV(count > GetSize(), "Attempting to free more memory than what is currently allocated!");
        m_pEnd -= count;
        pPtr = nullptr;
    }

    //----------------------------------------------------------------------------------------------------
    //		NOTES:
    //		
    ///		@brief : Returns a position in the stack that we can free to. This is useful if you want to save
    ///              keep memory before the current position and free it later.
    //----------------------------------------------------------------------------------------------------
    StackAllocator::Marker StackAllocator::GetMarker() const
    {
        return m_pEnd - m_pBuffer;
    }

    //----------------------------------------------------------------------------------------------------
    //		NOTES:
    //		
    ///		@brief : Free memory off the stack to a specific marker.
    ///		@param marker : Marker to free memory to.
    //----------------------------------------------------------------------------------------------------
    void StackAllocator::FreeToMarker(const Marker marker)
    {
        NES_ASSERTV(!IsEmpty() && marker > GetSize(), "Failed to free to Marker! Either we attempted to free memory that "
                                                   "wasn't allocated, or the allocator is empty!");
        m_pEnd = m_pBuffer + marker;
    }

    ScopedStackAllocator::ScopedStackAllocator(StackAllocator& allocator)
        : m_allocator(allocator)
        , m_marker(allocator.GetMarker())
    {
        //
    }

    ScopedStackAllocator::~ScopedStackAllocator()
    {
        m_allocator.FreeToMarker(m_marker);
    }
}