// TextureAssetComponent.cpp
#include "TextureAssetComponent.h"
#include "Nessie/Graphics/Texture.h"
#include "Nessie/Editor/PropertyTable.h"

void TextureAssetComponent::Serialize(nes::YamlOutStream& out, const TextureAssetComponent& component)
{
    out.Write("TextureAsset", component.m_assetID);
    out.Write("TestEntity", component.m_testEntity);
}

void TextureAssetComponent::Deserialize(const nes::YamlNode& in, TextureAssetComponent& component)
{
    in["TextureAsset"].Read(component.m_assetID, nes::kInvalidAssetID);
    in["TestEntity"].Read(component.m_testEntity, nes::kInvalidEntityID);
}

void TextureAssetComponentInspector::DrawImpl(TargetType* pTarget, const nes::InspectorContext& context)
{
    if (auto* pRegistry = context.m_pWorld->GetEntityRegistry())
    {
        nes::editor::PropertyAssetID<nes::Texture>("Texture", pTarget->m_assetID);
        nes::editor::PropertyEntityID("EntityRef", pTarget->m_testEntity, *pRegistry);
    }
}