// ViewportWindow.h
#pragma once
#include "Nessie/Editor/EditorWindow.h"

namespace nes
{
    //----------------------------------------------------------------------------------------------------
    /// @brief : The Viewport window renders the World from the perspective of an Editor Camera or an
    /// in-world Camera when running the program.
    //----------------------------------------------------------------------------------------------------
    class ViewportWindow : public EditorWindow
    {
        NES_DEFINE_TYPE_INFO(ViewportWindow)
        
    public:
        ViewportWindow();
        
        virtual void    RenderImGui() override;
        
        // Takes in a Camera?
        //void            RenderWorld();
    };
}
