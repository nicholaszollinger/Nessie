// SimpleTriangle.cpp
#include "Nessie/Application/EntryPoint.h"
#include "Nessie/Asset/AssetManager.h"
#include "Nessie/Graphics/Texture.h"

class SimpleTriangle final : public nes::Application
{
public:
    explicit SimpleTriangle(const nes::ApplicationDesc& appDesc)
        : nes::Application(appDesc)
    {
        //
    }

    virtual bool Internal_AppInit() override
    {
        // Add some assets:
        std::string texturePath1 = NES_CONTENT_DIR;
        texturePath1 += "miramar_bk.png";

        // Load Sync Test:
        {
            const nes::ELoadResult result = nes::AssetManager::LoadSync<nes::Texture>(m_texture1, texturePath1);
            if (result != nes::ELoadResult::Success && result != nes::ELoadResult::Pending)
            {
                NES_ERROR("Failed to load texture!");
                return false;
            }
        }

        // Load Request Test:
        {
            auto onProgress = [](const float progress)
            {
                NES_LOG("Load Progress: {0:2f}", progress);  
            };

            auto onComplete = [this](const nes::ELoadResult result)
            {
                if (result == nes::ELoadResult::Success)
                {
                    NES_LOG("Load Request complete!");
                }
            };
            
            // Begin a load request:
            nes::LoadRequest request = nes::AssetManager::BeginLoadRequest();
            request.SetOnCompleteCallback(onComplete);
            request.SetOnProgressUpdatedCallback(onProgress);
            
            request.AppendLoad<nes::Texture>(m_texture1, texturePath1);
            
            std::string texturePath2 = NES_CONTENT_DIR;
            texturePath2 += "miramar_dn.png";
            request.AppendLoad<nes::Texture>(m_texture2, texturePath2);
            
            // Submit the request:
            nes::AssetManager::SubmitLoadRequest(std::move(request));
        }
        
        // Load Async Test:
        {
            auto onSingleComplete = [this](const nes::ELoadResult result)
            {
                NES_LOG("Single Load Request complete!\n\tFreeing Texture 1...");
        
                if (result == nes::ELoadResult::Success)
                    nes::AssetManager::FreeAsset(m_texture1);
            };
            nes::AssetManager::LoadAsync<nes::Texture>(m_texture1, texturePath1, onSingleComplete);
        }

        return true;
    }

    virtual void Internal_AppRunFrame([[maybe_unused]] const float timeStep) override
    {
        if (nes::AssetManager::GetAsset<nes::Texture>(m_texture1) != nullptr)
        {
            NES_LOG("Freeing Texture 1...");
            nes::AssetManager::FreeAsset(m_texture1);
        }

        if (nes::AssetManager::GetAsset<nes::Texture>(m_texture2) != nullptr)
        {
            NES_LOG("Freeing Texture 2...");
            nes::AssetManager::FreeAsset(m_texture2);
        }
    }

private:
    nes::AssetID m_texture1 = nes::kInvalidAssetID;
    nes::AssetID m_texture2 = nes::kInvalidAssetID;
};

std::unique_ptr<nes::Application> nes::CreateApplication(ApplicationDesc& outAppDesc, WindowDesc& outWindowDesc, RendererDesc& outRendererDesc)
{
    outAppDesc.SetApplicationName("Graphics Tests")
        .SetIsHeadless(false);
       
    outWindowDesc.SetResolution(1280, 720)
        .SetLabel("Graphics Tests")
        .SetWindowMode(EWindowMode::Windowed)
        .EnableResize(true)
        .EnableVsync(false);

    outRendererDesc.EnableValidationLayer();

    return std::make_unique<SimpleTriangle>(outAppDesc);
}

NES_MAIN()