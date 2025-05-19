// BodyManager.h
#pragma once
#include <shared_mutex>
#include "Body.h"
#include "BodyActivationMode.h"
#include "Core/Thread/MutexArray.h"

namespace nes
{
    

    using BodyVector = std::vector<Body*>;
    using BodyIDVector = std::vector<BodyID>;
    class BodyActivationListener;

    //----------------------------------------------------------------------------------------------------
    /// @brief : Class that contains all bodies. 
    //----------------------------------------------------------------------------------------------------
    class BodyManager
    {
        friend class PhysicsScene;
        using BodyMutexes = MutexArray<std::shared_mutex>;
    
    public:
        /// Bodies are protected using an array of mutexes (so a fixed number, not 1 per body). Each set bit in this mask
        /// indicates a locked index.
        using MutexMask = uint64_t;
        
        struct BodyStats
        {
            uint32_t m_numBodies                = 0;
            uint32_t m_maxNumBodies             = 0;
            uint32_t m_numStaticBodies          = 0;
            uint32_t m_numDynamicBodies         = 0;
            uint32_t m_numActiveDynamicBodies   = 0;
            uint32_t m_numKinematicBodies       = 0;
            uint32_t m_numActiveKinematicBodies = 0;
        };

        // [TODO]: 
        // //----------------------------------------------------------------------------------------------------
        // /// @brief : Per Thread override of the locked state, to be used by PhysicsSystem only!!! 
        // //----------------------------------------------------------------------------------------------------
        // struct Internal_GrantActiveBodiesAccess
        // {
        //     // inline Internal_GrantActiveBodiesAccess(bool allowActivation, bool allowDeactivation)
        //     // {
        //     //     NES_ASSERT(!GetOverrideAllowActivation());
        //     //     SetOverrideAllowActivation(allowActivation);
        //     //
        //     //     NES_ASSERT(!GetOverrideAllowDeactivation());
        //     //     SetOverrideAllowDeactivation(allowDeactivation);
        //     // }
        //     //
        //     // inline ~Internal_GrantActiveBodiesAccess()
        //     // {
        //     //     SetOverrideAllowActivation(false);
        //     //     SetOverrideAllowDeactivation(false);
        //     // }
        // };
        
    private:
        
        /// Value that indicates that there are no more freed body IDs.
        static constexpr uintptr_t          kBodyIDFreeListEnd = ~static_cast<uintptr_t>(0);
        
        /// Bit that indicates a pointer in m_bodies is actually the index of the next freed body. We use the
        /// lowest bit because we know that bodies are 16 byte aligned so that addresses will never end in 1 bit.
        static constexpr uintptr_t          kIsFreedBody = static_cast<uintptr_t>(1);

        /// Amount of bits to shift to get an index to the next freed body.
        static constexpr unsigned           kFreedBodyIndexShift = 1;
        
        /// List of all pointers to all bodies. Contains invalid pointers for deleted bodies, check with
        /// kIsValidBodyPointer. Note that this array is reserved to hold the max num bodies that is passed
        /// in to the Init() function so that adding bodies will not reallocate the array.
        BodyVector                          m_bodies;

        /// Current number of allocated bodies.
        uint32_t                            m_numBodies = 0;

        /// Index of the first entry in m_bodies that is unused.
        uintptr_t                           m_bodyIDFreeListStart = kBodyIDFreeListEnd;

        /// Protects m_bodies array (bot not the bodies it points to), m_numBodies and m_bodyIDFreeListStart.
        mutable std::mutex                  m_bodiesMutex;
        
        /// Array of mutexes protecting the individual bodies in the m_bodies array. 
        mutable BodyMutexes                 m_bodyMutexes;
        
        /// List of the next sequence number for a body ID.
        std::vector<uint8_t>                m_bodySequenceNumbers;   

        /// Mutex that protects the m_pActiveBodies array.
        mutable std::mutex                  m_activeBodiesMutex;

        /// List of all active dynamic bodies (size is equal to max amount of bodies).
        // [TODO]: Body* m_pActiveBodies[kBodyTypeCount] = {} // <- Array for solid vs soft bodies.
        BodyID*                             m_pActiveBodies = nullptr;

        /// How many bodies are in the list of active bodies.
        // [TODO]: m_numActiveBodies[kBodyTypeCount] = {}; // <- Value for solid vs soft bodies.
        std::atomic<uint32_t>               m_numActiveBodies = 0;

        /// How many of the active bodies have continuous collision detected enabled.
        uint32_t                            m_numActiveCCDBodies = 0;

        /// Mutex that protects the m_bodiesCacheInvalid array.
        mutable std::mutex                  m_bodiesCacheInvalidMutex;

        /// List of all bodies that should have their cache invalidated.
        std::vector<BodyID>                 m_bodiesCacheInvalid;

        /// Listener that is notified whenever a body is activated/deactivated.
        BodyActivationListener*             m_pActivationListener = nullptr;

        /// Cached broadphase layer interface
        const BroadPhaseLayerInterface*     m_pBroadPhaseLayer = nullptr;
        
    public:
        BodyManager() = default;
        ~BodyManager();
        BodyManager(const BodyManager&) = delete;
        BodyManager& operator=(const BodyManager&) = delete;
        BodyManager(BodyManager&&) noexcept = delete;
        BodyManager& operator=(BodyManager&&) noexcept = delete;

        //----------------------------------------------------------------------------------------------------
        /// @brief : Initialize the manager.
        //----------------------------------------------------------------------------------------------------
        void                                Init(uint32_t maxBodies, uint32_t numBodyMutexes, const BroadPhaseLayerInterface& layerInterface);

        //----------------------------------------------------------------------------------------------------
        /// @brief : Get the current number of bodies in the body manager.
        //----------------------------------------------------------------------------------------------------
        inline uint32_t                     GetNumBodies() const;

        //----------------------------------------------------------------------------------------------------
        /// @brief : Get the max number of bodies that is supported. 
        //----------------------------------------------------------------------------------------------------
        inline uint32_t                     GetMaxNumBodies() const            { return static_cast<uint32_t>(m_bodies.capacity());}

        //----------------------------------------------------------------------------------------------------
        /// @brief : Get stats about the bodies in the Body Manager.
        /// @note : This is slow, it iterates through all bodies.
        //----------------------------------------------------------------------------------------------------
        BodyStats                           GetStats() const;

        //----------------------------------------------------------------------------------------------------
        /// @brief : Create a body using the CreateInfo object. The returned body will not be a part of the
        ///  Body Manager just yet. You need to explicitly call AddBody().
        //----------------------------------------------------------------------------------------------------
        Body*                               AllocateBody(const BodyCreateInfo& createInfo) const;

        //----------------------------------------------------------------------------------------------------
        /// @brief : Free a body that has *not* been added to the body manager yet.
        /// @note : If it has, use DestroyBodies().
        //----------------------------------------------------------------------------------------------------
        void                                FreeBody(Body* pBody) const;

        //----------------------------------------------------------------------------------------------------
        /// @brief : Add a body to the Body Manager, assigning it the next available ID. Returns false if there
        ///     are no more IDs available (i.e. the max limit of bodies has been reached).
        //----------------------------------------------------------------------------------------------------
        bool                                AddBody(Body* pBody);
    
        //----------------------------------------------------------------------------------------------------
        /// @brief : Add a body to the body manager, assigning it a custom ID. Returns false if the ID is not
        ///     valid. 
        //----------------------------------------------------------------------------------------------------
        bool                                AddBodyWithCustomID(Body* pBody, const BodyID& bodyID);

        //----------------------------------------------------------------------------------------------------
        /// @brief : Removes a list of bodies from the body manager, and stores them in pOutBodies.
        //----------------------------------------------------------------------------------------------------
        void                                RemoveBodies(const BodyID* pBodyIDs, const int count, Body** pOutBodies);

        //----------------------------------------------------------------------------------------------------
        /// @brief : Removes a list of bodies from the body manager, then frees them. 
        //----------------------------------------------------------------------------------------------------
        void                                DestroyBodies(const BodyID* pBodyIDs, const int count);

        //----------------------------------------------------------------------------------------------------
        /// @brief : Activate a list of bodies.
        /// @note : This function should only be called when an exclusive lock for the bodies are held.
        //----------------------------------------------------------------------------------------------------
        void                                ActivateBodies(const BodyID* pBodyIDs, const int count);

        //----------------------------------------------------------------------------------------------------
        /// @brief : Deactivate a list of bodies.
        /// @note : This function should only be called when an exclusive lock for the bodies are held.
        //----------------------------------------------------------------------------------------------------
        void                                DeactivateBodies(const BodyID* pBodyIDs, const int count);

        //----------------------------------------------------------------------------------------------------
        /// @brief : Update the motion quality of the Body. 
        //----------------------------------------------------------------------------------------------------
        void                                SetMotionQuality(Body& body, const BodyMotionQuality motionQuality);

        //----------------------------------------------------------------------------------------------------
        /// @brief : Get a copy of the Active bodies under protection of a lock. 
        //----------------------------------------------------------------------------------------------------
        void                                GetActiveBodies(/*BodyType type, */ BodyIDVector& outBodies) const;

        //----------------------------------------------------------------------------------------------------
        /// @brief : Get the list of active bodies. @note : This is not thread safe! The active bodies list can change at any moment. 
        //----------------------------------------------------------------------------------------------------
        const BodyID*                       GetActiveBodiesUnsafe(/*BodyType type*/) const { return m_pActiveBodies; }
        
        //----------------------------------------------------------------------------------------------------
        /// @brief : Get the number of active bodes. 
        //----------------------------------------------------------------------------------------------------
        inline uint32_t                     GetNumActiveBodies(/*BodyType type*/) const { return m_numActiveBodies.load(std::memory_order_acquire); }

        //----------------------------------------------------------------------------------------------------
        /// @brief : Get the number of active bodies that are using continuous collision detection. 
        //----------------------------------------------------------------------------------------------------
        inline uint32_t                     GetNumActiveCCDBodies() const { return m_numActiveCCDBodies; }

        //----------------------------------------------------------------------------------------------------
        /// @brief : Set the Listener that is notified whenever a body is activated/deactivated. 
        //----------------------------------------------------------------------------------------------------
        void                                SetBodyActivationListener(BodyActivationListener* pListener);

        //----------------------------------------------------------------------------------------------------
        /// @brief : Get the Listener that is notified whenever a body is activated/deactivated. 
        //----------------------------------------------------------------------------------------------------
        BodyActivationListener*             GetBodyActivationListener() const { return m_pActivationListener; }

        //----------------------------------------------------------------------------------------------------
        /// @brief : Check if this is a valid body pointer. When a body is freed, the memory that the pointer
        ///     occupies is reused to store a freelist.
        //----------------------------------------------------------------------------------------------------
        static inline bool                  IsValidBodyPointer(const Body* pBody) { return (reinterpret_cast<uintptr_t>(pBody) & kIsFreedBody) == 0; }

        //----------------------------------------------------------------------------------------------------
        /// @brief : Get all bodies. Note that this can contain invalid body pointers. Call IsValidBodyPointer(). 
        //----------------------------------------------------------------------------------------------------
        const BodyVector&                   GetBodies() const               { return m_bodies; }
        
        //----------------------------------------------------------------------------------------------------
        /// @brief : Get all bodies. Note that this can contain invalid body pointers. Call IsValidBodyPointer(). 
        //----------------------------------------------------------------------------------------------------
        BodyVector&                         GetBodies()                     { return m_bodies; }

        //----------------------------------------------------------------------------------------------------
        /// @brief : Get all body IDs under the protection of a lock. 
        //----------------------------------------------------------------------------------------------------
        void                                GetBodyIDs(BodyIDVector& outBodies) const;
        
        //----------------------------------------------------------------------------------------------------
        /// @brief : Access a Body. @note: Not protected by a lock!
        //----------------------------------------------------------------------------------------------------
        Body&                               GetBody(const BodyID& id)       { return *m_bodies[id.GetIndex()]; }

        //----------------------------------------------------------------------------------------------------
        /// @brief : Access a Body. @note: Not protected by a lock!
        //----------------------------------------------------------------------------------------------------
        const Body&                         GetBody(const BodyID& id) const { return *m_bodies[id.GetIndex()]; }

        //----------------------------------------------------------------------------------------------------
        /// @brief : Access a body, but will return nullptr if the bodyID is no longer valid. @note Not
        ///     protected by a lock!
        //----------------------------------------------------------------------------------------------------
        const Body*                         TryGetBody(const BodyID& id) const;

        //----------------------------------------------------------------------------------------------------
        /// @brief : Access a body, but will return nullptr if the bodyID is no longer valid. @note Not
        ///     protected by a lock!
        //----------------------------------------------------------------------------------------------------
        Body*                               TryGetBody(const BodyID& id);

        //----------------------------------------------------------------------------------------------------
        /// @brief : Get the mutex for a single body.
        //----------------------------------------------------------------------------------------------------
        std::shared_mutex&                  GetMutexForBody(const BodyID& id) const { return m_bodyMutexes.GetMutexByObjectIndex(id.GetIndex()); }

        //----------------------------------------------------------------------------------------------------
        /// @brief : Lock all bodies. This should only be done in PhysicsSystem::Update(). 
        //----------------------------------------------------------------------------------------------------
        void                                LockAllBodies() const;

        //----------------------------------------------------------------------------------------------------
        /// @brief : Unlock all bodies. This should only e done during PhysicsSystem::Update(). 
        //----------------------------------------------------------------------------------------------------
        void                                UnlockAllBodies() const;

        //----------------------------------------------------------------------------------------------------
        /// @brief : Set the Body::Flags::InvalidateContactCache flag for the specified body. This means that
        ///     the collision cache is invalid for any body pair involving that body until the next physics
        ///     step
        //----------------------------------------------------------------------------------------------------
        void                                InvalidateContactCacheForBody(Body& body);

        //----------------------------------------------------------------------------------------------------
        /// @brief : Reset the Body::Flags::InvalidateContactCache flag for all bodies. All body paris in the
        ///     contact cache will be valid again.
        //----------------------------------------------------------------------------------------------------
        void                                ValidateContactCacheForAllBodies();

        //----------------------------------------------------------------------------------------------------
        // FUNCTIONS BELOW ARE FOR INTERNAL USE ONLY.
        //----------------------------------------------------------------------------------------------------
        
        MutexMask                           Internal_GetAllBodiesMutexMask() const;
        MutexMask                           Internal_GetMutexMask(const BodyID* pBodyIDs, int number) const;
        void                                Internal_LockRead(const MutexMask mask) const;
        void                                Internal_UnlockRead(const MutexMask mask) const;
        void                                Internal_LockWrite(const MutexMask mask) const;
        void                                Internal_UnlockWrite(const MutexMask mask) const;
        void                                Internal_SetBodyCollisionLayer(Body& body, CollisionLayer layer) const;
    
    private:
        //----------------------------------------------------------------------------------------------------
        /// @brief : Increment and get the sequence number of a body. We intentionally overflow the uin8_t value.
        //----------------------------------------------------------------------------------------------------
        inline uint8_t                      GetNextSequenceNumber(int bodyIndex) { return ++m_bodySequenceNumbers[bodyIndex]; }

        //----------------------------------------------------------------------------------------------------
        /// @brief : Add a single body to m_activeBodies.
        /// @note : This doesn't lock the active body mutex!
        //----------------------------------------------------------------------------------------------------
        inline void                         AddBodyToActiveBodies(Body& body);

        //----------------------------------------------------------------------------------------------------
        /// @brief : Remove a single body to m_activeBodies.
        /// @note : This doesn't lock the active body mutex!
        //----------------------------------------------------------------------------------------------------
        inline void                         RemoveBodyFromActiveBodies(Body& body);

        //----------------------------------------------------------------------------------------------------
        /// @brief : Helper function to remove a Body from the manager.
        //----------------------------------------------------------------------------------------------------
        NES_INLINE Body*                    Internal_RemoveBody(const BodyID& id);

        //----------------------------------------------------------------------------------------------------
        /// @brief : Helper function to delete a body (which could actually be a BodyWithMotionProperties). 
        //----------------------------------------------------------------------------------------------------
        inline static void                  DeleteBody(Body* pBody);

#if defined(NES_DEBUG) && defined(NES_LOGGING_ENABLED)
        //----------------------------------------------------------------------------------------------------
        /// @brief : Function to ensure that the free list is not corrupted.
        //----------------------------------------------------------------------------------------------------
        void                                ValidateFreeList() const;
#endif
    };
}
