// Buffer.h
#pragma once
#include "Nessie/Core/Memory/Memory.h"
#include "Nessie/Debug/Assert.h"

namespace nes
{
    class Buffer
    {
    public:
        Buffer() = default;
        Buffer(const uint64 size);
        Buffer(void* pData, const uint64 size = 0) : m_pData(static_cast<uint8*>(pData)), m_size(size) {}
        Buffer(const Buffer& buffer) = delete;
        Buffer(Buffer&& other) noexcept;
        virtual ~Buffer() = default;
        
        /// Operators
        Buffer&             operator=(const Buffer& buffer) = delete;
        Buffer&             operator=(Buffer&& other) noexcept;
        explicit            operator bool() const { return m_pData != nullptr; }
        uint8&              operator[](const uint64 index);
        uint8               operator[](const uint64 index) const;

        //----------------------------------------------------------------------------------------------------
        /// @brief : Allocate a new buffer and copy its data from src.
        //----------------------------------------------------------------------------------------------------
        static Buffer       Copy(const Buffer& src);

        //----------------------------------------------------------------------------------------------------
        /// @brief : Return a newly allocated buffer that is a copy of size number of bytes of pData.
        /// @param pData : Buffer to copy from.
        /// @param size : Number of bytes to copy.
        /// @param srcOffset : Byte offset from pData to begin copying from.
        //----------------------------------------------------------------------------------------------------
        static Buffer       Copy(const void* pData, const uint64 size, const uint64 srcOffset = 0);

        //----------------------------------------------------------------------------------------------------
        /// @brief : Allocate 'size' number of bytes.
        /// @note : This will free the current buffer memory if it was previously allocated. 
        //----------------------------------------------------------------------------------------------------
        void                Allocate(const uint64 size);

        //----------------------------------------------------------------------------------------------------
        /// @brief : Free the buffer memory. 
        //----------------------------------------------------------------------------------------------------
        void                Free();

        //----------------------------------------------------------------------------------------------------
        /// @brief : Read a section of the buffer as a 'Type'.
        ///	@param offset : Byte offset into the buffer.
        //----------------------------------------------------------------------------------------------------
        template <typename Type>
        Type&               Read(const uint64 offset = 0);

        //----------------------------------------------------------------------------------------------------
        /// @brief : Read a section of the buffer as a 'Type'. Const version.
        ///	@param offset : Byte offset into the buffer.
        //----------------------------------------------------------------------------------------------------
        template <typename Type>
        const Type&         Read(const uint64 offset = 0) const;

        //----------------------------------------------------------------------------------------------------
        /// @brief : Allocates a buffer of 'size' and copies a 'size' number of bytes from this buffer's memory into
        ///     that new buffer.
        ///	@param size : Size of the new buffer; must not be larger than this buffer!
        ///	@param offset : Byte offset into this buffer to read from.
        //----------------------------------------------------------------------------------------------------
        uint8*              ReadBytes(const uint64 size, const uint64 offset) const;

        //----------------------------------------------------------------------------------------------------
        /// @brief : Copies a section of this buffer into pOutBuffer.
        ///	@param size : Number of bytes you want to read from this buffer.
        ///	@param offset : Byte offset into this buffer to read from.
        //----------------------------------------------------------------------------------------------------
        void                ReadBytesIntoBuffer(const uint64 size, const uint64 offset, uint8* pOutBuffer) const;

        //----------------------------------------------------------------------------------------------------
        /// @brief : Copy 'size' number of bytes from pInData buffer into this buffer. 
        ///	@param pInData : Source buffer to copy from.
        ///	@param size : Number of bytes to write.
        ///	@param srcOffset : Byte offset of 'pInData' to copy from.
        ///	@param dstOffset : Byte offset of this buffer to begin copying into.  
        //----------------------------------------------------------------------------------------------------
        void                Write(const void* pInData, const uint64 size, uint64 srcOffset = 0, uint64 dstOffset = 0);

        //----------------------------------------------------------------------------------------------------
        /// @brief : Set all bytes of this buffer to zero. 
        //----------------------------------------------------------------------------------------------------
        void                ZeroInitialize();

        //----------------------------------------------------------------------------------------------------
        /// @brief : Get the raw pointer to the buffer. 
        //----------------------------------------------------------------------------------------------------
        [[nodiscard]]
        NES_INLINE void*    Get() const             { return m_pData; }

        //----------------------------------------------------------------------------------------------------
        /// @brief : Get the size of the buffer. 
        //----------------------------------------------------------------------------------------------------
        NES_INLINE uint64   GetSize() const         { return m_size; }

        //----------------------------------------------------------------------------------------------------
        /// @brief : Get the raw pointer cast to another type.
        //----------------------------------------------------------------------------------------------------
        template <typename Type>
        Type*               GetAs() const { return reinterpret_cast<Type*>(m_pData); }

    protected:
        uint8*              m_pData = nullptr;
        uint64              m_size = 0;
    };

    class ScopedBuffer final : public Buffer
    {
    public:
        virtual ~ScopedBuffer() override { Free(); }
    };
}

#include "Buffer.inl"