// ViewportWindow.cpp
#include "ViewportWindow.h"

namespace nes
{
    ViewportWindow::ViewportWindow()
    {
        m_desc.m_name = "Viewport";
        m_desc.m_flags = ImGuiWindowFlags_NoNav;
    }

    void ViewportWindow::RenderImGui()
    {
        if (ImGui::Begin("Viewport", &m_desc.m_isOpen, ImGuiWindowFlags_NoNav))
        {
            ImGui::Text("This is the Viewport");

            // [TODO]: Render the current image to the window? how does this work?
            
            ImGui::End();
        }
    }
}
