// HierarchyWindow.cpp
#include "HierarchyWindow.h"

namespace nes
{
    HierarchyWindow::HierarchyWindow()
    {
        m_desc.m_name = "Hierarchy";
    }

    void HierarchyWindow::RenderImGui()
    {
        if (ImGui::Begin(m_desc.m_name.c_str(), &m_desc.m_isOpen, m_desc.m_flags))
        {
            // [TODO]:
            ImGui::Text("This needs to show the entity hierarchy");

            ImGui::End();
        }
    }
}
