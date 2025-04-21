// BodyManager.h
#pragma once
#include "Body.h"

namespace nes
{
    enum class AddBodyActivationMode
    {   
        Activate,    /// Activate the Body, making it part of the simulation.
        LeaveAsIs,   /// Leave the activation state as it is. This will not deactivate an active body!
    };

    using BodyArray = std::vector<Body*>;
    
    class BodyManager
    {
        friend class PhysicsScene;

    public:
        struct BodyStats
        {
            uint32_t m_numBodies                = 0;
            uint32_t m_maxNumBodies             = 0;
            uint32_t m_numStaticBodies          = 0;
            uint32_t m_numBodiesDynamic         = 0;
            uint32_t m_numActiveBodiesDynamic   = 0;
            uint32_t m_numKinematicBodies       = 0;
            uint32_t m_numActiveKinematicBodies = 0;
        };

    private:
        BodyArray m_bodies;
        BodyStats m_stats;
        
    public:
        BodyManager() = default;
        BodyManager(const BodyManager&) = delete;
        BodyManager& operator=(const BodyManager&) = delete;
        BodyManager(BodyManager&&) noexcept = delete;
        BodyManager& operator=(BodyManager&&) noexcept = delete;

        void Init(uint32_t maxBodies, uint32_t numBodyMutexes, const BroadPhaseLayerInterface& interface);

        Body*               CreateBody(const BodyCreateInfo& createInfo) const;
        void                AddBody(Body* pBody);
        void                RemoveBody(Body* pBody);
        void                DestroyBody(Body* body) const;
        void                LockAllBodies();
        void                UnlockAllBodies();
        uint32_t            GetNumActiveBodies() const;
        const BodyArray&    GetBodies() const               { return m_bodies; }
        BodyArray&          GetBodies()                     { return m_bodies; }
        uint32_t            GetMaxBodies() const            { return static_cast<uint32_t>(m_bodies.capacity());}
        const BodyStats&    GetStats() const;
    };
}
