// ImGuiUtils.h
#pragma once
#include "imgui.h"
#include "Nessie/Core/Config.h"

namespace nes::ui
{
//-------------------------------------------------------------------------------------------------------------
#pragma region ScopedStyling
//-------------------------------------------------------------------------------------------------------------
    
    class ScopedStyle
    {
    public:
        template <typename Type>
        ScopedStyle(ImGuiStyleVar styleVar, const Type& value) { ImGui::PushStyleVar(styleVar, value); }
        ~ScopedStyle() { ImGui::PopStyleVar(); }
        
        ScopedStyle(const ScopedStyle&) = delete;
        ScopedStyle& operator=(const ScopedStyle&) = delete;
    };

    #define NES_UI_SCOPED_STYLE(styleVar, value) nes::ui::ScopedStyle NES_SCOPED_TAG(scopeStyle)(styleVar, value)

    class ScopedColor
    {
    public:
        template <typename Type>
        ScopedColor(const ImGuiCol colorID, const Type& color) { ImGui::PushStyleColor(colorID, ImColor(color).Value); }
        ~ScopedColor() { ImGui::PopStyleColor(); }
        
        ScopedColor(const ScopedColor&) = delete;
        ScopedColor& operator=(const ScopedColor&) = delete;
    };
    
    #define NES_UI_SCOPED_COLOR(colorID, color) nes::ui::ScopedColor NES_SCOPED_TAG(scopeColor)(colorID, color)

    class ScopedFont
    {
    public:
        ScopedFont(ImFont* pFont) { ImGui::PushFont(pFont); }
        ~ScopedFont() { ImGui::PopFont(); }
        
        ScopedFont(const ScopedFont&) = delete;
        ScopedFont& operator=(const ScopedFont&) = delete;
    };
    
    #define NES_UI_SCOPED_FONT(font) nes::ui::ScopedFont NES_SCOPED_TAG(scopeFont)(font)
    
    class ScopedID
    {
    public:
        template <typename Type>
        ScopedID(const Type id) { ImGui::PushID(id); }
        ~ScopedID() { ImGui::PopID(); }
        
        ScopedID(const ScopedID&) = delete;
        ScopedID& operator=(const ScopedID&) = delete;
    };

    template <>
    inline ScopedID::ScopedID(const uint64 value) { ImGui::PushID(reinterpret_cast<const void*>(value)); }

    #define NES_UI_SCOPED_ID(id) nes::ui::ScopedID NES_SCOPED_TAG(scopeID)(id)

#pragma endregion
//-------------------------------------------------------------------------------------------------------------
}
