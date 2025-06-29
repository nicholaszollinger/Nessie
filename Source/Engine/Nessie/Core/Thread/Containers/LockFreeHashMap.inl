// LockFreeHashMap.inl
#pragma once
#include "Core/Memory/Memory.h"
#include "Math/Math.h"

namespace nes
{
    //------------------------------------------------------------------------------------------------------------
    // LFHM Allocator
    //------------------------------------------------------------------------------------------------------------
    
    inline LFHMAllocator::~LFHMAllocator()
    {
        NES_ALIGNED_FREE(m_objectStorage);
    }

    inline void LFHMAllocator::Init(const uint32 objectStorageSizeBytes)
    {
        NES_ASSERT(m_objectStorage == nullptr);

        m_objectStorageSizeBytes = objectStorageSizeBytes;
        m_objectStorage = static_cast<uint8*>(NES_ALIGNED_ALLOC(objectStorageSizeBytes, 16));
    }

    inline void LFHMAllocator::Clear()
    {
        m_writeOffset = 0;
    }

    inline void LFHMAllocator::Allocate(const uint32 blockSize, uint32& begin, uint32& end)
    {
        // If we're already beyond the end of our buffer, then don't do an atomic addition.
        // It's possible that many keys are inserted after the allocator is full, making it possible
        // for m_writeOffset (uint32) to wrap around to zero. When this happens, there will be a memory corruption.
        // This way, we will be able to progress the write offset beyond the size of the buffer.
        // worst case by max <CPU count> * blockSize.
        if (m_writeOffset.load(std::memory_order_relaxed) >= m_objectStorageSizeBytes)
            return;

        // Atomically fetch a block from the bool
        uint32 tBegin = m_writeOffset.fetch_add(blockSize, std::memory_order_relaxed);
        const uint32 tEnd = math::Min(begin + blockSize, m_objectStorageSizeBytes);

        if (end == tBegin)
        {
            // Block is allocated straight after our previous block.
            tBegin = begin;    
        }
        else
        {
            // Block is a new block.
            tBegin = math::Min(tBegin, m_objectStorageSizeBytes);
        }

        // Store the beginning and end of the resulting block.
        begin = tBegin;
        end = tEnd;
    }

    template <typename Type>
    uint32 LFHMAllocator::ToOffset(const Type* pData) const
    {
        const uint8* pByteData = reinterpret_cast<const uint8*>(pData);
        NES_ASSERT(pByteData >= m_objectStorage && pByteData < m_objectStorage + m_objectStorageSizeBytes);
        return static_cast<uint32>(pByteData - m_objectStorage);
    }

    template <typename Type>
    Type* LFHMAllocator::FromOffset(const uint32 offset) const
    {
        NES_ASSERT(offset < m_objectStorageSizeBytes);
        return reinterpret_cast<Type*>(m_objectStorage + offset);
    }

    //------------------------------------------------------------------------------------------------------------
    // LFHM Allocator Context
    //------------------------------------------------------------------------------------------------------------

    inline LFHMAllocatorContext::LFHMAllocatorContext(LFHMAllocator& allocator, const uint32 blockSize)
        : m_allocator(allocator)
        , m_blockSize(blockSize)
    {
        //
    }

    inline bool LFHMAllocatorContext::Allocate(const uint32 blockSize, const uint32 alignment, uint32& outWriteOffset)
    {
        // Calculate the bytes needed for alignment
        NES_ASSERT(math::IsPowerOf2(alignment));
        const uint32 alignmentMask = alignment - 1;
        uint32 finalAlignment = (alignment - (m_begin & alignmentMask)) & alignmentMask;

        // Check if we have space
        if (m_end - m_begin < blockSize + finalAlignment)
        {
            // Allocate a new block
            m_allocator.Allocate(blockSize, m_begin, m_end);

            // Update alignment
            finalAlignment = (alignment - (m_begin & alignmentMask)) & alignmentMask;

            // Check if we have space again
            if (m_end - m_begin < blockSize + finalAlignment)
                return false;
        }

        // Make the allocation
        m_begin += finalAlignment;
        outWriteOffset = m_begin;
        m_begin += blockSize;

        return true;
    }

    //------------------------------------------------------------------------------------------------------------
    // Lock Free Hash Map
    //------------------------------------------------------------------------------------------------------------

    template <TriviallyDestructible KeyType, TriviallyDestructible ValueType>
    typename LockFreeHashMap<KeyType, ValueType>::KeyValuePair& LockFreeHashMap<KeyType, ValueType>::Iterator::operator*()
    {
        NES_ASSERT(m_offset != kInvalidHandle);
        return *m_pMap->m_allocator.template FromOffset<KeyValuePair>(m_offset);
    }

    template <TriviallyDestructible KeyType, TriviallyDestructible ValueType>
    typename LockFreeHashMap<KeyType, ValueType>::Iterator& LockFreeHashMap<KeyType, ValueType>::Iterator::operator++()
    {
        NES_ASSERT(m_bucket < m_pMap->m_numBuckets);

        // Find the next key value in the bucket.
        if (m_offset != kInvalidHandle)
        {
            const KeyValuePair* pKeyValue = m_pMap->m_allocator.template FromOffset<KeyValuePair>(m_offset);
            m_offset = pKeyValue->m_nextOffset;
            if (m_offset != kInvalidHandle)
                return *this;
        }

        // Loop over the next buckets
        for (;;)
        {
            // Next bucket
            ++m_bucket;
            if (m_bucket >= m_pMap->m_numBuckets)
                return *this;

            // Fetch the first entry in the next bucket.
            m_offset = m_pMap->m_buckets[m_bucket];
            if (m_offset != kInvalidHandle)
                return *this;
        }
    }

    template <TriviallyDestructible KeyType, TriviallyDestructible ValueType>
    void LockFreeHashMap<KeyType, ValueType>::Init(const uint32 maxBuckets)
    {
        NES_ASSERT(maxBuckets >= 4 && math::IsPowerOf2(maxBuckets));
        NES_ASSERT(m_buckets == nullptr);

        m_numBuckets = maxBuckets;
        m_maxBuckets = maxBuckets;

        m_buckets = static_cast<std::atomic<uint32>*>(NES_ALIGNED_ALLOC(maxBuckets * sizeof(std::atomic<uint32>), 16));
    }

    template <TriviallyDestructible KeyType, TriviallyDestructible ValueType>
    LockFreeHashMap<KeyType, ValueType>::~LockFreeHashMap()
    {
        NES_ALIGNED_FREE(m_buckets);
    }

    template <TriviallyDestructible KeyType, TriviallyDestructible ValueType>
    void LockFreeHashMap<KeyType, ValueType>::Clear()
    {
    #ifdef NES_ASSERTS_ENABLED
        // Reset the number of key value pairs.
        m_numKeyValues = 0;
    #endif

        // Reset buckets 4 at a time.
        static_assert(sizeof(std::atomic<uint32>) == sizeof(uint32));
        UVec4Reg invalidHandle = UVec4Reg::Replicate(kInvalidHandle);
        uint32* pStart = reinterpret_cast<uint32*>(m_buckets);
        const uint32* pEnd = pStart + m_numBuckets;
        NES_ASSERT(math::IsAligned(pStart, 16));
        while (pStart < pEnd)
        {
            invalidHandle.StoreInt4Aligned(pStart);
            pStart += 4;
        }
    }

    template <TriviallyDestructible KeyType, TriviallyDestructible ValueType>
    void LockFreeHashMap<KeyType, ValueType>::SetNumBuckets(const uint32 numBuckets)
    {
        NES_ASSERT(m_numKeyValues == 0);
        NES_ASSERT(numBuckets <= m_maxBuckets);
        NES_ASSERT(numBuckets >= 4 && math::IsPowerOf2(numBuckets));

        m_numBuckets = numBuckets;
    }

    template <TriviallyDestructible KeyType, TriviallyDestructible ValueType>
    template <typename ... Params>
    typename LockFreeHashMap<KeyType, ValueType>::KeyValuePair* LockFreeHashMap<KeyType, ValueType>::Create(LFHMAllocatorContext& context, const KeyType& key, uint64 keyHash, const int extraBytes, Params&&... ctorParams)
    {
        // This is not a multimap; test that the key hasn't been inserted yet.
        NES_ASSERT(Find(key, keyHash) == nullptr);

        // Calculate the total size.
        const uint size = sizeof(KeyValuePair) + extraBytes;

        // Get the write offset for this key value pair.
        uint32 writeOffset;
        if (!context.Allocate(size, alignof(KeyValuePair), writeOffset))
            return nullptr;

    #ifdef NES_ASSERTS_ENABLED
        // Increment the number of key value pairs.
        m_numKeyValues.fetch_add(1, std::memory_order_relaxed);
    #endif

        // Construct a new key value pair:
        KeyValuePair* pKeyValue = m_allocator.template FromOffset<KeyValuePair>(writeOffset);
        NES_ASSERT(reinterpret_cast<intptr_t>(pKeyValue) % alignof(KeyValuePair) == 0);
        
    #ifdef NES_DEBUG
        std::memset(pKeyValue, 0xcd, size);
    #endif
        pKeyValue->m_key = key;
        new (&pKeyValue->m_value) ValueType(std::forward<Params>(ctorParams)...);

        // Get the offset to the first object from the bucket with the corresponding hash
        std::atomic<uint32>& offset = m_buckets[keyHash & (m_numBuckets - 1)];

        // Add this entry as the first element in the linked list.
        uint32 oldOffset = offset.load(std::memory_order_relaxed);
        for (;;)
        {
            pKeyValue->m_nextOffset = oldOffset;
            if (offset.compare_exchange_weak(oldOffset, writeOffset, std::memory_order_release))
                break;
        }
        
        return pKeyValue;
    }

    template <TriviallyDestructible KeyType, TriviallyDestructible ValueType>
    const typename LockFreeHashMap<KeyType, ValueType>::KeyValuePair* LockFreeHashMap<KeyType, ValueType>::Find(const KeyType& key, const uint64 keyHash) const
    {
        // Get the offset to the key value object from the bucket with the corresponding hash.
        uint32 offset = m_buckets[keyHash & (m_numBuckets - 1)].load(std::memory_order_acquire);
        while (offset != kInvalidHandle)
        {
            // Loop through the linked list of values until the right one is found
            const KeyValuePair* pKeyValue = m_allocator.template FromOffset<const KeyValuePair>(offset);
            if (pKeyValue->m_key == key)
                return pKeyValue;

            offset = pKeyValue->m_nextOffset;
        }

        // Not Found
        return nullptr;
    }
    
    template <TriviallyDestructible KeyType, TriviallyDestructible ValueType>
    uint32 LockFreeHashMap<KeyType, ValueType>::ToHandle(const KeyValuePair* pKeyValuePair) const
    {
        return m_allocator.ToOffset(pKeyValuePair);
    }

    template <TriviallyDestructible KeyType, TriviallyDestructible ValueType>
    const typename LockFreeHashMap<KeyType, ValueType>::KeyValuePair* LockFreeHashMap<KeyType, ValueType>::FromHandle(const uint32 handle) const
    {
        return m_allocator.template FromOffset<const KeyValuePair>(handle);
    }

    template <TriviallyDestructible KeyType, TriviallyDestructible ValueType>
    void LockFreeHashMap<KeyType, ValueType>::GetAllKeyValuePairs(std::vector<const KeyValuePair*>& outKeyValues) const
    {
        for (const std::atomic<uint32>* pBucket = m_buckets; pBucket < m_buckets + m_numBuckets; ++pBucket)
        {
            uint32 offset = *pBucket;
            // Walk the linked list and add all values.
            while (offset != kInvalidHandle)
            {
                const KeyValuePair* pKeyValue = m_allocator.template FromOffset<const KeyValuePair>(offset);
                outKeyValues.push_back(pKeyValue);
                offset = pKeyValue->m_nextOffset;
            }
        }
    }

    template <TriviallyDestructible KeyType, TriviallyDestructible ValueType>
    typename LockFreeHashMap<KeyType, ValueType>::Iterator LockFreeHashMap<KeyType, ValueType>::begin()
    {
        Iterator it(this, 0, m_buckets[0]);

        // If it doesn't contain a valid entry, use the ++operator to find the first valid entry.
        if (it.m_offset == kInvalidHandle)
            ++it;
        
        return it;
    }
    
    template <TriviallyDestructible KeyType, TriviallyDestructible ValueType>
    typename LockFreeHashMap<KeyType, ValueType>::Iterator LockFreeHashMap<KeyType, ValueType>::end()
    {
        return Iterator(this, m_numBuckets, kInvalidHandle);
    }    
}
