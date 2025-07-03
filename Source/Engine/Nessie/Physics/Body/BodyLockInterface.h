// BodyLockInterface.h
#pragma once
#include "Body.h"
#include "BodyManager.h"
#include "Nessie/Physics/PhysicsLock.h"
#include "Nessie/Core/Thread/Mutex.h"

namespace nes
{
    //----------------------------------------------------------------------------------------------------
    /// @brief : Base class interface for locking a body. 
    //----------------------------------------------------------------------------------------------------
    class BodyLockInterface
    {
    public:
        using MutexMask = BodyManager::MutexMask;
        
    public:
        explicit BodyLockInterface(BodyManager& bodyManager) : m_bodyManager(bodyManager) {}
        virtual ~BodyLockInterface() = default;
        BodyLockInterface(const BodyLockInterface&) = delete;
        BodyLockInterface& operator=(const BodyLockInterface&) = delete;

        /// Locking Functions
        virtual SharedMutex*    LockRead(const BodyID& bodyID) const = 0;
        virtual void            UnlockRead(SharedMutex* pMutex) const = 0;
        virtual SharedMutex*    LockWrite(const BodyID& bodyID) const = 0;
        virtual void            UnlockWrite(SharedMutex* pMutex) const = 0;

        //----------------------------------------------------------------------------------------------------
        /// @brief : Get the mask needed to lock all bodies. 
        //----------------------------------------------------------------------------------------------------
        inline MutexMask        GetAllBodiesMutexMask() const { return m_bodyManager.Internal_GetAllBodiesMutexMask(); }

        /// Batch Locking Functions
        virtual MutexMask       GetMutexMask(const BodyID* pBodies, int count) const = 0;
        virtual void            LockRead(MutexMask mutexMask) const = 0;
        virtual void            UnlockRead(MutexMask mutexMask) const = 0;
        virtual void            LockWrite(MutexMask mutexMask) const = 0;
        virtual void            UnlockWrite(MutexMask mutexMask) const = 0;

        //----------------------------------------------------------------------------------------------------
        /// @brief : Attempt to get the Body* from its BodyID. 
        //----------------------------------------------------------------------------------------------------
        inline Body*            TryGetBody(const BodyID& bodyID) const  { return m_bodyManager.TryGetBody(bodyID); }

    protected:
        BodyManager&            m_bodyManager;
    };

    //----------------------------------------------------------------------------------------------------
    /// @brief : Implementation that performs no locking (assumes the lock is taken elsewhere).
    //----------------------------------------------------------------------------------------------------
    class BodyLockInterfaceNoLock final : public BodyLockInterface
    {
    public:
        using BodyLockInterface::BodyLockInterface;

        /// Individual Locking Functions:
        virtual SharedMutex*    LockRead(const BodyID&) const override           { return nullptr; }
        virtual void            UnlockRead(SharedMutex*) const override          { /* Do nothing */ }
        virtual SharedMutex*    LockWrite(const BodyID&) const override          { return nullptr; }
        virtual void            UnlockWrite(SharedMutex*) const override         { /* Do nothing */ }
        
        /// Batch Locking Functions:
        virtual MutexMask       GetMutexMask(const BodyID*, int) const override  { return 0; }
        virtual void            LockRead(MutexMask) const override               { /* Do nothing */ }
        virtual void            UnlockRead(MutexMask) const override             { /* Do nothing */ }
        virtual void            LockWrite(MutexMask) const override              { /* Do nothing */ }
        virtual void            UnlockWrite(MutexMask) const override            { /* Do nothing */ }
    };

    //----------------------------------------------------------------------------------------------------
    /// @brief : Implementation that uses the body manager to lock the correct mutex for a body. 
    //----------------------------------------------------------------------------------------------------
    class BodyLockInterfaceLocking final : public BodyLockInterface
    {
    public:
        using BodyLockInterface::BodyLockInterface;
        
        /// Individual Locking Functions:
        virtual SharedMutex*    LockRead(const BodyID& bodyID) const override
        {
            SharedMutex& mutex = m_bodyManager.GetMutexForBody(bodyID);
            PhysicsLock::LockShared(mutex NES_IF_ASSERTS_ENABLED(, &m_bodyManager, EPhysicsLockTypes::PerBody));
            return &mutex;
        }
        
        virtual void            UnlockRead(SharedMutex* pMutex) const override
        {
            PhysicsLock::UnlockShared(*pMutex NES_IF_ASSERTS_ENABLED(, &m_bodyManager, EPhysicsLockTypes::PerBody));
        }
        
        virtual SharedMutex*    LockWrite(const BodyID& bodyID) const override
        {
            SharedMutex& mutex = m_bodyManager.GetMutexForBody(bodyID);
            PhysicsLock::Lock(mutex NES_IF_ASSERTS_ENABLED(, &m_bodyManager, EPhysicsLockTypes::PerBody));
            return &mutex;
        }
        
        virtual void            UnlockWrite(SharedMutex* pMutex) const override
        {
            PhysicsLock::Unlock(*pMutex NES_IF_ASSERTS_ENABLED(, &m_bodyManager, EPhysicsLockTypes::PerBody));
        }
        
        /// Batch Locking Functions:
        virtual MutexMask       GetMutexMask(const BodyID* pBodies, const int count) const override { return m_bodyManager.Internal_GetMutexMask(pBodies, count); }
        virtual void            LockRead(MutexMask mutexMask) const override                        { m_bodyManager.Internal_LockRead(mutexMask); }
        virtual void            UnlockRead(MutexMask mutexMask) const override                      { m_bodyManager.Internal_UnlockRead(mutexMask); }
        virtual void            LockWrite(MutexMask mutexMask) const override                       { m_bodyManager.Internal_LockWrite(mutexMask); }
        virtual void            UnlockWrite(MutexMask mutexMask) const override                     { m_bodyManager.Internal_UnlockWrite(mutexMask); }
    };
}
