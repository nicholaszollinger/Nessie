// DayNightSimComponentInspector.cpp
#include "DayNightSimComponentInspector.h"

#include "Nessie/Editor/PropertyTable.h"

namespace pbr
{
    void DayNightSimComponentInspector::DrawImpl(DayNightSimComponent* pTarget, const nes::InspectorContext&)
    {
        nes::editor::PropertyColor("Day Color", pTarget->m_dayColor);
        nes::editor::PropertyColor("Night Color", pTarget->m_nightColor);
        nes::editor::Property("Day Duration", pTarget->m_dayDuration, 1.f, 1.f, FLT_MAX, "%.0f sec");
        nes::editor::Property("Sun Max Lux", pTarget->m_sunMaxLux, 1.f, 1.f, FLT_MAX, "%.2f lux");
    }
}
