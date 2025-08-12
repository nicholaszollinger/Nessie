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
        // Load a texture synchronously.
        std::string path = NES_CONTENT_DIR;
        path += "miramar_bk.png";
        
        const nes::ELoadResult result = nes::AssetManager::LoadAsync<nes::Texture>(m_id, path);
        if (result != nes::ELoadResult::Success)
        {
            NES_ERROR("Failed to load texture!");
            return false;
        }

        return true;
    }

    virtual void Internal_AppRunFrame([[maybe_unused]] const float timeStep) override
    {
        if (nes::AssetManager::GetAsset<nes::Texture>(m_id) != nullptr)
        {
            NES_LOG("Texture asset loaded!");
            
            // Free the asset.
            nes::AssetManager::FreeAsset(m_id);
        }
    }

private:
    nes::AssetID m_id = nes::kInvalidAssetID;
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