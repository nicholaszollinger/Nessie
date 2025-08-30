// RenderCommandQueue.cpp
#include "RenderCommandQueue.h"
#include <cstring>
#include "Nessie/Core/Memory/Memory.h"
#include "Nessie/Math/Generic.h"

namespace nes
{
    RenderCommandQueue::RenderCommandQueue()
    {
        // Allocate the buffer, and set all to zero.
        m_pCommandsBuffer = NES_NEW_ARRAY(uint8, kBufferSize);
        m_pCurrentWritePosition = m_pCommandsBuffer;
        memset(m_pCommandsBuffer, 0, kBufferSize);
    }

    RenderCommandQueue::~RenderCommandQueue()
    {
        NES_DELETE_ARRAY(m_pCommandsBuffer);
        m_pCommandsBuffer = nullptr;
        m_pCurrentWritePosition = nullptr;
        m_commandCount = 0;
    }

    void* RenderCommandQueue::Allocate(RenderCommandFunc func, const uint32 size)
    {
        // Align to 16 byte alignment. 
        const uint32 aligned = nes::math::AlignUp(size, 16);

        // Ensure that we have space.
        NES_ASSERT((m_pCurrentWritePosition - m_pCommandsBuffer) + (sizeof(RenderCommandFunc) + sizeof(uint32) + aligned) < kBufferSize, "Render Command Queue out of space!");
        
        // Write the function pointer
        *reinterpret_cast<RenderCommandFunc*>(m_pCurrentWritePosition) = func;
        m_pCurrentWritePosition += sizeof(RenderCommandFunc);
        
        // Write the size of the object.
        *reinterpret_cast<uint32*>(m_pCurrentWritePosition) = aligned;
        m_pCurrentWritePosition += sizeof(uint32);
        
        // Write the size of the allocation.
        void* pMemory = m_pCurrentWritePosition;
        m_pCurrentWritePosition += aligned;

        // Increase the command count.
        ++m_commandCount;
        return pMemory;
    }

    void RenderCommandQueue::Execute()
    {
        uint8* buffer = m_pCommandsBuffer;

        for (uint32 i = 0; i < m_commandCount; ++i)
        {
            // Grab the function pointer.
            RenderCommandFunc function = *reinterpret_cast<RenderCommandFunc*>(buffer);
            buffer += sizeof(RenderCommandFunc);

            // Grab the size
            const uint32 size = *reinterpret_cast<uint32*>(buffer);
            buffer += sizeof(uint32);

            // Call the function, passing along the additional data.
            function(buffer);

            // Move to the next function pointer.
            buffer += size;
        }

        // Reset the buffer.
        m_pCurrentWritePosition = m_pCommandsBuffer;
        m_commandCount = 0;
    }
}
