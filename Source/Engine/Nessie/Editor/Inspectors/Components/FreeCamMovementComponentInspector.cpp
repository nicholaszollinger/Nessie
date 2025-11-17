// FreeCamMovementComponentInspector.cpp
#include "FreeCamMovementComponentInspector.h"
#include "Nessie/Editor/PropertyTable.h"

namespace nes
{
    void FreeCamMovementComponentInspector::DrawImpl(FreeCamMovementComponent* pTarget, const InspectorContext&)
    {
        editor::Property("Move Speed", pTarget->m_moveSpeed, 1.f, 1.f, FLT_MAX, "%.3f m/s");
        editor::Property("Sensitivity", pTarget->m_sensitivity, 0.1f, 0.1f, 5.f, "%.2f");
    }
}
