// EditorWindowManager.cpp
#include "EditorWindowManager.h"
#include "yaml-cpp/yaml.h"
#include <filesystem>
#include <fstream>
#include "imgui_internal.h"

namespace nes
{
    bool EditorWindowManager::Init()
    {
        // Load the Editor Config:
        std::filesystem::path path = NES_CONFIG_DIR;
        path /= "EditorConfig.yaml";
        YAML::Node file = YAML::LoadFile(path.string());
        if (!file)
        {
            NES_ERROR("Failed to load Editor config file!");
            return false;
        }

        auto editor = file["Editor"];
        auto windows = editor["Windows"];

        // Load open windows:
        std::string windowName = {};
        for (auto windowNode : windows)
        {
            windowName = windowNode["Name"].as<std::string>();
            auto pWindow = GetWindow<EditorWindow>(windowName);
            if (pWindow)
            {
                pWindow->SetOpen();
            }
        }

        // Load Editor Layouts:
        m_defaultLayout = editor["DefaultLayout"].as<std::string>();
        
        auto layouts = editor["Layouts"];
        for (auto layoutNode : layouts)
        {
            EditorWindowLayout layout;
            layout.m_name = layoutNode["Name"].as<std::string>();

            auto dockSplitsNode = layoutNode["DockSplits"];
            for (auto dockSplitNode : dockSplitsNode)
            {
                LayoutDockSplit dockSplit;
                dockSplit.m_direction = static_cast<ImGuiDir>(dockSplitNode["SplitDir"].as<int>(-1));
                dockSplit.m_ratio = dockSplitNode["SplitRatio"].as<float>(0.25f);
                layout.m_splits.emplace_back(dockSplit);
            }

            for (auto windowNode : layoutNode["Windows"])
            {
                LayoutDockWindow window;
                window.m_windowName = windowNode["Name"].as<std::string>();
                window.m_splitIndex = windowNode["DockIndex"].as<int>(-1); // -1 means the main dock space.
                layout.m_windows.emplace_back(std::move(window));
            }

            m_layouts.emplace(layout.m_name, layout);
        }

        return true;
    }

    void EditorWindowManager::Shutdown()
    {
        std::filesystem::path path = NES_CONFIG_DIR;
        path /= "EditorConfig.yaml";

        std::ofstream stream(path.string());
        if (!stream.is_open())
        {
            // [TODO]: Create the file.
            return;
        }

        YAML::Emitter emitter(stream);

        emitter << YAML::BeginMap << YAML::Key << "Editor" << YAML::BeginMap;
        
        // Default Layout:
        emitter << YAML::Key << "DefaultLayout" << YAML::Value << YAML::DoubleQuoted << m_defaultLayout;

        // Save the Windows:
        {
            emitter << YAML::Key << "Windows" << YAML::Value << YAML::BeginSeq;
            for (auto& window : m_windows)
            {
                if (!window->IsOpen())
                    continue;
            
                emitter << YAML::BeginMap;
                emitter << YAML::Key << "Name" << YAML::Value << YAML::DoubleQuoted << window->GetName();
                emitter << YAML::EndMap;
            }
            emitter << YAML::EndSeq;
        }

        // Save the Layouts:
        {
            emitter << YAML::Key << "Layouts" << YAML::Value << YAML::BeginSeq;
            for (const auto& [name, layout] : m_layouts)
            {
                emitter << YAML::BeginMap;
                emitter << YAML::Key << "Name" << YAML::Value << YAML::DoubleQuoted << name;
            
                // DockSplits
                emitter << YAML::Key << "DockSplits" << YAML::Value << YAML::BeginSeq;
                for (const auto& dockSplit : layout.m_splits)
                {
                    emitter << YAML::BeginMap;
                    emitter << YAML::Key << "SplitNode" << YAML::Value << dockSplit.m_splitNode;
                    emitter << YAML::Key << "SplitDir" << YAML::Value << static_cast<int>(dockSplit.m_direction);
                    emitter << YAML::Key << "SplitRatio" << YAML::Value << dockSplit.m_ratio;
                    emitter << YAML::EndMap;
                }
                emitter << YAML::EndSeq;

                // Windows
                emitter << YAML::Key << "Windows" << YAML::Value << YAML::BeginSeq;
                for (const auto& dockWindow : layout.m_windows)
                {
                    emitter << YAML::BeginMap;
                    emitter << YAML::Key << "Name" << YAML::Value << YAML::DoubleQuoted << dockWindow.m_windowName;
                    emitter << YAML::Key << "DockIndex" << YAML::Value << dockWindow.m_splitIndex;
                    emitter << YAML::EndMap;
                }
                emitter << YAML::EndSeq;
                emitter << YAML::EndMap; // End this layout.
            }

            // End the layouts array
            emitter << YAML::EndSeq;
        }
        
        emitter << YAML::EndMap; // End the Editor Map
        emitter << YAML::EndMap; // End the Root Map
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
