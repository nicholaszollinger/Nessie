#pragma once
// WorldManager.h
#include <unordered_map>
#include "World.h"
#include "Core/String/StringID.h"

namespace nes
{
    class World;
    class Application;

    //----------------------------------------------------------------------------------------------------
    //		NOTES:
    //		
    ///		@brief : Manages the loading of and transitioning between Worlds.
    //----------------------------------------------------------------------------------------------------
    class WorldManager
    {
        friend Application;

        //std::unordered_map<StringID, World*, StringIDHasher> m_loadedWorlds;
        //std::unordered_map<StringID, WorldMetaData, StringIDHasher> m_availableWorlds{};
        EntityRegistry m_registry;
        World* m_pActiveWorld = nullptr;
        StringID m_worldToTransitionTo;

    public:
        WorldManager() = default;
        ~WorldManager() = default;
        WorldManager(const WorldManager&) = delete;
        WorldManager& operator=(const WorldManager&) = delete;
        WorldManager(WorldManager&&) noexcept = delete;
        WorldManager& operator=(WorldManager&&) noexcept = delete;

        void QueueWorldTransition(const StringID& worldName);
        World* GetActiveWorld() const;
        bool IsTransitionQueued() const;

    private:
        bool Init();
        void Close();
        void Update(const double deltaRealTime);
        void TransitionToWorld();
        //void HandleWorldLoad(ResourcePtr<Resource> pResource);
    };
}