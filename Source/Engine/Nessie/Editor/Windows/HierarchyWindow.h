// HierarchyWindow.h
#pragma once
#include "Nessie/Editor/EditorWindow.h"

namespace nes
{
    class HierarchyWindow final : public EditorWindow
    {
        NES_DEFINE_TYPE_INFO(HierarchyWindow)
        
    public:
        HierarchyWindow();

        virtual void RenderImGui() override;
    };
}
