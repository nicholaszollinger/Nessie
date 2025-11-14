// InspectorWindow.h
#pragma once
#include "Nessie/Editor/EditorWindow.h"
#include "Nessie/World.h"

namespace nes
{
    //----------------------------------------------------------------------------------------------------
    /// @brief : The Inspector Window will show details about a selected entity and its components.
    //----------------------------------------------------------------------------------------------------
    class InspectorWindow final : public EditorWindow
    {
        NES_DEFINE_TYPE_INFO(InspectorWindow)
        
    public:
        InspectorWindow();
        virtual void RenderImGui() override;
    };
}
