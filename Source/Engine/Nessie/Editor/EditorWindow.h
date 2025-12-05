// EditorWindow.h
#pragma once
#include "EditorCore.h"
#include "Nessie/Core/Events/Event.h"
#include "Nessie/Core/Memory/StrongPtr.h"
#include "Nessie/FileIO/YAML/YamlSerializer.h"

namespace nes
{
    class WorldBase;
    class EditorWorld;
    
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
        /// @brief : Render the window and its contents. The base function sets whether the window 
        //----------------------------------------------------------------------------------------------------
        virtual void            RenderImGui() = 0;

        //----------------------------------------------------------------------------------------------------
        /// @brief : Set the current World reference that is being observed. 
        //----------------------------------------------------------------------------------------------------
        void                    SetWorld(const StrongPtr<EditorWorld>& world);

        //----------------------------------------------------------------------------------------------------
        /// @brief : Load the window settings from YAML. The default method loads the Window's name.
        //----------------------------------------------------------------------------------------------------
        virtual void            Deserialize(const YamlNode& in);

        //----------------------------------------------------------------------------------------------------
        /// @brief : Save the window settings to YAML. The default method saves the Window's name.
        //----------------------------------------------------------------------------------------------------
        virtual void            Serialize(YamlOutStream& out) const;

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

        //----------------------------------------------------------------------------------------------------
        /// @brief : Get the current world context for the Editor.
        //----------------------------------------------------------------------------------------------------
        StrongPtr<EditorWorld>  GetWorld() { return m_pWorld; }
    
    protected:
        //----------------------------------------------------------------------------------------------------
        /// @brief : Called when the World pointer has been updated. 
        //----------------------------------------------------------------------------------------------------
        virtual void            OnWorldSet(){}
        
    protected:
        EditorWindowDesc        m_desc{};
        StrongPtr<EditorWorld>  m_pWorld = nullptr;
    };
    
}
