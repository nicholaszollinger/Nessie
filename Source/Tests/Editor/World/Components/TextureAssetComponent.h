// TextureAssetComponent.h
#pragma once
#include "Nessie/Asset/AssetBase.h"
#include "Nessie/World.h"
#include "Nessie/Editor/EditorInspector.h"

struct TextureAssetComponent
{
    nes::AssetID    m_assetID = nes::kInvalidAssetID;
    nes::EntityID   m_testEntity = nes::kInvalidEntityID;

    static void     Serialize(nes::YamlOutStream& out, const TextureAssetComponent& component);
    static void     Deserialize(const nes::YamlNode& in, TextureAssetComponent& component);
};

class TextureAssetComponentInspector final : public nes::EditorInspector<TextureAssetComponent>
{
    virtual void    DrawImpl(TargetType* pTarget, const nes::InspectorContext& context) override;
};