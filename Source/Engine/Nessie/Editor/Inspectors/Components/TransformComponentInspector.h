// TransformComponentInspector.h
#pragma once
#include "Nessie/Editor/EditorInspector.h"
#include "Nessie/World/ComponentSystems/TransformSystem.h"

namespace nes
{
    class TransformComponentInspector final : public EditorInspector<TransformComponent>
    {
    public:
        TransformComponentInspector() : EditorInspector(EInspectorLevel::Internal) {}

    private:
        virtual void DrawImpl(TransformComponent* pTarget, const InspectorContext& context) override;
    };
}