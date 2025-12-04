// SkyboxComponentInspector.cpp
#include "SkyboxComponentInspector.h"
#include "Nessie/Editor/PropertyTable.h"
#include "Nessie/Graphics/Texture.h"

namespace pbr
{
    void SkyboxComponentInspector::DrawImpl(TargetType* pTarget, const nes::InspectorContext&)
    {
        nes::editor::PropertyAssetID<nes::TextureCube>("Skybox", pTarget->m_skyboxAssetID);
        nes::editor::Property("Priority", pTarget->m_priority, "Skyboxes with higher priority values will be used over others. There can only be a single Skybox active at one time.");
    }
}
