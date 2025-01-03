#pragma once
// World.h
#include "Entity.h"
#include "Components/IDComponent.h"

namespace nes
{
    //----------------------------------------------------------------------------------------------------
    //		NOTES:
    //      TODO: World Layers
    //      - UILayer vs PhysicsLayer, etc.
    //      - This is a way to separate different update/rendering systems through different
    //        collections of ComponentSystems. A Layer can have any number of these systems, and
    //        process them individually.
    //		
    ///		@brief : A World represents a collection of Entities, and the Systems that process them.
    //----------------------------------------------------------------------------------------------------
    class World
    {
        static constexpr float kDefaultFixedTimeStep = (1.f / 60.f);

        friend class WorldManager;

        // [TODO]: System Arrays.

        EntityRegistry* m_pRegistry = nullptr;
        StringID m_name;

        // TimeInfo:
        double m_realTimeElapsed = 0.0f;                // The amount of Time elapsed since the start of the Application.
        double m_timeLeftForFixed = 0.f;                // The amount of Time left before the next Fixed Update is run.
        float m_fixedTimeStep = kDefaultFixedTimeStep;  // The Interval at which the Fixed Update is run.
        float m_worldTimeScale = 1.f;                   // Current Time Scale applied to the World.
        float m_worldDeltaTime = 0.f;                   // Current Delta Time of the World, scaled by the World Time Scale.

    public:
        World() = default;
        World(const World&) = delete;
        World& operator=(const World&) = delete;
        World(World&&) noexcept = delete;
        World& operator=(World&&) noexcept = delete;

        // Entities:
        Entity CreateEntity(const std::string& name);
        Entity CreateEntityWithID(const EntityID id, const std::string& name);

        // [TODO]: 
        // Systems:

        // Time:
        void SetGlobalTimeScale(const float timeScale);
        [[nodiscard]] float GetDeltaTime() const        { return m_worldDeltaTime; }
        [[nodiscard]] float GetTimeScale() const        { return m_worldTimeScale; }
        [[nodiscard]] float GetFixedTimeStep() const    { return m_fixedTimeStep; }

    private:
        void Update(const double deltaRealTime);
        bool UpdateTime(const double deltaRealTime);
        void Destroy();
    };
}