// MeshComponentInspector.h
#pragma once
#include "Nessie/Editor/EditorInspector.h"
#include "Components/MeshComponent.h"

namespace pbr
{
    class MeshComponentInspector final : public nes::EditorInspector<MeshComponent>
    {
        virtual void DrawImpl(TargetType* pTarget, const nes::InspectorContext& context) override;
    };
}
