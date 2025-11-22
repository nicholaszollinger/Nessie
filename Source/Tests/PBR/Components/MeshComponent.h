// MeshComponent.h
#pragma once
#include "Nessie/World.h"

namespace pbr
{
    // 'Static Mesh Component'
    struct MeshComponent
    {
        nes::AssetID    m_sourceMeshID = nes::kInvalidAssetID;
        std::vector<nes::AssetID> m_materials{};

        static void     Serialize(nes::YamlOutStream& out, const MeshComponent& component);
        static void     Deserialize(const nes::YamlNode& in, MeshComponent& component);
    };
}