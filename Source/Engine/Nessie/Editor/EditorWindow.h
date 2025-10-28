// EditorWindow.h
#pragma once
#include "EditorCore.h"

namespace nes
{
    struct EditorWindowDesc
    {
        std::string         m_name = {};
        ImGuiWindowFlags    m_flags = ImGuiWindowFlags_None;
        bool                m_isOpen = false;
    };

    //----------------------------------------------------------------------------------------------------
    /// @brief : Base class for all windows rendered in the Editor. Must be registered to the Editor Window Manager
    /// before being able to be used.
    //----------------------------------------------------------------------------------------------------
    class EditorWindow
    {
    public:
        EditorWindow() = default;
        EditorWindow(const EditorWindow&) = delete;
        EditorWindow& operator=(const EditorWindow&) = delete;
        EditorWindow(EditorWindow&&) noexcept = default;
        EditorWindow& operator=(EditorWindow&&) noexcept = default;
        virtual ~EditorWindow() = default;

        // Type Info Interface.
        virtual const char*     GetTypename() const = 0;
        virtual TypeID          GetTypeID() const = 0;

        //----------------------------------------------------------------------------------------------------
        /// @brief : Render the window and its contents.
        //----------------------------------------------------------------------------------------------------
        virtual void            RenderImGui() = 0;

        //----------------------------------------------------------------------------------------------------
        /// @brief : Set whether the window should be opened or closed.
        //----------------------------------------------------------------------------------------------------
        void                    SetOpen(const bool open = true)     { m_desc.m_isOpen = open; }

        //----------------------------------------------------------------------------------------------------
        /// @brief : Get properties of the Editor Window.
        //----------------------------------------------------------------------------------------------------
        const EditorWindowDesc& GetDesc() const                     { return m_desc; }

        //----------------------------------------------------------------------------------------------------
        /// @brief : Get the name of the window. This name is registered with the Editor Window Manager.
        //----------------------------------------------------------------------------------------------------
        const std::string&      GetName() const                     { return m_desc.m_name; }

        //----------------------------------------------------------------------------------------------------
        /// @brief : Check to see if this window is open.
        //----------------------------------------------------------------------------------------------------
        bool                    IsOpen() const                      { return m_desc.m_isOpen; }
    
    protected:
        EditorWindowDesc        m_desc{};
    };
}
