// EditorWindowManager.cpp
#include "EditorWindowManager.h"
#include "yaml-cpp/yaml.h"
#include <filesystem>
#include <fstream>
#include "imgui_internal.h"
#include "SelectionManager.h"
#include "Nessie/FileIO/YAML/YamlSerializer.h"

namespace nes
{
    bool EditorWindowManager::Init()
    {
        // Load the Editor Config:
        std::filesystem::path path = NES_CONFIG_DIR;
        path /= "EditorConfig.yaml";

        YamlInStream reader(path);
        if (!reader.IsOpen())
        {
            NES_ERROR("Failed to load Editor config file!");
            return false;
        }

        YamlNode root = reader.GetRoot();
        YamlNode editor = root["Editor"];
        YamlNode windows = editor["Windows"];
        
        // Load open windows:
        std::string windowName = {};
        for (auto windowNode : windows)
        {
            // [TODO]: This should create an instance, rather than getting one.
            windowNode["Name"].Read(windowName);
            auto pWindow = GetWindow<EditorWindow>(windowName);
            if (pWindow)
            {
                pWindow->Deserialize(windowNode);
                pWindow->SetOpen();
            }
        }
        
        // Load Editor Layouts:
        editor["DefaultLayout"].Read(m_defaultLayout);

        YamlNode layouts = editor["Layouts"];
        for (auto layoutNode : layouts)
        {
            EditorWindowLayout layout;
            layoutNode["Name"].Read(layout.m_name);
            
            auto dockSplitsNode = layoutNode["DockSplits"];
            for (auto dockSplitNode : dockSplitsNode)
            {
                LayoutDockSplit dockSplit;
                dockSplitNode["SplitDir"].Read(dockSplit.m_direction, ImGuiDir_None);
                dockSplitNode["SplitRatio"].Read(dockSplit.m_ratio, 0.25f);
                layout.m_splits.emplace_back(dockSplit);
            }

            for (auto windowNode : layoutNode["Windows"])
            {
                LayoutDockWindow window;
                windowNode["Name"].Read(window.m_windowName);
                windowNode["DockIndex"].Read(window.m_splitIndex, -1);
                layout.m_windows.emplace_back(std::move(window));
            }

            m_layouts.emplace(layout.m_name, layout);
        }

        return true;
    }

    void EditorWindowManager::Shutdown()
    {
        // Remove all Selections.
        editor::SelectionManager::DeselectAll();
        
        std::filesystem::path path = NES_CONFIG_DIR;
        path /= "EditorConfig.yaml";

        std::ofstream stream(path.string());
        if (!stream.is_open())
        {
            // [TODO]: Create the file if not present.
            return;
        }

        YamlOutStream writer(path, stream);
        writer.BeginMap("Editor");

        // Default Layout
        writer.Write("DefaultLayout", m_defaultLayout);

        // Save the open windows:
        writer.BeginSequence("Windows");
        for (auto& window : m_windows)
        {
            writer.BeginMap();
            window->Serialize(writer);
            writer.EndMap();
        }
        writer.EndSequence();

        // Save the Layouts:
        writer.BeginSequence("Layouts");
        for (const auto& [name, layout] : m_layouts)
        {
            writer.BeginMap();
            writer.Write("Name", name);

            // DockSplits:
            writer.BeginSequence("DockSplits");
            for (const auto& dockSplit : layout.m_splits)
            {
                writer.BeginMap();
                writer.Write("SplitNode", dockSplit.m_splitNode);
                writer.Write("SplitDir", dockSplit.m_direction);
                writer.Write("SplitRatio", dockSplit.m_ratio);
                writer.EndMap();
            }
            writer.EndSequence();

            // Windows:
            writer.BeginSequence("Windows");
            for (const auto& dockWindow : layout.m_windows)
            {
                writer.BeginMap();
                writer.Write("Name", dockWindow.m_windowName);
                writer.Write("DockIndex", dockWindow.m_splitIndex);
                writer.EndMap();
            }
            writer.EndSequence();

            writer.EndMap(); // Ends the layout
        }
        writer.EndSequence(); // Ends the array of layouts
        
        writer.EndMap(); // End the Editor Map.
        
        // Clear the windows instances.
        m_windows.clear();
    }

    void EditorWindowManager::SetupMainWindowAndDockSpace()
    {
        // Set up the main viewport window
        ImGuiViewport* imGuiViewport = ImGui::GetMainViewport();
        ImGui::SetNextWindowPos(imGuiViewport->WorkPos);
        ImGui::SetNextWindowSize(imGuiViewport->WorkSize);
        ImGui::SetNextWindowViewport(imGuiViewport->ID);

        // Window flags for the main container
        ImGuiWindowFlags mainWindowFlags = ImGuiWindowFlags_MenuBar | ImGuiWindowFlags_NoDocking;
        mainWindowFlags |= ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoCollapse;
        mainWindowFlags |= ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoMove;
        mainWindowFlags |= ImGuiWindowFlags_NoBringToFrontOnFocus | ImGuiWindowFlags_NoNavFocus;

        // Make the window background transparent
        ImGui::PushStyleVar(ImGuiStyleVar_WindowRounding, 0.0f);
        ImGui::PushStyleVar(ImGuiStyleVar_WindowBorderSize, 0.0f);
        ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(0.0f, 0.0f));

        ImGui::Begin("DockSpaceWindow", nullptr, mainWindowFlags);
        ImGui::PopStyleVar(3);

        // Create the dock space
        m_dockSpaceID = ImGui::GetID("MainDockSpace");
        
        // Check to see if a layout configuration has been set yet.
        static bool layoutInitialized = false;
        if (!layoutInitialized)
        {
            layoutInitialized = true;
            // Check if the dock space has already been configured (from .ini file)
            ImGuiDockNode* node = ImGui::DockBuilderGetNode(m_dockSpaceID);
            if (node == nullptr || !node->IsSplitNode())
            {
                // No saved layout exists, set the default layout:
                ApplyWindowLayout(m_defaultLayout);
            }
        }

        // Use the Dock space.
        ImGui::DockSpace(m_dockSpaceID, ImVec2(0.0f, 0.0f), ImGuiDockNodeFlags_PassthruCentralNode);
    }

    void EditorWindowManager::ApplyWindowLayout(const std::string& layoutName)
    {
        NES_ASSERT(m_layouts.contains(layoutName));

        // Close all windows:
        for (auto& pWindow : m_windows)
        {
            pWindow->SetOpen(false);
        }
        
        ImGuiViewport* imGuiViewport = ImGui::GetMainViewport();
        
        // Clear any existing layout
        ImGui::DockBuilderRemoveNode(m_dockSpaceID);
        ImGui::DockBuilderAddNode(m_dockSpaceID, ImGuiDockNodeFlags_DockSpace);
        ImGui::DockBuilderSetNodeSize(m_dockSpaceID, imGuiViewport->WorkSize);

        auto& layout = m_layouts.at(layoutName);

        std::vector<ImGuiID> splitIDs;
        splitIDs.reserve(layout.m_splits.size());

        // Split the dock space into sections
        ImGuiID dockMainID = m_dockSpaceID;
        for (auto& dockSplit : layout.m_splits)
        {
            ImGuiID splitID = ImGui::DockBuilderSplitNode(dockMainID, dockSplit.m_direction, dockSplit.m_ratio, nullptr, &dockMainID);
            splitIDs.emplace_back(splitID);
        }

        // Dock Windows to specific locations:
        for (auto& dockWindow : layout.m_windows)
        {
            ImGuiID nodeID;
            if (dockWindow.m_splitIndex < 0)
                nodeID = dockMainID;
            else
            {
                NES_ASSERT(dockWindow.m_splitIndex < static_cast<int>(splitIDs.size()));
                nodeID = splitIDs[dockWindow.m_splitIndex];
            }
            ImGui::DockBuilderDockWindow(dockWindow.m_windowName.c_str(), nodeID);

            // Open the window:
            auto pWindow = GetWindow<EditorWindow>(dockWindow.m_windowName);
            if (pWindow != nullptr)
            {
                pWindow->SetOpen(true);
            }
        }

        // Finish.
        ImGui::DockBuilderFinish(m_dockSpaceID);
    }

    void EditorWindowManager::OpenWindow(const std::string& name) const
    {
        auto pWindow = GetWindow<EditorWindow>(name);
        
        if (pWindow != nullptr)
        {
            // Focus the window if already open.
            if (pWindow->IsOpen())
            {
                ImGui::SetWindowFocus(pWindow->GetName().c_str());
            }

            // Open the window.
            else
            {
                pWindow->SetOpen();
            }
        }
    }
    
    void EditorWindowManager::RenderWindowMenu() const
    {
        if (ImGui::BeginMenu("Window"))
        {
            for (auto& pWindow : m_windows)
            {
                auto& name = pWindow->GetName();
                if (ImGui::MenuItem(name.c_str()))
                {
                    OpenWindow(name);
                }
            }

            ImGui::EndMenu();
        }
    }

    void EditorWindowManager::RenderWindows() const
    {
        for (auto& pWindow : m_windows)
        {
            if (pWindow->IsOpen())
            {
                pWindow->RenderImGui();
            }
        }
    }
}
