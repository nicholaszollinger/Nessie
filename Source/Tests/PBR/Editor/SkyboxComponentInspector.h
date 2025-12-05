// SkyboxComponentInspector.h
#pragma once
#include "Nessie/Editor/EditorInspector.h"
#include "Components/SkyboxComponent.h"

namespace pbr
{
    class SkyboxComponentInspector final : public nes::EditorInspector<pbr::SkyboxComponent>
    {
        virtual void DrawImpl(TargetType* pTarget, const nes::InspectorContext& context) override;
    };
}
