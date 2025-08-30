// EntryPoint.cpp
#include "Application.h"
#include "Nessie/Application/EntryPoint.h"

std::unique_ptr<nes::Application> nes::CreateApplication(ApplicationDesc& outAppDesc, WindowDesc& outWindowDesc, RendererDesc& outRendererDesc)
{
    outAppDesc.SetApplicationName("PhysicsTests");

    outWindowDesc.SetResolution(1600, 900)
        .SetLabel("Physics Tests")
        .SetWindowMode(EWindowMode::Windowed)
        .EnableResize(true)
        .EnableVsync(false);
    
    outRendererDesc.EnableValidationLayer();
    
    return std::make_unique<TestApplication>(outAppDesc);
}

NES_MAIN()