// MutexArray.h
#pragma once
#include "Nessie/Core/Config.h"
#include "Nessie/Core/Memory/Memory.h"
#include "Nessie/Math/Generic.h"

namespace nes
{
    template <typename MutexType = std::mutex>
    class MutexArray
    {
        /// Align the mutex to a cache line to ensure that there is no
        /// false sharing (this is platform-dependent, we do this to be safe). 
        struct alignas(NES_CACHE_LINE_SIZE) MutexStorage
        {
            NES_OVERRIDE_NEW_DELETE
            MutexType m_mutex;
        };
        
    public:
        /// If default constructing, you need to initialize with Init(). 
        MutexArray() = default;
        explicit MutexArray(const uint32_t numMutexes)              { Init(numMutexes); }
        ~MutexArray()                                               { delete m_pMutexStorage; } 
        
        MutexArray(const MutexArray&)                               = delete;
        MutexArray& operator=(const MutexArray&)                    = delete;
        MutexArray(MutexArray&&) noexcept                           = default;
        MutexArray& operator=(MutexArray&&) noexcept                = default;

        //----------------------------------------------------------------------------------------------------
        /// @brief : Initialize the array, allocating space for the number of mutexes. 
        //----------------------------------------------------------------------------------------------------
        void                Init(const uint32_t numMutexes)
        {
            NES_ASSERT(m_pMutexStorage == nullptr);
            NES_ASSERT(numMutexes > 0 && math::IsPowerOf2(numMutexes));

            m_pMutexStorage = new MutexStorage[numMutexes];
            m_numMutexes = numMutexes;
        }

        //----------------------------------------------------------------------------------------------------
        /// @brief : Get the number of mutexes that were allocated. 
        //----------------------------------------------------------------------------------------------------
        inline uint32_t     GetNumMutexes() const                       { return m_numMutexes; }

        //----------------------------------------------------------------------------------------------------
        /// @brief : Convert an object index to a mutex index
        //----------------------------------------------------------------------------------------------------
        inline uint32_t     GetMutexIndex(const uint32_t objectIndex) const
        {
            static std::hash<uint32_t> hasher{};
            return hasher(objectIndex) & (m_numMutexes - 1);
        }

        //----------------------------------------------------------------------------------------------------
        /// @brief : Get the mutex belonging to a certain object by index.
        //----------------------------------------------------------------------------------------------------
        inline MutexType&   GetMutexByObjectIndex(const uint32_t objectIndex)
        {
            return m_pMutexStorage[GetMutexIndex(objectIndex)].m_mutex;
        }

        //----------------------------------------------------------------------------------------------------
        /// @brief : Get a mutex by index in the array.
        //----------------------------------------------------------------------------------------------------
        inline MutexType&   GetMutexByIndex(uint32_t mutexIndex)
        {
            NES_ASSERT(mutexIndex < m_numMutexes);
            return m_pMutexStorage[mutexIndex].m_mutex;
        }
        
        //----------------------------------------------------------------------------------------------------
        /// @brief : Lock all mutexes. 
        //----------------------------------------------------------------------------------------------------
        void                LockAll()
        {
            MutexStorage* pEnd = m_pMutexStorage + m_numMutexes;
            for (MutexStorage* pCurrent = m_pMutexStorage; pCurrent < pEnd; ++pCurrent)
            {
                pCurrent->m_mutex.lock();
            }
        }

        //----------------------------------------------------------------------------------------------------
        /// @brief : Unlock all mutexes. 
        //----------------------------------------------------------------------------------------------------
        void                UnlockAll()
        {
            MutexStorage* pEnd = m_pMutexStorage + m_numMutexes;
            for (MutexStorage* pCurrent = m_pMutexStorage; pCurrent < pEnd; ++pCurrent)
            {
                pCurrent->m_mutex.unlock();
            }
        }

    private:
        MutexStorage*   m_pMutexStorage = nullptr;
        uint32_t        m_numMutexes = 0;
    };
}
