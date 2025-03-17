// SceneManager.cpp
#include "SceneManager.h"
#include "Application/Application.h"

namespace nes
{
    bool SceneManager::Init(YAML::Node& applicationSettings)
    {
        auto sceneManager = applicationSettings["SceneManager"];
        if (!sceneManager)
        {
            NES_ERRORV("Application", "Failed to find SceneManager in Application Settings file!");
            return false;
        }

        // Load the Scene Map:
        auto sceneMap = sceneManager["SceneMap"];
        if (!sceneMap)
        {
            NES_ERRORV("Application", "Failed to find SceneMap in Application Settings file!");
            return false;
        }

        for (auto sceneNode : sceneMap)
        {
            StringID sceneName = sceneNode["Name"].as<std::string>();
            std::filesystem::path scenePath = sceneNode["Path"].as<std::string>();
            m_sceneMap.emplace(sceneName, SceneData(scenePath, StrongPtr<Scene>::Create()));
        }

        // Get the Start Scene info:
        auto startScene = sceneManager["StartScene"];
        if (!startScene)
        {
            NES_ERRORV("Application", "Failed to find StartScene in SceneManager!");
            return false;
        }

        StringID startSceneName = startScene["Runtime"].as<std::string>();
        NES_ASSERT(m_sceneMap.contains(startSceneName));
        
        // Create/Load the start scene:
        m_sceneToTransitionTo = startSceneName;
        TransitionToScene();
        
        return true;
    }

    //----------------------------------------------------------------------------------------------------
    ///		@brief : Update the active Scene, then handle any scene transitions.
    //----------------------------------------------------------------------------------------------------
    void SceneManager::Update(const double deltaRealTime)
    {
        if (m_pActiveScene)
        {
            m_pActiveScene->Tick(deltaRealTime);
        }

        // If a World Transition is queued, transition to that World.
        if (IsTransitionQueued())
        {
            TransitionToScene();
        }
    }

    //----------------------------------------------------------------------------------------------------
    ///		@brief : Propagate Events to the active Scene. 
    //----------------------------------------------------------------------------------------------------
    void SceneManager::OnEvent(Event& event)
    {
        if (m_pActiveScene)
        {
            m_pActiveScene->OnEvent(event);
        }
    }

    //----------------------------------------------------------------------------------------------------
    ///		@brief : Close the SceneManager.
    //----------------------------------------------------------------------------------------------------
    void SceneManager::Shutdown()
    {
        m_pActiveScene.Reset();
        
        // Destroy all loaded scene assets:
        for (auto& [_, sceneData] : m_sceneMap)
        {
            if (sceneData.m_pScene)
            {
                sceneData.m_pScene->Destroy();
                sceneData.m_pScene.Reset();
            }
        }
        
        m_sceneMap.clear();
    }

    void SceneManager::PreRender()
    {
        if (m_pActiveScene)
            m_pActiveScene->PreRender();
    }

    void SceneManager::Render()
    {
        if (m_pActiveScene)
            m_pActiveScene->Render();
    }

    //----------------------------------------------------------------------------------------------------
    //		NOTES:
    //		
    ///		@brief : Queue the transition to a new Scene. If the Scene is not one of the available scenes,
    ///              this will do nothing. If the Scene is not loaded in memory, the actual transition will
    ///              occur when the destination Scene is loaded.
    ///		@param sceneName : Name of the Scene to transition to.
    //----------------------------------------------------------------------------------------------------
    void SceneManager::QueueSceneTransition([[maybe_unused]] const StringID& sceneName)
    {
        auto& worldManager = Application::Get().GetSceneManager();
        
        // [TODO]: Should I fail here? Throw an Error?
        if (worldManager.m_sceneToTransitionTo != StringID::GetInvalidID())
            return;
        
        worldManager.m_sceneToTransitionTo = sceneName;
    }

    //----------------------------------------------------------------------------------------------------
    ///		@brief : Returns the Active World.
    //----------------------------------------------------------------------------------------------------
    WeakPtr<Scene> SceneManager::GetActiveScene()
    {
        return Application::Get().GetSceneManager().m_pActiveScene;
    }

    //----------------------------------------------------------------------------------------------------
    ///		@brief : Returns if a World Transition is Queued.
    //----------------------------------------------------------------------------------------------------
    bool SceneManager::IsTransitionQueued()
    {
        const auto& sceneManager = Application::Get().GetSceneManager();
        return sceneManager.m_sceneToTransitionTo != StringID::GetInvalidID();
    }

    //----------------------------------------------------------------------------------------------------
    //		NOTES:
    //		
    ///		@brief : Handle the World Transition.
    //----------------------------------------------------------------------------------------------------
    bool SceneManager::TransitionToScene()
    {
        NES_LOGV("SceneManager", "Transitioning to Scene: ", m_sceneToTransitionTo.CStr());
        
        if (m_pActiveScene)
        {
            m_pActiveScene->Destroy();
        }

        NES_ASSERT(m_sceneMap.contains(m_sceneToTransitionTo));
        const SceneData& sceneData = m_sceneMap.at(m_sceneToTransitionTo);
        m_pActiveScene = sceneData.m_pScene;

        if (!m_pActiveScene->Load(sceneData.m_scenePath))
        {
            NES_ERRORV("Application", "Failed to load Scene: ", m_sceneToTransitionTo.CStr());
            return false;
        }

        if (!m_pActiveScene->Init())
        {
            NES_ERRORV("Application", "Failed to load Scene: ", m_sceneToTransitionTo.CStr());
            return false;
        }
        
        m_sceneToTransitionTo = StringID::GetInvalidID();
        m_pActiveScene->Begin();
        
        return true;
    }
}
