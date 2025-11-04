// TestWorld.cpp
#include "TestWorld.h"
#include "SimpleRenderer.h"

void TestWorld::Tick(const float)
{
    ProcessEntityLifecycle();
    m_pTransformSystem->UpdateHierarchy();
}

void TestWorld::Render(nes::CommandBuffer& commandBuffer, const nes::RenderFrameContext& context) const
{
    m_pSimpleRenderer->RenderWorld(commandBuffer, context);
}

void TestWorld::AddComponentSystems()
{
    m_pTransformSystem = AddComponentSystem<nes::TransformSystem>();
    m_pSimpleRenderer = AddComponentSystem<SimpleRenderer>();

    // Set our Renderer.
    m_pRenderer = nes::Cast<nes::WorldRenderer>(m_pSimpleRenderer);
}

bool TestWorld::PostInit()
{
    NES_LOG("TestWorld Initialized!");
    return true;
}

void TestWorld::OnDestroy()
{
    m_pSimpleRenderer = nullptr;
    m_pTransformSystem = nullptr;
}
