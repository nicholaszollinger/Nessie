// MeshComponent.h
#pragma once
#include "Nessie/World.h"

namespace pbr
{
    struct MeshComponent
    {
        nes::AssetID    m_meshID = nes::kInvalidAssetID;
        nes::AssetID    m_materialID = nes::kInvalidAssetID;

        static void     Serialize(YAML::Emitter& out, const MeshComponent& component);
        static void     Deserialize(const YAML::Node& in, MeshComponent& component);
    };
}