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
        if (IsSimulating() && !IsPaused())
        {
            m_pFreeCamSystem->OnEvent(event);
        }
    }

    void PBRExampleWorld::Tick(const float deltaTime)
    {
        ProcessEntityLifecycle();
        m_pTransformSystem->UpdateHierarchy();

        if (IsSimulating())
        {
            m_pDayNightSystem->Tick(deltaTime);
            m_pFreeCamSystem->Tick(deltaTime);
        }
    }

    void PBRExampleWorld::ParentEntity(const nes::EntityHandle entity, const nes::EntityHandle parent)
    {
        if (m_pTransformSystem)
            m_pTransformSystem->SetParent(entity, parent);
    }

    void PBRExampleWorld::OnNewEntityCreated(nes::EntityRegistry& registry, const nes::EntityHandle newEntity)
    {
        registry.AddComponent<nes::NodeComponent>(newEntity);
        registry.AddComponent<nes::TransformComponent>(newEntity);
    }

    void PBRExampleWorld::AddComponentSystems()
    {
        m_pTransformSystem = AddComponentSystem<nes::TransformSystem>();
        m_pSceneRenderer = AddComponentSystem<PBRSceneRenderer>();
        m_pDayNightSystem = AddComponentSystem<DayNightSystem>();
        m_pFreeCamSystem = AddComponentSystem<nes::FreeCamSystem>();
    }
}
