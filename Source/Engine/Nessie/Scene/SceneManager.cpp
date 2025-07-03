// SceneManager.cpp
#include "SceneManager.h"
#include <yaml-cpp/yaml.h>
#include "Nessie/Application/Application.h"

namespace nes
{
    static SceneManager* g_pSceneManager = nullptr;
    
    bool SceneManager::Init(YAML::Node& applicationSettings)
    {
        NES_ASSERT(g_pSceneManager == nullptr);
        g_pSceneManager = this;
        
        auto sceneManager = applicationSettings["SceneManager"];
        if (!sceneManager)
        {
            NES_ERROR(kApplicationLogTag, "Failed to find SceneManager in Application Settings file!");
            return false;
        }

        // Load the Scene Map:
        auto sceneMap = sceneManager["SceneMap"];
        if (!sceneMap)
        {
            NES_ERROR(kApplicationLogTag, "Failed to find SceneMap in Application Settings file!");
            return false;
        }

        for (auto sceneNode : sceneMap)
        {
            StringID sceneName = sceneNode["Name"].as<std::string>();
            std::filesystem::path scenePath = sceneNode["Path"].as<std::string>();
            m_sceneMap.emplace(sceneName, SceneData(scenePath, Create<Scene>()));
        }

        // Get the Start Scene info:
        auto startScene = sceneManager["StartScene"];
        if (!startScene)
        {
            NES_ERROR(kSceneLogTag, "Failed to find StartScene in SceneManager!");
            return false;
        }

        StringID startSceneName = startScene["Runtime"].as<std::string>();
        NES_ASSERT(m_sceneMap.contains(startSceneName));

        if (!m_tickManager.Init())
        {
            NES_ERROR(kSceneLogTag, "Failed to initialize tick manager!");
            return false;
        }
        
        // Create/Load the start scene:
        m_sceneToTransitionTo = startSceneName;
        TransitionToScene();
        
        return true;
    }

    //----------------------------------------------------------------------------------------------------
    ///		@brief : Run a tick frame, then transition
    //----------------------------------------------------------------------------------------------------
    void SceneManager::Update(const double deltaRealTime)
    {
        // Run each stage of the Tick:
        // Right now this all runs synchronously.
        {
            m_tickManager.BeginFrame(static_cast<float>(deltaRealTime));
            m_tickManager.RunTickStage(ETickStage::PrePhysics);
            m_tickManager.RunTickStage(ETickStage::Physics);
            m_tickManager.RunTickStage(ETickStage::PostPhysics);
            m_tickManager.RunTickStage(ETickStage::Late);
            m_tickManager.EndFrame();
        }
        
        m_pActiveScene->OnPostTick();

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
        m_tickManager.Shutdown();

        g_pSceneManager = nullptr;
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
        NES_ASSERT(g_pSceneManager != nullptr);
        auto& sceneManager = *g_pSceneManager;
        
        // [TODO]: Should I fail here? Throw an Error?
        if (sceneManager.m_sceneToTransitionTo != StringID::GetInvalidID())
            return;
        
        sceneManager.m_sceneToTransitionTo = sceneName;
    }

    //----------------------------------------------------------------------------------------------------
    ///		@brief : Returns the Active World.
    //----------------------------------------------------------------------------------------------------
    StrongPtr<Scene> SceneManager::GetActiveScene()
    {
        NES_ASSERT(g_pSceneManager != nullptr);
        return g_pSceneManager->m_pActiveScene;
    }

    //----------------------------------------------------------------------------------------------------
    ///		@brief : Returns if a World Transition is Queued.
    //----------------------------------------------------------------------------------------------------
    bool SceneManager::IsTransitionQueued()
    {
        NES_ASSERT(g_pSceneManager != nullptr);
        return g_pSceneManager->m_sceneToTransitionTo != StringID::GetInvalidID();
    }

    //----------------------------------------------------------------------------------------------------
    //		NOTES:
    //		
    ///		@brief : Handle the World Transition.
    //----------------------------------------------------------------------------------------------------
    bool SceneManager::TransitionToScene()
    {
        NES_LOG(kSceneLogTag, "Transitioning to Scene: {}", m_sceneToTransitionTo.CStr());
        
        if (m_pActiveScene)
        {
            m_pActiveScene->Destroy();
        }

        NES_ASSERT(m_sceneMap.contains(m_sceneToTransitionTo));
        const SceneData& sceneData = m_sceneMap.at(m_sceneToTransitionTo);
        m_pActiveScene = sceneData.m_pScene;

        if (!m_pActiveScene->Load(sceneData.m_scenePath))
        {
            NES_ERROR(kSceneLogTag, "Failed to load Scene: ", m_sceneToTransitionTo.CStr());
            return false;
        }

        if (!m_pActiveScene->Init())
        {
            NES_ERROR(kSceneLogTag, "Failed to load Scene: ", m_sceneToTransitionTo.CStr());
            return false;
        }
        
        m_sceneToTransitionTo = StringID::GetInvalidID();
        m_pActiveScene->Begin();
        
        return true;
    }
}
