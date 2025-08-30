// BodyLock.h
#pragma once
#include "BodyLockInterface.h"

namespace nes
{
    //----------------------------------------------------------------------------------------------------
    /// @brief : Base class for locking bodies for the scope of this class. (Do not directly use). 
    //----------------------------------------------------------------------------------------------------
    template <bool Write, class BodyType>
    class BodyLockBase
    {
    public:
        //----------------------------------------------------------------------------------------------------
        /// @brief : Attempts to acquire a lock on the bodies on construction. 
        //----------------------------------------------------------------------------------------------------
        BodyLockBase(const BodyLockInterface& bodyLockInterface, const BodyID& bodyID)
            : m_bodyLockInterface(bodyLockInterface)
        {
            // Check for invalid ID
            if (!bodyID.IsValid())
            {
                m_pBodyMutex = nullptr;
                m_pBody = nullptr;
            }
            else
            {
                // Get a mutex.
                if constexpr (Write)
                    m_pBodyMutex = bodyLockInterface.LockWrite(bodyID);
                else
                    m_pBodyMutex = bodyLockInterface.LockRead(bodyID);

                // Get a reference to the body or nullptr if when it is no longer valid.
                m_pBody = bodyLockInterface.TryGetBody(bodyID);
            }
        }

        BodyLockBase(const BodyLockBase&) = delete;
        BodyLockBase& operator=(const BodyLockBase&) = delete;
        
        //----------------------------------------------------------------------------------------------------
        /// @brief : Releases the Lock on leaving scope. 
        //----------------------------------------------------------------------------------------------------
        ~BodyLockBase() { ReleaseLock(); }

        //----------------------------------------------------------------------------------------------------
        /// @brief : Explicitly release the lock (normally this is done in the destructor)
        //----------------------------------------------------------------------------------------------------
        inline void ReleaseLock()
        {
            if (m_pBodyMutex != nullptr)
            {
                if constexpr (Write)
                    m_bodyLockInterface.UnlockWrite(m_pBodyMutex);
                else
                    m_bodyLockInterface.UnlockRead(m_pBodyMutex);

                m_pBodyMutex = nullptr;
                m_pBody = nullptr;
            }
        }

        //----------------------------------------------------------------------------------------------------
        /// @brief : Test if the lock was successful (if the BodyID was valid).  
        //----------------------------------------------------------------------------------------------------
        inline bool Succeeded() const { return m_pBody != nullptr; }

        //----------------------------------------------------------------------------------------------------
        /// @brief : Test if the lock was successful (if the BodyID was valid) and the body is still in the
        ///     broad phase.
        //----------------------------------------------------------------------------------------------------
        inline bool SucceededAndIsInBroadPhase() const { return m_pBody != nullptr && m_pBody->IsInBroadPhase(); }

        //----------------------------------------------------------------------------------------------------
        /// @brief : Access the body. Make sure that the lock succeeded before getting the body. 
        //----------------------------------------------------------------------------------------------------
        inline BodyType& GetBody() const
        {
            NES_ASSERT(m_pBody != nullptr);
            return *m_pBody;
        }

    private:
        const BodyLockInterface&    m_bodyLockInterface;
        SharedMutex*                m_pBodyMutex;
        BodyType*                   m_pBody;  
    };

    template <bool Write, class BodyType>
    class BodyLockMultiBase
    {
    public:
        using MutexMask = BodyLockInterface::MutexMask;

    public:
        //----------------------------------------------------------------------------------------------------
        /// @brief : Attempts to acquire a lock on the bodies on construction. 
        //----------------------------------------------------------------------------------------------------
        BodyLockMultiBase(const BodyLockInterface& bodyLockInterface, const BodyID* pBodies, const int count)
            : m_bodyLockInterface(bodyLockInterface)
            , m_mutexMask(bodyLockInterface.GetMutexMask(pBodies, count))
            , m_pBodies(pBodies)
            , m_numBodies(count)
        {
            if (m_mutexMask != 0)
            {
                if constexpr (Write)
                    bodyLockInterface.LockWrite(m_mutexMask);
                else
                    bodyLockInterface.LockRead(m_mutexMask);
            }
        }

        BodyLockMultiBase(const BodyLockMultiBase&) = delete;
        BodyLockMultiBase& operator=(const BodyLockMultiBase&) = delete;

        //----------------------------------------------------------------------------------------------------
        /// @brief : Release the lock on destruction. 
        //----------------------------------------------------------------------------------------------------
        ~BodyLockMultiBase()
        {
            if (m_mutexMask != 0)
            {
                if (Write)
                    m_bodyLockInterface.UnlockWrite(m_mutexMask);
                else
                    m_bodyLockInterface.UnlockRead(m_mutexMask);
            }
        }

        //----------------------------------------------------------------------------------------------------
        /// @brief : Access a body at a given index. Returns nullptr if the body was not properly locked.
        //----------------------------------------------------------------------------------------------------
        inline BodyType* GetBody(const int bodyIndex) const
        {
            NES_ASSERT(bodyIndex >= 0 && bodyIndex < m_numBodies);

            const BodyID& bodyID = m_pBodies[bodyIndex];
            if (!bodyID.IsValid())
                return nullptr;
            
            return m_bodyLockInterface.TryGetBody(bodyID);
        }

    private:
        const BodyLockInterface&    m_bodyLockInterface;
        MutexMask                   m_mutexMask;
        const BodyID*               m_pBodies;
        int                         m_numBodies;
    };

    //----------------------------------------------------------------------------------------------------
    /// @brief : Lock a Body for read, for the scope of this class.
    /// A body lock takes a body ID and locks the underlying body so that other threads cannot access its members.
    /// The common usage pattern is:
    /// <code>
    ///		BodyLockInterface lock_interface = physics_system.GetBodyLockInterface(); // Or non-locking interface if the lock is already taken
    ///		BodyID body_id = ...; // Obtain ID to body
    ///
    ///		// Scoped lock
    ///		{
    ///			BodyLockRead lock(lock_interface, body_id);
    ///			if (lock.Succeeded()) // body_id may no longer be valid
    ///			{
    ///				const Body &body = lock.GetBody();
    ///
    ///				// Do something with body
    ///				...
    ///			}
    ///		}
    /// </code>
    //----------------------------------------------------------------------------------------------------
    class BodyLockRead final : public BodyLockBase<false, const Body>
    {
        using BodyLockBase::BodyLockBase;  
    };

    //----------------------------------------------------------------------------------------------------
    /// @brief : Lock a Body for write, for the scope of this class.
    /// A body lock takes a body ID and locks the underlying body so that other threads cannot access its members.
    /// The common usage pattern is:
    /// <code>
    ///		BodyLockInterface lock_interface = physics_system.GetBodyLockInterface(); // Or non-locking interface if the lock is already taken
    ///		BodyID body_id = ...; // Obtain ID to body
    ///
    ///		// Scoped lock
    ///		{
    ///			BodyLockRead lock(lock_interface, body_id);
    ///			if (lock.Succeeded()) // body_id may no longer be valid
    ///			{
    ///				const Body &body = lock.GetBody();
    ///
    ///				// Do something with body
    ///				...
    ///			}
    ///		}
    /// </code>
    //----------------------------------------------------------------------------------------------------
    class BodyLockWrite final : public BodyLockBase<true, Body>
    {
        using BodyLockBase::BodyLockBase;
    };

    //----------------------------------------------------------------------------------------------------
    /// @brief : Lock multiple bodies for read, for the scope of this class.
    /// A multi body lock takes a number of body IDs and locks the underlying bodies so that other threads cannot access its members
    ///
    /// The common usage pattern is:
    /// <code>
    ///		BodyLockInterface lock_interface = physics_system.GetBodyLockInterface(); // Or non-locking interface if the lock is already taken
    ///		const BodyID *body_id = ...; // Obtain IDs to bodies
    ///		int num_body_ids = ...;
    ///
    ///		// Scoped lock
    ///		{
    ///			BodyLockMultiRead lock(lock_interface, body_ids, num_body_ids);
    ///			for (int i = 0; i < num_body_ids; ++i)
    ///			{
    ///				const Body *body = lock.GetBody(i);
    ///				if (body != nullptr)
    ///				{
    ///					const Body &body = lock.Body();
    ///
    ///					// Do something with body
    ///					...
    ///				}
    ///			}
    ///		}
    /// </code>
    //----------------------------------------------------------------------------------------------------
    class BodyLockMultiRead final : public BodyLockMultiBase<false, const Body>
    {
        using BodyLockMultiBase::BodyLockMultiBase;  
    };

    //----------------------------------------------------------------------------------------------------
    /// @brief : Lock multiple bodies for write, for the scope of this class.
    /// A multi body lock takes a number of body IDs and locks the underlying bodies so that other threads cannot access its members
    ///
    /// The common usage pattern is:
    /// <code>
    ///		BodyLockInterface lock_interface = physics_system.GetBodyLockInterface(); // Or non-locking interface if the lock is already taken
    ///		const BodyID *body_id = ...; // Obtain IDs to bodies
    ///		int num_body_ids = ...;
    ///
    ///		// Scoped lock
    ///		{
    ///			BodyLockMultiRead lock(lock_interface, body_ids, num_body_ids);
    ///			for (int i = 0; i < num_body_ids; ++i)
    ///			{
    ///				const Body *body = lock.GetBody(i);
    ///				if (body != nullptr)
    ///				{
    ///					const Body &body = lock.Body();
    ///
    ///					// Do something with body
    ///					...
    ///				}
    ///			}
    ///		}
    /// </code>
    //----------------------------------------------------------------------------------------------------
    class BodyLockMultiWrite final : public BodyLockMultiBase<true, Body>
    {
        using BodyLockMultiBase::BodyLockMultiBase;  
    };
}
