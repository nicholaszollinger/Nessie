// LightComponentInspectors.cpp
#include "LightComponentInspectors.h"
#include "Nessie/Editor/PropertyTable.h"

namespace pbr
{
    void PointLightComponentInspector::DrawImpl(PointLightComponent* pTarget, const nes::InspectorContext&)
    {
        nes::editor::PropertyColor("Color", pTarget->m_color, false);
        nes::editor::Property("Intensity", pTarget->m_intensity, 1.f, 001.f, FLT_MAX, "%.2f lm", "The amount of energy emitted by a light, in lumens.");
        nes::editor::Property("Radius", pTarget->m_radius, 0.1f, 0.f, 0.f, "%.3f", "Radius of the light's effect.");
    }

    void DirectionalLightComponentInspector::DrawImpl(DirectionalLightComponent* pTarget, const nes::InspectorContext&)
    {
        nes::editor::PropertyColor("Color", pTarget->m_color, false);
        nes::editor::Property("Direction", pTarget->m_direction);
        nes::editor::Property("Intensity", pTarget->m_intensity, 1.f, 001.f, FLT_MAX, "%.2f lux", "The amount of energy emitted by the light, in lux (lumens/m^2).");        
    }
}