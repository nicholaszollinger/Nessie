// Cursor.h
#pragma once
#include "Nessie/Core/Config.h"

namespace nes
{
    //----------------------------------------------------------------------------------------------------
    /// @brief : Defines the cursor behavior with Application Window. 
    //----------------------------------------------------------------------------------------------------
    enum class ECursorMode : uint8
    {
        Visible,  // The cursor is visible on screen.
        Hidden,   // The cursor is invisible, but still moves around the screen as normal.
        Disabled, // The cursor is locked to the center of the screen, useful for things FPS cameras.
        Captured, // The cursor is locked to the bounds of the window.
    };
}