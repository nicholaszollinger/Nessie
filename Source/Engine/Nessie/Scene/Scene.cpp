// Scene.cpp
#include "Scene.h"
#include <yaml-cpp/yaml.h>
#include "Debug/Assert.h"
#include "World/World.h"

namespace nes
{
    bool Scene::Init()
    {
        // Layers should already be added when loading the World.
        NES_ASSERTV(!m_layerStack.empty(), "World contains no Layers!");

        // Initialize layers.
        for (auto& pLayer : m_layerStack)
        {
            if (!pLayer->InitializeLayer())
            {
                return false;
            }
        }
        
        return true;
    }

    //----------------------------------------------------------------------------------------------------
    ///		@brief : Begin running the World. 
    //----------------------------------------------------------------------------------------------------
    bool Scene::Begin()
    {
        for (auto& pLayer : m_layerStack)
        {
            pLayer->OnSceneBegin();
        }

        // m_isSimulating = true;?

        return true;
    }

    //----------------------------------------------------------------------------------------------------
    //		NOTES:
    //		
    ///		@brief : Destroy the World, clearing all Entities that are not marked as Persistent and
    ///              destroying all Systems.
    //----------------------------------------------------------------------------------------------------
    void Scene::Destroy()
    {
        m_tickFunctions.clear();

        for (auto& pLayer : m_layerStack)
        {
            pLayer->DestroyLayer();
            pLayer.Reset();
        }

        m_layerStack.clear();

        NES_LOGV("Scene", "Destroy() Complete");
    }

    //----------------------------------------------------------------------------------------------------
    //		NOTES:
    //		
    ///		@brief : Ticks each layer in the World, from the bottom to the top of the Layer stack. 
    ///		@param deltaRealTime : 
    //----------------------------------------------------------------------------------------------------
    void Scene::Tick(const double deltaRealTime)
    {
        if (UpdateTime(deltaRealTime))
        {
            // Run registered physics tick???
        }

        // [TODO]: Should these be managed by the Tick Functions?
        for (auto& pLayer : m_layerStack)
        {
            pLayer->Tick(m_sceneDeltaTime);
        }

        // Process any destroyed entities as a result of the update:
        // for (auto& pLayer : m_layerStack)
        // {
        //     pLayer->ProcessDestroyedEntities();
        // }
    }

    //----------------------------------------------------------------------------------------------------
    //		NOTES:
    //		
    ///		@brief : Renders each Layer in the World, from the bottom to the top of the Layer stack. 
    //----------------------------------------------------------------------------------------------------
    void Scene::Render()
    {
        if (!m_pActiveCamera)
            return;
        
        for (auto& pLayer : m_layerStack)
        {
            pLayer->Render(*m_pActiveCamera);
        }
    }

    //----------------------------------------------------------------------------------------------------
    ///		@brief : Propagates Events from the top of the layer stack to the bottom. 
    //----------------------------------------------------------------------------------------------------
    void Scene::OnEvent(Event& event)
    {
        // Process input listeners???

        for (auto it = m_layerStack.rbegin(); it != m_layerStack.rend(); ++it)
        {
            (*it)->OnEvent(event);
        }
    }

    //----------------------------------------------------------------------------------------------------
    //      [TODO]:
    //      There is no identification as a part of registering, so we can't easily unregister.
    //
    ///		@brief : Register a Tick function to be processed in the next Update. 
    //----------------------------------------------------------------------------------------------------
    void Scene::RegisterTickFunction(const TickFunction& function)
    {
        m_tickFunctions.emplace_back(function);
    }

    //----------------------------------------------------------------------------------------------------
    ///		@brief : Hook into events. 
    //----------------------------------------------------------------------------------------------------
    void Scene::RegisterEventHandler(const EventHandler& eventHandler)
    {
        m_eventHandlers.emplace_back(eventHandler);
    }

    void Scene::SetActiveCamera(const Camera* camera)
    {
        m_pActiveCamera = camera;
    }

    void Scene::PushLayer(const StrongPtr<EntityLayer>& pLayer)
    {
        m_layerStack.push_back(pLayer);
    }

    //----------------------------------------------------------------------------------------------------
    //		NOTES:
    //		
    ///		@brief : Updates the World Delta Time & Real Time, and checks if it's time for a Fixed Update.
    ///		@param deltaRealTime : Real Time elapsed since the last frame.
    ///		@returns : True if it's time for a Fixed Update, False otherwise.
    //----------------------------------------------------------------------------------------------------
    bool Scene::UpdateTime(const double deltaRealTime)
    {
        m_realTimeElapsed += deltaRealTime;
        m_sceneDeltaTime = static_cast<float>(deltaRealTime) * m_worldTimeScale;
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
    ///		@brief : Set the global timescale of the World.
    ///		@param timeScale : Timescale to set. 1.0f is no scaling, 0.5f is half speed, 2.0f is double speed.
    ///     @note : A timescale of 0 will make DeltaTime always equal to 0.
    ///     @note : However, Fixed Updates are not effected by the timescale. If you want a fixed
    ///     @note : system to be effected by the timescale, you must manually scale the fixed time step
    ///     @note : in the system.
    //----------------------------------------------------------------------------------------------------
    void Scene::SetGlobalTimeScale(const float timeScale)
    {
        m_worldTimeScale = timeScale;
    }

    //----------------------------------------------------------------------------------------------------
    ///		@brief : Try to Load a scene from a filepath.
    //----------------------------------------------------------------------------------------------------
    bool Scene::Load(const std::filesystem::path& scenePath)
    {
        std::string fullPath = NES_CONTENT_DIR;
        fullPath += scenePath.string();
        
        YAML::Node file = YAML::LoadFile(fullPath);
        if (!file)
        {
            NES_ERRORV("Scene", "Failed to load scene! Filepath invalid!");
            return false;
        }
        
        auto sceneNode = file["Scene"];
        NES_ASSERT(sceneNode);
        m_name = sceneNode["Name"].as<std::string>();
        
        auto layers = sceneNode["Layers"];
        NES_ASSERT(layers);

        // [HACK]: For now, just checking for the World Layer:
        if (auto worldNode = layers["World"])
        {
            // A EntityLayerFactory should handle creating the EntityLayer.
            StrongPtr<World> pWorld = StrongPtr<World>::Create(this);
            
            StrongPtr<EntityLayer> pLayer = pWorld.Cast<EntityLayer>();
            pLayer->LoadLayer(worldNode);
            PushLayer(pLayer);
        }
        
        // [TODO]: 
        // for (auto layerNode : layers)
        // {
        //     // Get the Layer Name:
        //     const auto name = layerNode.first.as<std::string>();
        //
        //     // [TODO]: Use the EntityLayerFactory to load the Layer.
        //     // - I need to use the Game one.
        // }

        return true;
    }
}
