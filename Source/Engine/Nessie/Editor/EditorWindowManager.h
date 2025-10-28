﻿// EditorWindowManager.h
#pragma once
#include <unordered_map>
#include <memory>
#include "EditorWindow.h"
#include "EditorWindowLayout.h"

namespace nes
{
    //----------------------------------------------------------------------------------------------------
    //	NOTES:
    //  Right now, I am maintaining instances of windows even if they are not open. Plus, only a single instance
    //  of a window is allowed. I need to have the Window name map to WindowTypeDesc info rather than the instance itself.
    //  it could have a number of allowed instances, etc.
    //		
    /// @brief : [Under Development]
    //----------------------------------------------------------------------------------------------------
    class EditorWindowManager
    {
    public:
        EditorWindowManager() = default;
        EditorWindowManager(const EditorWindowManager&) = delete;
        EditorWindowManager(EditorWindowManager&&) noexcept = delete;
        EditorWindowManager& operator=(const EditorWindowManager&) = delete;
        EditorWindowManager& operator=(EditorWindowManager&&) noexcept = delete;
        ~EditorWindowManager() = default;

        //----------------------------------------------------------------------------------------------------
        /// @brief : Loads window states and layouts from the EditorConfig.yaml
        //----------------------------------------------------------------------------------------------------
        bool Init();

        //----------------------------------------------------------------------------------------------------
        /// @brief : Closes all windows, and saves window settings to the EditorConfig.yaml
        //----------------------------------------------------------------------------------------------------
        void Shutdown();

        //----------------------------------------------------------------------------------------------------
        /// @brief : Must be called every frame. Initializes the main window and docking area.
        //----------------------------------------------------------------------------------------------------
        void SetupMainWindowAndDockSpace();

        //----------------------------------------------------------------------------------------------------
        /// @brief : Sets a layout for the editor.
        //----------------------------------------------------------------------------------------------------
        void ApplyWindowLayout(const std::string& layoutName);

        //----------------------------------------------------------------------------------------------------
        /// @brief : Opens a window, or focuses it if the window is already open.
        //----------------------------------------------------------------------------------------------------
        void OpenWindow(const std::string& name) const;

        //----------------------------------------------------------------------------------------------------
        /// @brief : Renders the "Window" dropdown menu in the main menu bar.
        //----------------------------------------------------------------------------------------------------
        void RenderWindowMenu() const;

        //----------------------------------------------------------------------------------------------------
        /// @brief : Renders all open windows.
        //----------------------------------------------------------------------------------------------------
        void RenderWindows() const;

        //----------------------------------------------------------------------------------------------------
        /// @brief : Register an Editor Window type so that it can be opened and used.
        //----------------------------------------------------------------------------------------------------
        template <typename Type>
        void RegisterWindow()
        {
            // Naive approach for now, assuming that we aren't adding duplicate window types, and single instances.
            std::shared_ptr<Type> pWindow = std::make_shared<Type>();
            m_windows.emplace_back(pWindow);
            m_nameToIndexMap[pWindow->GetName()] = m_windows.size() - 1;
        }

        //----------------------------------------------------------------------------------------------------
        /// @brief : Get a registered window by type.
        //----------------------------------------------------------------------------------------------------
        template <typename Type>
        std::shared_ptr<Type> GetWindow(const std::string& name) const
        {
            if (auto it = m_nameToIndexMap.find(name); it != m_nameToIndexMap.end())
            {
                const size_t index = it->second;
                NES_ASSERT(index < m_windows.size());
                return m_windows[index];
            }

            return nullptr;
        }
    
    private:
        std::unordered_map<std::string, size_t>    m_nameToIndexMap{};      // Maps a window name to it's index in the windows array.
        std::vector<std::shared_ptr<EditorWindow>> m_windows{};             // Container of available windows.
        std::unordered_map<std::string, EditorWindowLayout> m_layouts{};    // Container of layouts that can be applied to the Editor.
        std::string                                 m_defaultLayout = {};   // Name of the default layout.
        ImGuiID                                     m_dockSpaceID = {};     // The id of the main window dockspace.
    };
}
