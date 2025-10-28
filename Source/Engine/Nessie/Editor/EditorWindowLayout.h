// EditorLayout.h
#pragma once
#include "EditorCore.h"

namespace nes
{
    struct LayoutDockSplit
    {
        int         m_splitNode = -1;               // Which node is being split. -1 = the main dock space.
        ImGuiDir    m_direction = ImGuiDir_None;    // Which direction this node will be split from.
        float       m_ratio = 0.2f;                 // Split ratio [0, 1]
    };

    struct LayoutDockWindow
    {
        std::string m_windowName{};
        int         m_splitIndex;
    };
    
    struct EditorWindowLayout
    {
        std::string m_name{};                       // Name of the Layout.
        std::vector<LayoutDockSplit> m_splits{};    // How the main dock space is split.
        std::vector<LayoutDockWindow> m_windows{};  // Maps a Window to a split in the dock space.
    };
}