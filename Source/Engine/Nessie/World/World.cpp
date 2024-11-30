// World.cpp

#include "World.h"
#include "Components/HierarchyComponent.h"
#include "Components/NameComponent.h"
#include "Debug/Assert.h"

namespace nes
{
    //----------------------------------------------------------------------------------------------------
    //		NOTES:
    //		
    ///		@brief : Create a new Entity with an IDComponent and a TagComponent.
    ///		@param name : Name of the Entity.
    ///		@returns : The newly created Entity.
    //----------------------------------------------------------------------------------------------------
    Entity World::CreateEntity(const std::string& name)
    {
        NES_ASSERT(m_pRegistry);

        Entity entity = m_pRegistry->CreateEntity();
        entity.AddComponent<IDComponent>(); // Generates a new ID for the Entity.

        if (!name.empty())
        {
            entity.AddComponent<NameComponent>();
        }

        return entity;
    }

    //----------------------------------------------------------------------------------------------------
    //		NOTES:
    //		
    ///		@brief : Create an Entity with a specified ID and name.
    ///		@param id : ID to give the new Entity.
    ///		@param name : Name of the Entity.
    ///		@returns : The new Entity.
    //----------------------------------------------------------------------------------------------------
    Entity World::CreateEntityWithID(const EntityID id, const std::string& name)
    {
        NES_ASSERT(m_pRegistry);

        Entity entity = m_pRegistry->CreateEntity();

        // Add a IDComponent with the given ID.
        entity.AddComponent<IDComponent>(id);

        if (!name.empty())
        {
            entity.AddComponent<NameComponent>();
        }

        return entity;
    }

    void World::Update(const double deltaRealTime)
    {
        if (UpdateTime(deltaRealTime))
        {
            // Fixed Update
            /*for (auto& system : m_systems)
            {
                system->FixedUpdate();
            }*/
        }

        // Update
        /*for (auto& system : m_systems)
        {
            system->Update();
        }*/
    }

    //----------------------------------------------------------------------------------------------------
    //		NOTES:
    //		
    ///		@brief : Updates the World Delta Time & Real Time, and checks if it's time for a Fixed Update.
    ///		@param deltaRealTime : Real Time elapsed since the last frame.
    ///		@returns : True if it's time for a Fixed Update, False otherwise.
    //----------------------------------------------------------------------------------------------------
    bool World::UpdateTime(const double deltaRealTime)
    {
        m_realTimeElapsed += deltaRealTime;
        m_worldDeltaTime = static_cast<float>(deltaRealTime) * m_worldTimeScale;
        m_timeLeftForFixed -= deltaRealTime;

        if (m_timeLeftForFixed < 0.0)
        {
            m_timeLeftForFixed = m_fixedTimeStep;
            return true;
        }

        return false;
    }

    //----------------------------------------------------------------------------------------------------
    //		NOTES:
    //		
    ///		@brief : Destroy the World, clearing all Entities that are not marked as Persistent and
    ///              destroying all Systems.
    //----------------------------------------------------------------------------------------------------
    void World::Destroy()
    {
        // [TODO]: Destroy all Entities not marked as Persistent.
        // [TODO]: Destroy all Systems.
    }

    //----------------------------------------------------------------------------------------------------
    //		NOTES:
    //				
    ///		@brief : Set the global timescale of the World.
    ///		@param timeScale : Timescale to set. 1.0f is no scaling, 0.5f is half speed, 2.0f is double speed.
    ///     @note : A timescale of 0 will make DeltaTime always equal to 0.
    ///     @note : However, Fixed Updates are not effected by the timescale. If you want a fixed
    ///     @note : system to be effected by the timescale, you must manually scale the fixed time step
    ///     @note : in the system.
    //----------------------------------------------------------------------------------------------------
    void World::SetGlobalTimeScale(const float timeScale)
    {
        m_worldTimeScale = timeScale;
    }
}