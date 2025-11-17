// FreeCamMovementComponentInspector.h
#pragma once
#include "Nessie/World/ComponentSystems/FreeCamSystem.h"
#include "Nessie/Editor/EditorInspector.h"

namespace nes
{
    class FreeCamMovementComponentInspector final : public EditorInspector<FreeCamMovementComponent>
    {
        virtual void DrawImpl(FreeCamMovementComponent* pTarget, const InspectorContext& context) override;
    };
}