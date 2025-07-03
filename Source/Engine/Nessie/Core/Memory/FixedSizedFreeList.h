// FixedSizedFreeList.h
#pragma once
#include <atomic>
#include <numeric>

#include "Nessie/Core/Memory/Memory.h"
#include "Nessie/Core/Config.h"
#include "Nessie/Debug/Assert.h"
#include "Nessie/Math/Generic.h"
#include "Nessie/Core/Thread/Mutex.h"

namespace nes
{
    //----------------------------------------------------------------------------------------------------
    /// @brief : Class that allows lock-free creation and destruction of objects (unless a new page of objects
    ///         needs to be allocated). It contains a fixed pool of objects and also allows batching up a lot
    ///         of objects to be destroyed while doing the actual free in a single atomic operation.
    ///	@tparam ObjectType : Type of Object that will be allocated.
    //----------------------------------------------------------------------------------------------------
    template <typename ObjectType>
    class FixedSizeFreeList
    {
        struct ObjectStorage
        {
            /// The object we are storing.
            ObjectType              m_object;

            /// When the object is freed (or in the process of being freed as a batch) this will contain
            /// the next free object. When an object is in use, it will contain the object's index in the
            /// free list.
            std::atomic<uint32_t>   m_nextFreeObject;
        };
        static_assert(alignof(ObjectStorage) == alignof(ObjectType), "Object not properly aligned");

    public:
        static constexpr uint32_t   kInvalidObjectIndex = std::numeric_limits<uint32_t>::max();
        static constexpr int        kObjectStorageSize = sizeof(ObjectStorage);
        
        //----------------------------------------------------------------------------------------------------
        /// @brief : A Batch of objects to be destructed. 
        //----------------------------------------------------------------------------------------------------
        struct Batch
        {
            uint32_t m_firstObjectIndex = kInvalidObjectIndex;
            uint32_t m_lastObjectIndex = kInvalidObjectIndex;
            uint32_t m_numObjects = 0;
        };
        
    public:
        FixedSizeFreeList() = default;
        FixedSizeFreeList(const FixedSizeFreeList&) = delete;
        FixedSizeFreeList& operator=(const FixedSizeFreeList&) = delete;
        ~FixedSizeFreeList();

        //----------------------------------------------------------------------------------------------------
        /// @brief : Initialize the list, up to maxObjects can be allocated. 
        ///	@param maxObjects : Maximum amount of Objects that can be allocated.
        ///	@param numObjectsPerPage : Number of objects per page. When a new page needs to be allocated, a lock is obtained.
        //----------------------------------------------------------------------------------------------------
        void                            Init(const unsigned int maxObjects, unsigned int numObjectsPerPage);

        //----------------------------------------------------------------------------------------------------
        /// @brief : Destruct all objects allocated by the list. 
        //----------------------------------------------------------------------------------------------------
        void                            Clear();

        //----------------------------------------------------------------------------------------------------
        /// @brief : Lockless construct a new object. ctorParams are passed into the constructor.
        ///	@returns : Object Index. Can be equal to kInvalidObjectIndex, meaning that the allocator is out of
        ///         room!
        //----------------------------------------------------------------------------------------------------
        template <typename ... Parameters>
        uint32                          ConstructObject(Parameters&&...ctorParams);

        //----------------------------------------------------------------------------------------------------
        /// @brief : Lockless destruct an object and return it to the free pool. 
        //----------------------------------------------------------------------------------------------------
        void                            DestructObject(const uint32_t objectIndex);

        //----------------------------------------------------------------------------------------------------
        /// @brief : Lockless destruct an object and return it to the free pool. 
        //----------------------------------------------------------------------------------------------------
        void                            DestructObject(ObjectType* pObject);

        //----------------------------------------------------------------------------------------------------
        /// @brief : Add an object to an existing batch to be destructed.
        //----------------------------------------------------------------------------------------------------
        void                            AddObjectToBatch(Batch& batch, const uint32_t objectIndex);

        //----------------------------------------------------------------------------------------------------
        /// @brief : Lockless destruct batch of objects. 
        //----------------------------------------------------------------------------------------------------
        void                            DestructBatch(Batch& batch);

        //----------------------------------------------------------------------------------------------------
        /// @brief : Access an object by index.
        //----------------------------------------------------------------------------------------------------
        [[nodiscard]] ObjectType&       Get(const uint32_t objectIndex)             { return GetStorage(objectIndex).m_object; }

        //----------------------------------------------------------------------------------------------------
        /// @brief : Access an object by index 
        //----------------------------------------------------------------------------------------------------
        [[nodiscard]] const ObjectType& Get(const uint32_t objectIndex) const { return GetStorage(objectIndex).m_object; }

        [[nodiscard]] uint32_t          Count()          const { return m_numObjectsConstructed.load(std::memory_order_relaxed); }
        [[nodiscard]] uint32_t          Capacity()       const { return m_numPages * m_numObjectsPerPage; }
        [[nodiscard]] uint32_t          AllocatedSize()  const { return m_numObjectsAllocated; }
        
    private:
        const ObjectStorage&            GetStorage(const uint32_t objectIndex) const;
        ObjectStorage&                  GetStorage(const uint32_t objectIndex);

    private:
        /// Size (in objects) of a single page.
        uint32                          m_numObjectsPerPage;

        /// Number of bits to shift an object index to the right to get the page number.
        uint32                          m_pageShift;

        /// Mask to use with an object index to get the page number.
        uint32                          m_objectMask;

        /// Total number of pages that are usable.
        uint32                          m_numPages;

        /// Total number of objects that have been allocated.
        uint32                          m_numObjectsAllocated;

        /// Array of pages of objects.
        ObjectStorage**                 m_pPages = nullptr;
        
        /// Mutex that is used to allocate a new page if the storage runs out.
        /// This variable is aligned to the cache line to prevent false sharing with the
        /// constants used to index in to the list via "Get()".
        alignas(NES_CACHE_LINE_SIZE) Mutex m_pageMutex;

#ifdef NES_LOGGING_ENABLED
        /// Number of objects that are currently in the free list / new pages.
        std::atomic<uint32>             m_numFreeObjects;
#endif
        
        /// Counter that makes the first free object pointer update with every compare-and-exchange so that we don't suffer
        /// the "ABA" problem, which is when compare-and-exchange will succeed when it shouldn't.
        ///
        /// Say Thread 1 reads the value in the atomic. Then, Thread B does two operations: Read the value 
        /// update it, then return it to the same value and store it again. When Thread 1 goes to exchange,
        /// it sees the same expected value and performs the swap. This can cause unintended results. Having this
        /// counter be a part of the "m_firstFreeObjectAndTag" value, it means that we can properly exit
        /// when the tags are not equal. The counter is incremented every time that we are constructing a
        /// new "m_firstFreeObjectAndTag". So regardless if the indexes match, the allocationTag must match as well.
        /// - This is a similar mechanism to the Generation value in the "GenerationalID".
        std::atomic<uint32>             m_allocationTag;

        /// Index of the first free object, the first 32 bits of an object are used to point to the next free object.
        std::atomic<uint64>             m_firstFreeObjectAndTag;

        /// The first free object to use when the free list is empty (may need to allocate a new page).
        std::atomic<uint32>             m_firstFreeObjectInNewPage;

        /// Total number of objects that are actually constructed.
        std::atomic<uint32>             m_numObjectsConstructed;
    };
}

namespace nes
{
    template <typename ObjectType>
    FixedSizeFreeList<ObjectType>::~FixedSizeFreeList()
    {
        if (m_pPages != nullptr)
        {
            NES_ASSERT(m_numFreeObjects.load(std::memory_order::relaxed) == m_numPages * m_numObjectsPerPage);

            // Free Memory for pages:
            const uint32_t numPages = m_numObjectsAllocated / m_numObjectsPerPage;
            for (uint32_t page = 0; page < numPages; ++page)
            {
                NES_ALIGNED_FREE(m_pPages[page]);
            }
            
            NES_FREE(m_pPages);
        }
    }

    template <typename ObjectType>
    void FixedSizeFreeList<ObjectType>::Init(const unsigned int maxObjects, unsigned int numObjectsPerPage)
    {
        NES_ASSERT(numObjectsPerPage > 0 && math::IsPowerOf2(numObjectsPerPage));
        NES_ASSERT(m_pPages == nullptr);

        // Store configuration parameters
        m_numPages = (maxObjects + numObjectsPerPage - 1) / numObjectsPerPage;
        m_numObjectsPerPage = numObjectsPerPage;
        m_pageShift = math::CountTrailingZeros(numObjectsPerPage);
        m_objectMask = numObjectsPerPage - 1;
        NES_IF_LOGGING_ENABLED(m_numFreeObjects = m_numPages * m_numObjectsPerPage);

        // Allocate page table:
        m_pPages = reinterpret_cast<ObjectStorage**>(NES_ALLOC(m_numPages * sizeof(ObjectStorage*)));

        // No objects have been used on any pages.
        m_numObjectsAllocated = 0;
        m_firstFreeObjectInNewPage = 0;

        // Start with 1 as the first tag.
        m_allocationTag = 1;

        // Set the first free object (with tag == 0).
        m_firstFreeObjectAndTag = kInvalidObjectIndex;
    }

    template <typename ObjectType>
    void FixedSizeFreeList<ObjectType>::Clear()
    {
        if (m_numObjectsConstructed == 0)
            return;
        
        // Collect all Objects
        Batch batch{};

        for (uint32_t page = 0; page < m_numPages; ++page)
        {
            ObjectStorage* pPage = m_pPages[page];

            for (uint32_t objectIndex = 0; objectIndex < m_numObjectsPerPage; ++objectIndex)
            {
                ObjectStorage& storage = pPage[objectIndex];
                
                // Destruct if valid.
                if (storage.m_nextFreeObject == objectIndex)
                {
                    AddObjectToBatch(batch, objectIndex);
                    
                    if (batch.m_numObjects == m_numObjectsConstructed)
                        break;
                }
            }

            if (batch.m_numObjects == m_numObjectsConstructed)
                break;
        }

        DestructBatch(batch);
    }

    template <typename ObjectType>
    template <typename ... Parameters>
    uint32_t FixedSizeFreeList<ObjectType>::ConstructObject(Parameters&&... ctorParams)
    {
        for (;;)
        {
            // Get the first object from the linked list:
            uint64_t firstFreeObjectAndTag = m_firstFreeObjectAndTag.load(std::memory_order_acquire);
            uint32_t firstFreeObject = static_cast<uint32_t>(firstFreeObjectAndTag);
            if (firstFreeObject == kInvalidObjectIndex)
            {
                // The free list is empty, we take an object from the page that has never been used before
                firstFreeObject = m_firstFreeObjectInNewPage.fetch_add(1, std::memory_order::relaxed);
                if (firstFreeObject >= m_numObjectsAllocated)
                {
                    std::lock_guard lock(m_pageMutex);
                    while (firstFreeObject >= m_numObjectsAllocated)
                    {
                        uint32_t nextPage = m_numObjectsAllocated / m_numObjectsPerPage;

                        if (nextPage == m_numPages)
                            return kInvalidObjectIndex; // Out of Space!!!

                        // Allocate a new page of objects:
                        m_pPages[nextPage] = reinterpret_cast<ObjectStorage*>(NES_ALIGNED_ALLOC(m_numObjectsPerPage * sizeof(ObjectStorage), math::Max<size_t>(alignof(ObjectStorage), NES_CACHE_LINE_SIZE)));
                        m_numObjectsAllocated += m_numObjectsPerPage;
                    }
                }

                // Allocation Succeeded:
                NES_IF_LOGGING_ENABLED(m_numFreeObjects.fetch_sub(1, std::memory_order_relaxed));
                ObjectStorage& storage = GetStorage(firstFreeObject);
                // Construct the object
                new (&storage.m_object) ObjectType(std::forward<Parameters>(ctorParams)...);
                storage.m_nextFreeObject.store(firstFreeObject, std::memory_order_release);
                m_numObjectsConstructed.fetch_add(1, std::memory_order::relaxed);
                return firstFreeObject;
            }

            else
            {
                // The free list is not empty for this page, so get the next pointer
                const uint32_t newFirstFreeObject = GetStorage(firstFreeObject).m_nextFreeObject.load(std::memory_order_acquire);

                // Construct a new first free object tag
                uint64_t newFirstFreeObjectAndTag = static_cast<uint64_t>(newFirstFreeObject)
                    + (static_cast<uint64_t>(m_allocationTag.fetch_add(1, std::memory_order_relaxed)) << 32);

                // Compare and swap. If this fails (another thread beat us to this spot), we try again from the start of the loop. 
                if (m_firstFreeObjectAndTag.compare_exchange_weak(firstFreeObjectAndTag, newFirstFreeObjectAndTag, std::memory_order_release))
                {
                    // Allocation Successful
                    NES_IF_LOGGING_ENABLED(m_numFreeObjects.fetch_sub(1, std::memory_order_relaxed));
                    ObjectStorage& storage = GetStorage(firstFreeObject);
                    // Construct the Object
                    new (&storage.m_object) ObjectType(std::forward<Parameters>(ctorParams)...);
                    storage.m_nextFreeObject.store(firstFreeObject, std::memory_order_release);
                    m_numObjectsConstructed.fetch_add(1, std::memory_order::relaxed);
                    return firstFreeObject;
                }
            }
        }
    }

    template <typename ObjectType>
    void FixedSizeFreeList<ObjectType>::DestructObject(const uint32_t objectIndex)
    {
        NES_ASSERT(objectIndex != kInvalidObjectIndex);

        ObjectStorage& storage = GetStorage(objectIndex);

        // Call the Destructor, if non-trivial.
        if constexpr (!std::is_trivially_destructible_v<ObjectType>)
        {
            storage.m_object.~ObjectType();
        }

        // Add to the Object free list
        for (;;)
        {
            // Get the first object from the list:
            uint64_t firstFreeObjectAndTag = m_firstFreeObjectAndTag.load(std::memory_order_acquire);
            uint32_t firstFreeObject = static_cast<uint32_t>(firstFreeObjectAndTag);

            // Make it the next pointer of the last object in the batch that is to be freed.
            storage.m_nextFreeObject.store(firstFreeObject, std::memory_order_release);

            // Construct a new first free object tag
            uint64_t newFirstFreeObjectAndTag = static_cast<uint64_t>(objectIndex)
                    + (static_cast<uint64_t>(m_allocationTag.fetch_add(1, std::memory_order_relaxed)) << 32);

            // Compare and swap. If this fails (another thread beat us), then we try again from the new m_firstFreeObjectAndTag.
            if (m_firstFreeObjectAndTag.compare_exchange_weak(firstFreeObjectAndTag, newFirstFreeObjectAndTag, std::memory_order_release))
            {
                // Free Successful.
                m_numObjectsConstructed.fetch_sub(1, std::memory_order::relaxed);
                NES_IF_LOGGING_ENABLED(m_numFreeObjects.fetch_add(1, std::memory_order_relaxed));
                return;
            }
        }
    }

    template <typename ObjectType>
    void FixedSizeFreeList<ObjectType>::DestructObject(ObjectType* pObject)
    {
        const uint32_t index = reinterpret_cast<ObjectStorage*>(pObject)->m_nextFreeObject.load(std::memory_order_relaxed);
        NES_ASSERT(index < m_numObjectsAllocated);
        DestructObject(index);
    }

    template <typename ObjectType>
    void FixedSizeFreeList<ObjectType>::AddObjectToBatch(Batch& batch, const uint32_t objectIndex)
    {
        NES_ASSERT(batch.m_numObjects != std::numeric_limits<uint32_t>::max(), "Trying to reuse a FixedSizeFreeList::Batch that has already been freed!");

        // Reset the next index
        std::atomic<uint32_t>& nextFreeObject = GetStorage(objectIndex).m_nextFreeObject;
        NES_ASSERT(nextFreeObject.load(std::memory_order_relaxed) == objectIndex, "Trying to add an object to the FixedSizeFreeList::Batch that is already in the free list!");
        nextFreeObject.store(kInvalidObjectIndex, std::memory_order_release);
        
        // Link object in the batch to free:
        if (batch.m_firstObjectIndex == kInvalidObjectIndex)
            batch.m_firstObjectIndex = objectIndex;
        else
            GetStorage(batch.m_lastObjectIndex).m_nextFreeObject.store(objectIndex, std::memory_order_release);
            
        batch.m_lastObjectIndex = objectIndex;
        ++batch.m_numObjects;
    }

    template <typename ObjectType>
    void FixedSizeFreeList<ObjectType>::DestructBatch(Batch& batch)
    {
        if (batch.m_firstObjectIndex == kInvalidObjectIndex)
            return;

        // Call the destructors
        if constexpr (!std::is_trivially_destructible_v<ObjectType>)
        {
            uint32_t objectIndex = batch.m_firstObjectIndex;
            do
            {
                ObjectStorage& storage = GetStorage(objectIndex);
                storage.m_object.~ObjectType();
                objectIndex = storage.m_nextFreeObject.load(std::memory_order_relaxed);
            }
            while (objectIndex != kInvalidObjectIndex);
        }

        // Add Objects to the free list:
        ObjectStorage& storage = GetStorage(batch.m_lastObjectIndex);
        for (;;)
        {
            // Get the first object from the list
            uint64_t firstFreeObjectAndTag = m_firstFreeObjectAndTag.load(std::memory_order_acquire);
            uint32_t firstFreeObject = static_cast<uint32_t>(firstFreeObjectAndTag);

            // Make it the next pointer of the last object in the batch that is to be freed:
            storage.m_nextFreeObject.store(firstFreeObject, std::memory_order_release);

            // Construct a new first free object tag
            uint64_t newFirstFreeObjectAndTag = static_cast<uint64_t>(batch.m_firstObjectIndex)
                    + (static_cast<uint64_t>(m_allocationTag.fetch_add(1, std::memory_order_relaxed)) << 32);

            // Compare and swap
            if (m_firstFreeObjectAndTag.compare_exchange_weak(firstFreeObjectAndTag, newFirstFreeObjectAndTag, std::memory_order_release))
            {
                // Free Complete:
                NES_IF_LOGGING_ENABLED(m_numFreeObjects.fetch_add(batch.m_numObjects, std::memory_order_relaxed));

                m_numObjectsConstructed.fetch_sub(batch.m_numObjects, std::memory_order::relaxed);
                // Mark the batch as freed:
                NES_IF_LOGGING_ENABLED(batch.m_numObjects = std::numeric_limits<uint32_t>::max());

                return;
            }
        }
    }

    template <typename ObjectType>
    const typename FixedSizeFreeList<ObjectType>::ObjectStorage& FixedSizeFreeList<ObjectType>::GetStorage(const uint32_t objectIndex) const
    {
        return m_pPages[objectIndex >> m_pageShift][objectIndex & m_objectMask];
    }

    template <typename ObjectType>
    typename FixedSizeFreeList<ObjectType>::ObjectStorage& FixedSizeFreeList<ObjectType>::GetStorage(const uint32_t objectIndex)
    {
        return m_pPages[objectIndex >> m_pageShift][objectIndex & m_objectMask];
    }
}
