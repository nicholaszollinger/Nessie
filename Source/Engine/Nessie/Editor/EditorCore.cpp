// EditorCore.cpp
#include "EditorCore.h"

namespace nes::editor
{
    static int      s_globalContextID = 0;           // Current nesting depth of the ID scopes.
    static uint32   s_globalContextCounter = 0;   // Unique ID within the current context scope.
    static char     s_idBuffer[16 + 2 + 1] = "##";
    static char     s_labelIDBuffer[1024 + 1];
    
    const char* GenerateID()
    {
        (void)snprintf(s_idBuffer + 2, 16, "%u", s_globalContextCounter++);
        return s_idBuffer;
    }
    
    const char* GenerateLabelID(std::string_view label)
    {
        *std::format_to_n(s_labelIDBuffer, std::size(s_labelIDBuffer), "{}##{}", label, s_globalContextCounter++).out = 0;
        return s_labelIDBuffer;
    }
    
    bool IsInputEnabled()
    {
        const auto& io = ImGui::GetIO();
        return (io.ConfigFlags & ImGuiConfigFlags_NoMouse) == 0 && (io.ConfigFlags & ImGuiConfigFlags_NavNoCaptureKeyboard) == 0;
    }
    
    void SetInputEnabled(const bool enabled)
    {
        auto& io = ImGui::GetIO();
    
        if (enabled)
        {
            io.ConfigFlags &= ~ImGuiConfigFlags_NoMouse;
            io.ConfigFlags &= ~ImGuiConfigFlags_NavNoCaptureKeyboard;
        }
        else
        {
            io.ConfigFlags |= ImGuiConfigFlags_NoMouse;
            io.ConfigFlags |= ImGuiConfigFlags_NavNoCaptureKeyboard;
        }
    }
    
    void PushIDLevel()
    {
        ImGui::PushID(s_globalContextID++);
        s_globalContextCounter = 0;
    }
    
    void PopIDLevel()
    {
        ImGui::PopID();
        --s_globalContextID;
        s_globalContextCounter = 0;
    }
    
    void ShiftCursorX(const float distance)
    {
        ImGui::SetCursorPosX(ImGui::GetCursorPosX() + distance);
    }
    
    void ShiftCursorY(const float distance)
    {
        ImGui::SetCursorPosY(ImGui::GetCursorPosY() + distance);
    }
    
    void ShiftCursor(const float xDist, const float yDist)
    {
        const ImVec2 cursor = ImGui::GetCursorPos();
        ImGui::SetCursorPos(ImVec2(cursor.x + xDist, cursor.y + yDist));
    }

    void SetToolTip(const char* toolTip)
    {
        if (ImGui::IsItemHovered(ImGuiHoveredFlags_DelayNormal | ImGuiHoveredFlags_AllowWhenDisabled))
        {
            ImGui::SetTooltip("%s", toolTip);
        }

        // [NOTE]: Keeping this for later. It shows a (?) that when hovered reveals the tooltip. 
        // ImGui::TextDisabled("(?)");
        // if (ImGui::IsItemHovered(ImGuiHoveredFlags_AllowWhenDisabled))
        // {
        //     ImGui::BeginTooltip();
        //     ImGui::PushTextWrapPos(ImGui::GetFontSize() * 35.0f);
        //     ImGui::TextUnformatted(toolTip);
        //     ImGui::PopTextWrapPos();
        //     ImGui::EndTooltip();
        // }
    }
}
