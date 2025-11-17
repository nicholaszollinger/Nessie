// DayNightSimComponentInspector.h
#pragma once
#include "ComponentSystems/DayNightSystem.h"
#include "Nessie/Editor/EditorInspector.h"

namespace pbr
{
    class DayNightSimComponentInspector final : public nes::EditorInspector<DayNightSimComponent>
    {
        virtual void DrawImpl(DayNightSimComponent* pTarget, const nes::InspectorContext& context) override;   
    };
}
