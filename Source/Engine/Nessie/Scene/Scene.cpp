// Scene.cpp
#include "Scene.h"
#include <yaml-cpp/yaml.h>
#include "Debug/Assert.h"
#include "World/World.h"

namespace nes
{
    bool Scene::Init()
    {
        m_isBeingDestroyed = false;
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
        m_isBeingDestroyed = true;

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
    ///		@brief : Run any cleanup operations after Ticking has finished.  
    //----------------------------------------------------------------------------------------------------
    void Scene::OnPostTick()
    {
        for (auto& pLayer : m_layerStack)
        {
            pLayer->OnPostTick();
        }
    }

    //----------------------------------------------------------------------------------------------------
    ///		@brief : Called before Render on each layer in the stack, from bottom to top.
    //----------------------------------------------------------------------------------------------------
    void Scene::PreRender()
    {
        if (!m_pActiveCamera)
            return;
        
        for (auto& pLayer : m_layerStack)
        {
            pLayer->PreRender(*m_pActiveCamera);
        }
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

    void Scene::SetActiveCamera(const Camera* camera)
    {
        m_pActiveCamera = camera;
    }

    void Scene::PushLayer(const StrongPtr<EntityLayer>& pLayer)
    {
        m_layerStack.push_back(pLayer);
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
