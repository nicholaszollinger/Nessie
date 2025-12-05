// EnvironmentSettingsComponent.cpp
#include "SkyboxComponent.h"
#include "ComponentSystems/PBRSceneRenderer.h"

namespace pbr
{
    void SkyboxComponent::Serialize(nes::YamlOutStream& out, const SkyboxComponent& component)
    {
        out.Write("Skybox", component.m_skyboxAssetID);
        out.Write("Priority", component.m_priority);
    }

    void SkyboxComponent::Deserialize(const nes::YamlNode& in, SkyboxComponent& component)
    {
        in["Skybox"].Read(component.m_skyboxAssetID, nes::kInvalidAssetID);
        if (component.m_skyboxAssetID == nes::kInvalidAssetID)
        {
            component.m_skyboxAssetID = PBRSceneRenderer::GetDefaultSkyboxID();
        }
        in["Priority"].Read(component.m_priority, 0);
    }
}
