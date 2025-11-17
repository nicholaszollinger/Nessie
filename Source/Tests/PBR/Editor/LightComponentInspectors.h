// LightComponentInspectors.h
#pragma once
#include "Components/LightComponents.h"
#include "Nessie/Editor/EditorInspector.h"

namespace pbr
{
    class PointLightComponentInspector final : public nes::EditorInspector<PointLightComponent>
    {
        virtual void DrawImpl(PointLightComponent* pTarget, const nes::InspectorContext& context) override;
    };

    class DirectionalLightComponentInspector final : public nes::EditorInspector<DirectionalLightComponent>
    {
        virtual void DrawImpl(DirectionalLightComponent* pTarget, const nes::InspectorContext& context) override;
    };
}