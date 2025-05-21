// SceneManager.h
#pragma once
#include <filesystem>
#include <unordered_map>
#include "Scene.h"
#include "TickManager.h"
#include "Core/Memory/StrongPtr.h"
#include "Core/String/StringID.h"

namespace nes
{
    class Scene;
    class Application;

    //----------------------------------------------------------------------------------------------------
    //		NOTES:
    //		
    ///		@brief : Manages the loading of and transitioning between Scenes.
    //----------------------------------------------------------------------------------------------------
    class SceneManager
    {
        friend Application;

        struct SceneData
        {
            std::filesystem::path m_scenePath{};   // Path to the scene on disk...
            StrongPtr<Scene> m_pScene{};           // Scene Resource.
        };
        
        using SceneMap = std::unordered_map<StringID, SceneData, StringIDHasher>;

        SceneMap m_sceneMap{};
        StrongPtr<Scene> m_pActiveScene{}; // At the moment, we only have a single scene.
        StringID m_sceneToTransitionTo{};
        TickManager m_tickManager{};

    public:
        SceneManager() = default;
        ~SceneManager() = default;
        SceneManager(const SceneManager&) = delete;
        SceneManager& operator=(const SceneManager&) = delete;
        SceneManager(SceneManager&&) noexcept = delete;
        SceneManager& operator=(SceneManager&&) noexcept = delete;
        
        static void QueueSceneTransition(const StringID& sceneName);
        static StrongPtr<Scene> GetActiveScene();
        static bool IsTransitionQueued();
    
    private:
        bool Init(YAML::Node& applicationSettings);
        void Shutdown();
        void PreRender();
        void Render();
        void Update(const double deltaRealTime);
        void OnEvent(Event& event);
        bool TransitionToScene();
    };
}
