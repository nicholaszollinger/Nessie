// PBRRenderSettingsComponent.h
#pragma once
#include "Nessie/Asset/AssetBase.h"
#include "Nessie/World/Component.h"

namespace pbr
{
    struct SkyboxComponent
    {
        nes::AssetID        m_skyboxAssetID = nes::kInvalidAssetID;
        int                 m_priority = 0;
        
        static void         Serialize(nes::YamlOutStream& out, const SkyboxComponent& component);
        static void         Deserialize(const nes::YamlNode& in, SkyboxComponent& component);
    };
}