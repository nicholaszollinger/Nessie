// WorldManager.cpp
#include "WorldManager.h"

#include <BleachNew.h>

namespace nes
{
    bool WorldManager::Init()
    {
        // [TODO]: 
        return true;
    }

    //----------------------------------------------------------------------------------------------------
    //		NOTES:
    //		
    ///		@brief : Update the Active World, and handle any World Transitions.
    ///		@param deltaRealTime : 
    //----------------------------------------------------------------------------------------------------
    void WorldManager::Update(const double deltaRealTime)
    {
        if (m_pActiveWorld)
        {
            m_pActiveWorld->Update(deltaRealTime);
        }

        // If a World Transition is queued, transition to that World.
        if (IsTransitionQueued())
        {
            TransitionToWorld();
        }
    }

    //----------------------------------------------------------------------------------------------------
    //		NOTES:
    //		
    ///		@brief : Close the World Manager.
    //----------------------------------------------------------------------------------------------------
    void WorldManager::Close()
    {
        if (m_pActiveWorld)
        {
            m_pActiveWorld->Destroy();
            BLEACH_DELETE(m_pActiveWorld);
            m_pActiveWorld = nullptr;
        }

        // Clear the Entity Registry.
        m_registry.Clear();
    }

    //----------------------------------------------------------------------------------------------------
    //		NOTES:
    //		
    ///		@brief : Queue the transition to a new World. If the world is not one of the available Worlds,
    ///              this will do nothing. If the World is not loaded in memory, the actual transition will
    ///              occur when the destination World is loaded.
    ///		@param worldName : Name of the World to transition to.
    //----------------------------------------------------------------------------------------------------
    void WorldManager::QueueWorldTransition([[maybe_unused]] const StringID& worldName)
    {
        // [TODO]: Should I fail here? Throw an Error?
        if (IsTransitionQueued())
            return;

        //m_worldToTransitionTo = worldID;
    }

    //----------------------------------------------------------------------------------------------------
    ///		@brief : Returns the Active World.
    //----------------------------------------------------------------------------------------------------
    World* WorldManager::GetActiveWorld() const
    {
        return m_pActiveWorld;
    }

    //----------------------------------------------------------------------------------------------------
    ///		@brief : Returns if a World Transition is Queued.
    //----------------------------------------------------------------------------------------------------
    bool WorldManager::IsTransitionQueued() const
    {
        return m_worldToTransitionTo != StringID::GetInvalid();
    }

    //----------------------------------------------------------------------------------------------------
    //		NOTES:
    //		
    ///		@brief : Handle the World Transition.
    //----------------------------------------------------------------------------------------------------
    void WorldManager::TransitionToWorld()
    {
        // [TODO]: 

        // Clear the World to Transition to.
        m_worldToTransitionTo = StringID::GetInvalid();
    }
}
