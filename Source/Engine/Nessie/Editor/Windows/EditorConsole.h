// ConsoleWindow.h
#pragma once
#include "Nessie/Editor/EditorWindow.h"

namespace nes
{
    class EditorConsole final : public EditorWindow
    {
        NES_DEFINE_TYPE_INFO(EditorConsole)
        
    public:
        EditorConsole();
        virtual ~EditorConsole() override { Clear(); }
        
        virtual void    RenderImGui() override;
        void            PostToConsole(const LogMemoryBuffer& message);
        void            Clear();

    private:
        LogTargetPtr    m_pConsoleTarget = nullptr;
        ImGuiTextBuffer m_buffer;
        ImGuiTextFilter m_filter;
        ImVector<int>   m_lineOffsets; // Index to lines offset. We maintain this with AddLog() calls.
        bool            m_autoScrollEnabled = true;
    };
}