// InspectorWindow.cpp
#include "InspectorWindow.h"

namespace nes
{
    InspectorWindow::InspectorWindow()
    {
        m_desc.m_name = "Inspector";
    }

    void InspectorWindow::RenderImGui()
    {
        if (ImGui::Begin(m_desc.m_name.c_str(), &m_desc.m_isOpen, 0))
        {
            // [TODO]:
            ImGui::Text("This needs to show the entity and its details.");
            
            ImGui::End();
        }
    }
}
