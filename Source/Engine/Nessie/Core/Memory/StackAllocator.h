#pragma once
// StackAllocator.h

#include "Debug/Assert.h"

namespace nes
{
    std::byte* GetAlignedPtr(std::byte* pPtr, const size_t alignment);
    
    //----------------------------------------------------------------------------------------------------
    //		NOTES:
    //		
    ///		@brief : Allocates a chunk of memory that is managed like a stack. Memory is allocated on the top
    ///              and freed from the top.
    //----------------------------------------------------------------------------------------------------
    class StackAllocator
    {
        static constexpr size_t kDefaultAlignment = 16;

    public:
        using Marker = size_t;

    private:
        std::byte* m_pBuffer = nullptr;
        std::byte* m_pEnd = nullptr;
        std::byte* m_pCapacity = nullptr;

    public:
        StackAllocator(const size_t stackSizeInBytes);
        StackAllocator(const StackAllocator&) = delete;
        StackAllocator(StackAllocator&& right) noexcept;
        StackAllocator& operator=(const StackAllocator&) = delete;
        StackAllocator& operator=(StackAllocator&& right) noexcept;
        ~StackAllocator();

        void* Alloc(const size_t size, const size_t alignment = kDefaultAlignment);
        void Free(std::byte*& pPtr, const size_t count);
        void FreeToMarker(const Marker marker);
        void FreeAll() { m_pEnd = m_pBuffer; }

        [[nodiscard]] Marker GetMarker() const;
        [[nodiscard]] size_t GetSize() const { return m_pEnd - m_pBuffer; }
        [[nodiscard]] size_t GetCapacity() const { return m_pCapacity - m_pBuffer; }
        [[nodiscard]] size_t GetFree() const { return m_pCapacity - m_pEnd; }
        [[nodiscard]] bool IsEmpty() const { return m_pBuffer == m_pEnd; }
        [[nodiscard]] bool IsFull() const { return m_pEnd == m_pCapacity; }
    };

    //----------------------------------------------------------------------------------------------------
    //		NOTES:
    //		
    ///		@brief : On construction, this will set a marker in the stack allocator. On destruction, it will free
    ///              memory up to the marker. Useful to use in a scope.
    //----------------------------------------------------------------------------------------------------
    class ScopedStackAllocator
    {
        StackAllocator& m_allocator;
        StackAllocator::Marker m_marker;

    public:
        ScopedStackAllocator(StackAllocator& allocator);
        ~ScopedStackAllocator();

        [[nodiscard]] StackAllocator& GetAllocator() const { return m_allocator; }
    };
}