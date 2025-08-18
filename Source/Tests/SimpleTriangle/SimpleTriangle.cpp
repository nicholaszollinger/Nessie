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

        // Load Async Test:
        {
            auto onSingleComplete = [this](const nes::AsyncLoadResult& result)
            {
                NES_LOG("Single Load Request complete!\n\tFreeing Texture 1...");
        
                if (result.IsValid())
                    nes::AssetManager::FreeAsset(result.GetAssetID());
            };
            nes::AssetManager::LoadAsync<nes::Texture>(m_texture1, texturePath1, onSingleComplete);
        }

        // Load Sync Test (forces the texture to be loaded now, regardless of the Async Load above).
        {
            const nes::ELoadResult result = nes::AssetManager::LoadSync<nes::Texture>(m_texture1, texturePath1);
            if (result != nes::ELoadResult::Success)
            {
                NES_ERROR("Failed to load texture!");
                return false;
            }
        }

        // Load Request Test:
        {
            auto onAssetLoaded = []([[maybe_unused]] const nes::AsyncLoadResult& result)
            {
                NES_LOG("Load Progress: {0:2f}", result.GetRequestProgress());
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
            request.SetOnAssetLoadedCallback(onAssetLoaded);
            
            request.AppendLoad<nes::Texture>(m_texture1, texturePath1);
            
            std::string texturePath2 = NES_CONTENT_DIR;
            texturePath2 += "miramar_dn.png";
            request.AppendLoad<nes::Texture>(m_texture2, texturePath2);
            
            // Submit the request:
            nes::AssetManager::SubmitLoadRequest(std::move(request));
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