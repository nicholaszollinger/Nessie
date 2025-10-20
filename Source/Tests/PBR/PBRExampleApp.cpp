// PBRExampleApp.cpp
#include "PBRExampleApp.h"
#include "Nessie/Application/EntryPoint.h"
#include "Nessie/Graphics/Renderer.h"
#include "Nessie/Graphics/Texture.h"
#include "Nessie/Graphics/Shader.h"

void PBRExampleApp::OnEvent(nes::Event& e)
{
    if (m_pWorld)
        m_pWorld->OnEvent(e);
}

bool PBRExampleApp::Internal_AppInit()
{
    NES_REGISTER_ASSET_TYPE(nes::Shader);
    NES_REGISTER_ASSET_TYPE(nes::Texture);
    NES_REGISTER_ASSET_TYPE(nes::TextureCube);
    NES_REGISTER_ASSET_TYPE(pbr::MeshAsset);
    NES_REGISTER_ASSET_TYPE(pbr::PBRMaterial);
    NES_REGISTER_ASSET_TYPE(nes::WorldAsset);
    
    m_pWorld = nes::Create<pbr::PBRExampleWorld>();
    if (!m_pWorld->Init())
    {
        NES_ERROR("Failed to initialize PBR World!");
        return false;
    }

    // Load the World Asset
    m_worldLoaded = false;
    std::filesystem::path path = NES_CONTENT_DIR;
    path /= "Worlds/PBRTestWorld.yaml";
    if (nes::AssetManager::LoadSync<nes::WorldAsset>(m_worldAssetID, path) != nes::ELoadResult::Success)
    {
        NES_ERROR("Failed to load World Asset!");
        return false;
    }

    auto pWorldAsset = nes::AssetManager::GetAsset<nes::WorldAsset>(m_worldAssetID);
    NES_ASSERT(pWorldAsset);
    auto& assetPack = pWorldAsset->GetAssetPack();

    // Load the World's Assets, asynchronously:
    auto onComplete = [this](const bool succeeded)
    {
        if (succeeded)
        {
            NES_LOG("World load successful!");
    
            // Merge the entities from the World Asset into the runtime world.
            auto pWorldAsset = nes::AssetManager::GetAsset<nes::WorldAsset>(m_worldAssetID);
            NES_ASSERT(pWorldAsset);
            m_pWorld->MergeWorld(*pWorldAsset);
        }
        else
        {
            NES_ERROR("Failed to load World!");
            Quit();
        }
    };
    
    nes::AssetManager::LoadAssetPackAsync(assetPack, onComplete);
    
    return true;
}

void PBRExampleApp::Internal_AppUpdate(const float timeStep)
{
    if (m_pWorld)
        m_pWorld->Tick(timeStep);
}

void PBRExampleApp::Internal_OnResize(const uint32 width, const uint32 height)
{
    if (m_pWorld)
        m_pWorld->OnResize(width, height);
}

void PBRExampleApp::Internal_AppRender(nes::CommandBuffer& commandBuffer, const nes::RenderFrameContext& context)
{
    if (m_pWorld)
        m_pWorld->Render(commandBuffer, context);

    // Transition the Swapchain image to Present layout to present!
    {
        nes::ImageBarrierDesc imageBarrier = nes::ImageBarrierDesc()
            .SetImage(context.GetSwapchainImage())
            .SetLayout(nes::EImageLayout::Undefined, nes::EImageLayout::Present);
        //.SetAccess(nes::EAccessBits::None, nes::EAccessBits::None);
    
        nes::BarrierGroupDesc barrierGroup = nes::BarrierGroupDesc()
            .SetImageBarriers(imageBarrier);
        
        commandBuffer.SetBarriers(barrierGroup);
    }
}

void PBRExampleApp::Internal_AppShutdown()
{
    if (m_pWorld)
    {
        m_pWorld->Destroy();
        m_pWorld = nullptr;
    }
}

std::unique_ptr<nes::Application> nes::CreateApplication(ApplicationDesc& outAppDesc, WindowDesc& outWindowDesc, RendererDesc& outRendererDesc)
{
    outAppDesc.SetApplicationName("PBRExampleApp")
        .SetIsHeadless(false);
       
    outWindowDesc.SetResolution(1920, 1080)
        .SetLabel("PBR Example")
        .SetWindowMode(EWindowMode::Windowed)
        .EnableResize(true)
        .EnableVsync(false);

    outRendererDesc.EnableValidationLayer()
        .RequireQueueType(EQueueType::Graphics)
        .RequireQueueType(EQueueType::Transfer);
    
    return std::make_unique<PBRExampleApp>(outAppDesc);
}

NES_MAIN()