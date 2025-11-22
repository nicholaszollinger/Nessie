// MeshComponent.cpp
#include "MeshComponent.h"

#include "ComponentSystems/PBRSceneRenderer.h"

namespace pbr
{
    void MeshComponent::Serialize(nes::YamlOutStream& out, const MeshComponent& component)
    {
        out.Write("Mesh", component.m_sourceMeshID);
        
        out.BeginSequence("Materials", true);
        for (auto& materialID : component.m_materials)
        {
            // Memory-Only Materials (default materials loaded with the Mesh Asset) will not have their ID saved.
            // The invalid index is used to signify a mesh source material.
            if (nes::AssetManager::IsMemoryAsset(materialID))
            {
                out.Write( nes::kInvalidAssetID);
            }
            else
            {
                out.Write(materialID); 
            }    
        }
        out.EndSequence();
    }

    void MeshComponent::Deserialize(const nes::YamlNode& in, MeshComponent& component)
    {
        in["Mesh"].Read(component.m_sourceMeshID, nes::kInvalidAssetID);
        if (component.m_sourceMeshID == nes::kInvalidAssetID)
        {
            component.m_sourceMeshID = PBRSceneRenderer::GetDefaultMeshID(EDefaultMeshType::Cube);
        }

        auto materials = in["Materials"];
        component.m_materials.clear();
        for (auto node : materials)
        {
            component.m_materials.emplace_back();
            node.Read<nes::AssetID>(component.m_materials.back(), nes::kInvalidAssetID);
        }
    }
}
