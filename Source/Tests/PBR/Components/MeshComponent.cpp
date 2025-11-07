// MeshComponent.cpp
#include "MeshComponent.h"

#include "ComponentSystems/PBRSceneRenderer.h"

namespace pbr
{
    void MeshComponent::Serialize(nes::YamlOutStream& out, const MeshComponent& component)
    {
        out.Write("Mesh", component.m_meshID);
        out.Write("Material", component.m_materialID); 
    }

    void MeshComponent::Deserialize(const nes::YamlNode& in, MeshComponent& component)
    {
        in["Mesh"].Read(component.m_meshID, nes::kInvalidAssetID);
        if (component.m_meshID == nes::kInvalidAssetID)
        {
            component.m_meshID = PBRSceneRenderer::GetDefaultMeshID(EDefaultMeshType::Cube);
        }
        in["Material"].Read(component.m_materialID, nes::kInvalidAssetID);
    }
}
