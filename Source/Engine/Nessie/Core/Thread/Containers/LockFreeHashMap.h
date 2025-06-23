// LockFreeHashMap.h
#pragma once
#include "Core/Config.h"
#include "Core/Generic/Concepts.h"
#include "Core/Thread/Atomics.h"
#include "Debug/Assert.h"

namespace nes
{
    //----------------------------------------------------------------------------------------------------
    /// @brief : Allocator class for the Lock-Free Hash Map.
    //----------------------------------------------------------------------------------------------------
    class LFHMAllocator
    {
    public:
        LFHMAllocator() = default;
        LFHMAllocator(LFHMAllocator&) = delete;
        LFHMAllocator& operator=(LFHMAllocator&) = delete;
        ~LFHMAllocator();

        //----------------------------------------------------------------------------------------------------
        /// @brief : Initialize the allocator. 
        ///	@param objectStorageSizeBytes : Number of bytes to reserve for all Key-Value pairs. 
        //----------------------------------------------------------------------------------------------------
        inline void                 Init(const uint32 objectStorageSizeBytes);

        //----------------------------------------------------------------------------------------------------
        /// @brief : Clear all allocations. 
        //----------------------------------------------------------------------------------------------------
        inline void                 Clear();

        //----------------------------------------------------------------------------------------------------
        /// @brief : Allocate a new block of data. 
        ///	@param blockSize : Size of the block to allocate (will potentially return a smaller block if memory is full).
        ///	@param begin : Should be the start of the first free byte in the current memory block on input. On return,
        ///     this will contain the start of the first free byte in allocated block.
        ///	@param end : Should be the byte beyond the current memory block on input. On return,
        ///     this will contain the byte beyond the allocated block.
        //----------------------------------------------------------------------------------------------------
        inline void                 Allocate(const uint32 blockSize, uint32& begin, uint32& end);

        //----------------------------------------------------------------------------------------------------
        /// @brief : Convert a pointer to an offset.
        //----------------------------------------------------------------------------------------------------
        template <typename Type>
        inline uint32               ToOffset(const Type* pData) const;

        //----------------------------------------------------------------------------------------------------
        /// @brief : Convert an offset to a pointer.
        //----------------------------------------------------------------------------------------------------
        template <typename Type>
        inline Type*                FromOffset(const uint32 offset) const;

    private:
        uint8*                      m_objectStorage = nullptr;          /// This contains a contiguous list of objects, possible of varying size.
        uint32                      m_objectStorageSizeBytes = 0;       /// The size of m_objectStorage in bytes.
        std::atomic<uint32>         m_writeOffset { 0 };          /// Next offset to write to in m_objectStorage.
    };

    //----------------------------------------------------------------------------------------------------
    /// @brief : Allocator context object for a Lock-Free Hash Map. This allocates a larger block of
    ///     memory at once and hands it out in smaller portions. This avoids contention on the atomic
    ///     LFHMAllocator::m_writeOffset.
    //----------------------------------------------------------------------------------------------------
    class LFHMAllocatorContext
    {
    public:
        /// Ctor
        inline                      LFHMAllocatorContext(LFHMAllocator& allocator, const uint32 blockSize);

        //----------------------------------------------------------------------------------------------------
        /// @brief : Allocate a data block.
        ///	@param blockSize : Size of the block, in bytes.
        ///	@param alignment : Alignment of the block to allocate.
        ///	@param outWriteOffset : Offset in the buffer where the block is located.
        ///	@returns : True if the allocation succeeded.
        //----------------------------------------------------------------------------------------------------
        inline bool                 Allocate(const uint32 blockSize, const uint32 alignment, uint32& outWriteOffset);

    private:
        LFHMAllocator&              m_allocator;
        uint32                      m_blockSize;
        uint32                      m_begin = 0;
        uint32                      m_end = 0;
    };

    //----------------------------------------------------------------------------------------------------
    /// @brief : Simple lock-free hash map class that only allows insertion, retrieval and provides a
    ///     fixed number of buckets and fixed storage.
    /// @note : For now, this class only accepts trivial types for the Key and Value.
    //----------------------------------------------------------------------------------------------------
    template <TriviallyDestructible KeyType, TriviallyDestructible ValueType>
    class LockFreeHashMap
    {
    public:
        using MapType = LockFreeHashMap<KeyType, ValueType>;

        /// Value of an invalid handle in the map.
        static constexpr uint32     kInvalidHandle = static_cast<uint32>(-1);

        //----------------------------------------------------------------------------------------------------
        /// @brief : Key Value pair object inserted into the map. 
        //----------------------------------------------------------------------------------------------------
        class KeyValuePair
        {
        public:
            const KeyType&          GetKey() const      { return m_key; }
            ValueType&              GetValue()          { return m_value; }
            const ValueType&        GetValue() const    { return m_value; }
            
        private:
            template <TriviallyDestructible K, TriviallyDestructible V> friend class LockFreeHashMap;

            KeyType                 m_key;          /// Key for this entry.
            uint32                  m_nextOffset;   /// Offset in m_objectStorage of the next KeyValuePair entry with the same hash.
            ValueType               m_value;        /// Value for this entry plus optional extra bytes.
        };

        //----------------------------------------------------------------------------------------------------
        /// @brief : Non-const iterator. 
        //----------------------------------------------------------------------------------------------------
        class Iterator
        {
            /// Private Ctor for internal use.
            Iterator(MapType* pMap, const uint32 bucket, const uint32 offset) : m_pMap(pMap), m_bucket(bucket), m_offset(offset) {}
            
        public:
            Iterator() = default;

            /// Comparison operators.
            bool                    operator==(const Iterator& other) const { return m_pMap == other.m_pMap && m_bucket == other.m_bucket && m_offset == other.m_offset; }
            bool                    operator!=(const Iterator& other) const { return !(*this == other); }

            /// Dereference to the key value pair.
            KeyValuePair&           operator*();

            /// Increment iterator
            Iterator&               operator++();
            
        private:
            template <TriviallyDestructible K, TriviallyDestructible V> friend class LockFreeHashMap;
            
            MapType*                m_pMap;
            uint32                  m_bucket;
            uint32                  m_offset;
        };

    public:
        explicit LockFreeHashMap(LFHMAllocator& allocator) : m_allocator(allocator) {}
        LockFreeHashMap(LockFreeHashMap&) = delete;
        LockFreeHashMap& operator=(LockFreeHashMap&) = delete;
        ~LockFreeHashMap();

        //----------------------------------------------------------------------------------------------------
        /// @brief : Initialize the hash map, allocating space for the fixed number of max buckets. 
        //----------------------------------------------------------------------------------------------------
        void                        Init(const uint32 maxBuckets);

        //----------------------------------------------------------------------------------------------------
        /// @brief : Remove all elements from the map.
        /// @note : This cannot happen simultaneously with adding new elements.
        //----------------------------------------------------------------------------------------------------
        void                        Clear();

        //----------------------------------------------------------------------------------------------------
        /// @brief : Get the number of buckets that this map is using. 
        //----------------------------------------------------------------------------------------------------
        uint32                      GetNumBuckets() const  { return m_numBuckets; }

        //----------------------------------------------------------------------------------------------------
        /// @brief : Get the maximum number of buckets that this map supports. 
        //----------------------------------------------------------------------------------------------------
        uint32                      GetMaxBuckets() const  { return m_maxBuckets; }

        //----------------------------------------------------------------------------------------------------
        /// @brief : Update the number of buckets. This must be done after clearing the map and cannot be
        ///     done concurrently with any other operations on the map.
        /// @note : The number of buckets can never become bigger than the specified max buckets during
        ///     initialization and that it must be a power of 2.
        //----------------------------------------------------------------------------------------------------
        void                        SetNumBuckets(const uint32 numBuckets);
        
        //----------------------------------------------------------------------------------------------------
        /// @brief : Insert a new element. Returns nullptr if the map is full.
        ///     Multiple threads can insert elements into the map at the same time.
        //----------------------------------------------------------------------------------------------------
        template <typename ... Params>
        inline KeyValuePair*        Create(LFHMAllocatorContext& context, const KeyType& key, uint64 keyHash, const int extraBytes, Params&&...ctorParams);

        //----------------------------------------------------------------------------------------------------
        /// @brief : Find an element. Returns nullptr if none found.
        //----------------------------------------------------------------------------------------------------
        inline const KeyValuePair*  Find(const KeyType& key, const uint64 keyHash) const;

        //----------------------------------------------------------------------------------------------------
        /// @brief : Convert a key value pair to a handle.
        //----------------------------------------------------------------------------------------------------
        inline uint32               ToHandle(const KeyValuePair* pKeyValuePair) const;

        //----------------------------------------------------------------------------------------------------
        /// @brief : Convert a handle to a KeyValuePair pointer.
        //----------------------------------------------------------------------------------------------------
        inline const KeyValuePair*  FromHandle(const uint32 handle) const;

#ifdef NES_ASSERTS_ENABLED
        //----------------------------------------------------------------------------------------------------
        /// @brief : Get the number of key value pairs that this map currently contains.
        /// @note : This is available only when asserts are enabled because adding elements creates contention
        ///     on this atomic and negatively affects performance.
        //----------------------------------------------------------------------------------------------------
        inline uint32               GetNumKeyValues() const { return m_numKeyValues; }
#endif

        //----------------------------------------------------------------------------------------------------
        /// @brief : Returns an array of all key value pairs.
        //----------------------------------------------------------------------------------------------------
        inline void                 GetAllKeyValuePairs(std::vector<const KeyValuePair*>& outKeyValues) const;

        //----------------------------------------------------------------------------------------------------
        /// @brief : Iterator to the first key value pair.
        /// @note : It is not safe to iterate in parallel to Clear(). However, it is safe to iterate while
        ///     adding elements to the map, but newly added elements may or may not be returned by this iterator.
        //----------------------------------------------------------------------------------------------------
        Iterator                    begin();

        //----------------------------------------------------------------------------------------------------
        /// @brief : Iterator to the end of the map.
        /// @note : It is not safe to do iterate in parallel to Clear(). However, it is safe to iterate while
        ///     adding elements to the map, but newly added elements may or may not be returned by this iterator.
        //----------------------------------------------------------------------------------------------------
        Iterator                    end();
        

    private:
        LFHMAllocator&              m_allocator;            /// Allocator used to allocate key value pairs.

#ifdef NES_ASSERTS_ENABLED
        std::atomic<uint32>         m_numKeyValues = 0;     /// Number of key value pairs in the storage.
#endif

        std::atomic<uint32>*        m_buckets = nullptr;    ///  This contains the offset in m_objectStorage of the first object with a particular hash.
        uint32                      m_numBuckets = 0;       /// Current number of buckets.
        uint32                      m_maxBuckets = 0;       /// Maximum number of buckets.
        
    };
}

#include "LockFreeHashMap.inl"