// MeshComponent.cpp
#include "MeshComponent.h"

#include "ComponentSystems/PBRSceneRenderer.h"

namespace pbr
{
    void MeshComponent::Serialize(YAML::Emitter&, const MeshComponent&)
    {
        // [TODO]: 
    }

    void MeshComponent::Deserialize(const YAML::Node& in, MeshComponent& component)
    {
        component.m_meshID = in["Mesh"].as<uint64>(nes::kInvalidAssetID.GetValue());
        if (component.m_meshID == nes::kInvalidAssetID)
        {
            component.m_meshID = PBRSceneRenderer::GetDefaultMeshID(EDefaultMeshType::Cube);
        }
        
        component.m_materialID = in["Material"].as<uint64>(nes::kInvalidAssetID.GetValue());
    }
}
