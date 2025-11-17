// CameraComponentInspector.h
#pragma once
#include "Nessie/World/Components/CameraComponent.h"
#include "Nessie/Editor/EditorInspector.h"

namespace nes
{
    class CameraComponentInspector final : public EditorInspector<CameraComponent>
    {
        virtual void DrawImpl(CameraComponent* pTarget, const InspectorContext& context) override;
    };
}