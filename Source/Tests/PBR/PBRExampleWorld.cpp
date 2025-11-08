// PBRExampleWorld.cpp
#include "PBRExampleWorld.h"

#include "Nessie/World/ComponentSystems/TransformSystem.h"
#include "Nessie/World/ComponentSystems/FreeCamSystem.h"
#include "ComponentSystems/DayNightSystem.h"
#include "ComponentSystems/PBRSceneRenderer.h"
#include "Nessie/Graphics/Shader.h"
#include "Nessie/Graphics/Texture.h"

namespace pbr
{
    bool PBRExampleWorld::PostInit()
    {
        NES_LOG("World Initialized!");
        return true;
    }

    void PBRExampleWorld::OnDestroy()
    {
        m_pTransformSystem = nullptr;
        m_pSceneRenderer = nullptr;
        m_pDayNightSystem = nullptr;
        m_pFreeCamSystem = nullptr;
    }

    void PBRExampleWorld::OnEvent(nes::Event& event)
    {
        m_pFreeCamSystem->OnEvent(event);
        m_pDayNightSystem->OnEvent(event);
    }

    void PBRExampleWorld::Tick(const float deltaTime)
    {
        ProcessEntityLifecycle();
        
        m_pTransformSystem->UpdateHierarchy();
        
        m_pDayNightSystem->Tick(deltaTime);
        m_pFreeCamSystem->Tick(deltaTime);
    }

    void PBRExampleWorld::OnResize(const uint32 width, const uint32 height)
    {
        if (m_pSceneRenderer)
            m_pSceneRenderer->ResizeRenderTargets(width, height);
    }

    void PBRExampleWorld::Render(nes::CommandBuffer& commandBuffer, const nes::RenderFrameContext& context)
    {
        if (m_pSceneRenderer)
            m_pSceneRenderer->RenderScene(commandBuffer, context);
    }

    void PBRExampleWorld::ParentEntity(const nes::EntityHandle entity, const nes::EntityHandle parent)
    {
        if (m_pTransformSystem)
            m_pTransformSystem->SetParent(entity, parent);
    }

    void PBRExampleWorld::OnNewEntityCreated(const nes::EntityHandle newEntity)
    {
        m_entityRegistry.AddComponent<nes::NodeComponent>(newEntity);
        m_entityRegistry.AddComponent<nes::TransformComponent>(newEntity);
    }

    void PBRExampleWorld::AddComponentSystems()
    {
        m_pTransformSystem = AddComponentSystem<nes::TransformSystem>();
        m_pSceneRenderer = AddComponentSystem<PBRSceneRenderer>();
        m_pDayNightSystem = AddComponentSystem<DayNightSystem>();
        m_pFreeCamSystem = AddComponentSystem<nes::FreeCamSystem>();
    }
}
