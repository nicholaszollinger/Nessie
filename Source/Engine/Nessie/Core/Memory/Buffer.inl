// Buffer.inl
#pragma once

namespace nes
{
    inline Buffer Buffer::Copy(const Buffer& src)
    {
        Buffer buffer;
        buffer.Allocate(src.m_size);
        memcpy(buffer.m_pData, src.m_pData, src.m_size);
        return buffer;
    }

    inline Buffer Buffer::Copy(const void* pData, const uint64 size, const uint64 srcOffset)
    {
        Buffer buffer;
        buffer.Allocate(size);
        memcpy(buffer.m_pData, pData + srcOffset, size);
        return buffer;
    }
    
    inline uint8& Buffer::operator[](const uint64 index)
    {
        NES_ASSERT(index < m_size);
        return m_data[index];
    }

    inline uint8 Buffer::operator[](const uint64 index) const
    {
        NES_ASSERT(index < m_size);
        return m_data[index];
    }

    inline void Buffer::Allocate(const uint64 size)
    {
        Free();
        m_size = size;
        if (m_size == 0)
            return;

        m_pData = NES_NEW_ARRAY(uint8, size);
    }

    inline void Buffer::Free()
    {
        NES_DELETE_ARRAY(m_pData);
        m_pData = nullptr;
        m_size = 0;
    }

    template <typename Type>
    Type& Buffer::Read(const uint64 offset)
    {
        NES_ASSERT(offset * sizeof(Type) < size);
        return *reinterpret_cast<Type*>(m_pData + offset);
    }

    template <typename Type>
    const Type& Buffer::Read(const uint64 offset) const
    {
        NES_ASSERT(offset * sizeof(Type) < size);
        return *reinterpret_cast<Type*>(m_pData + offset);
    }

    inline void Buffer::Write(const void* pInData, const uint64 size, const uint64 srcOffset, const uint64 dstOffset)
    {
        NES_ASSERT(dstOffset + size <= m_size, "Buffer overflow!");
        memcpy(m_pData + dstOffset, pInData + srcOffset, size);
    }

    inline uint8* Buffer::ReadBytes(const uint64 size, const uint64 offset) const
    {
        NES_ASSERT(offset + size <= m_size, "Buffer overflow!");
        uint8* pData = NES_NEW_ARRAY(uint8, size);
        memcpy(pData, m_pData + offset, size);
        return pData;
    }

    inline void Buffer::ReadBytesIntoBuffer(const uint64 size, const uint64 offset, uint8* pOutBuffer) const
    {
        NES_ASSERT(m_pBuffer != nullptr);
        NES_ASSERT(pOutBuffer != nullptr);
        NES_ASSERT(offset + size <= m_size, "Buffer overflow!");
        memcpy(pOutBuffer, m_pData + offset, size);
    }

    inline void Buffer::Write(const void* pData, const uint64 size, uint64 offset)
    {
        NES_ASSERT(offset + size <= m_size, "Buffer overflow!");
        memcpy(m_pData + offset, pData, size);
    }

    inline void Buffer::ZeroInitialize()
    {
        
    }
}
