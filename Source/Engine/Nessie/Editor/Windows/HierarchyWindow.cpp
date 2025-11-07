// HierarchyWindow.cpp
#include "HierarchyWindow.h"

#include "imgui_internal.h"
#include "Nessie/Editor/SelectionManager.h"
#include "Nessie/Graphics/ImGui/ImGuiUtils.h"

namespace nes
{
    HierarchyWindow::HierarchyWindow()
    {
        m_desc.m_name = "Hierarchy";
    }

    void HierarchyWindow::RenderImGui()
    {
        if (ImGui::Begin(m_desc.m_name.c_str(), &m_desc.m_isOpen, m_desc.m_flags))
        {
            // Search Bar:
            ImGui::SetNextItemWidth(-FLT_MIN);
            ImGui::SetNextItemShortcut(ImGuiMod_Ctrl | ImGuiKey_F, ImGuiInputFlags_Tooltip);
            ImGui::PushItemFlag(ImGuiItemFlags_NoNavDefaultFocus, true);
            static constexpr ImGuiInputTextFlags kInputFlags = ImGuiInputTextFlags_EscapeClearsAll | ImGuiInputTextFlags_ElideLeft;
            if (ImGui::InputTextWithHint("##Filter", "incl,-excl", m_filter.InputBuf, IM_ARRAYSIZE(m_filter.InputBuf), kInputFlags))
            {
                m_filter.Build();
            }
            ImGui::PopItemFlag();
            
            static constexpr ImGuiTableFlags kTableFlags = ImGuiTableFlags_NoPadInnerX
                 | ImGuiTableFlags_Resizable
                 | ImGuiTableFlags_Reorderable
                 | ImGuiTableFlags_ScrollY;
            
            EntityRegistry& registry = m_pWorld->GetRegistry();
            
            if (ImGui::BeginTable("##HierarchyTree", 1, kTableFlags))
            {
                //ImGui::TableSetupColumn("Label");
                //ImGui::TableSetupColumn("Type");
                //ImGui::TableSetupColumn("Visibility");

                auto view = registry.GetAllEntitiesWith<IDComponent, NodeComponent>();

                for (auto entity : view)
                {
                    auto& nodeComp = view.get<NodeComponent>(entity);
                    Entity entityWrapper(registry, entity);
                    if (nodeComp.m_parentID == kInvalidEntityID)
                    {
                        DrawEntityNode(registry, entityWrapper);
                    }
                }

                ImGui::EndTable();
            }
        }
        
        ImGui::End();
    }

    void HierarchyWindow::DrawEntityNode(EntityRegistry& registry, Entity& entity)
    {
        auto& idComp = entity.GetComponent<IDComponent>();
        auto& nodeComp = entity.GetComponent<NodeComponent>();
        const auto& name = idComp.GetName();
        const std::string strID = name;

        // Check if this entity matches the search
        const bool matchesSearch = m_filter.PassFilter(idComp.GetName().c_str());

        // [TODO]: Needs to be recursive.
        // Check to see if this entity has an immediate child matching the current filter.
        static constexpr uint32 kMaxNameDepth = 10;
        const bool hasChildMatchingSearch = NameSearchRecursive(registry, entity, kMaxNameDepth);
        
        // If this node or any children of this node don't match the current search filter, skip.
        if (!hasChildMatchingSearch && !matchesSearch)
            return;
        
        // [TODO]: Could be settings.
        static constexpr float kEdgeOffset = 4.f;
        static constexpr float kRowHeight = 21.f;
        
        auto* pImGuiWindow = ImGui::GetCurrentWindow();
        pImGuiWindow->DC.CurrLineSize.y = kRowHeight;
        
        ImGui::TableNextRow();
        ImGui::TableNextColumn();
        
        ui::ScopedID scopedID(idComp.GetID());
        pImGuiWindow->DC.CurrLineTextBaseOffset = 3.f;

        // const ImVec2 rowAreaMin = ImGui::TableGetCellBgRect(ImGui::GetCurrentTable(), 0).Min;
        //const ImVec2 rowAreaMax = { ImGui::TableGetCellBgRect(ImGui::GetCurrentTable(), ImGui::TableGetColumnCount() - 1).Max.x - 20,
        //                          rowAreaMin.y + kRowHeight };

        // Is the Entity Selected:
        const bool isSelected = editor::SelectionManager::IsSelectedGlobal(idComp.GetID());
        
        ImGuiTreeNodeFlags nodeFlags = ImGuiTreeNodeFlags_SpanAvailWidth
            | ImGuiTreeNodeFlags_OpenOnDoubleClick | ImGuiTreeNodeFlags_OpenOnArrow;
            //| ImGuiTreeNodeFlags_DrawLinesToNodes;

        if (isSelected)
            nodeFlags |= ImGuiTreeNodeFlags_Selected;

        if (hasChildMatchingSearch)
            nodeFlags |= ImGuiTreeNodeFlags_DefaultOpen;

        if (nodeComp.m_childrenIDs.empty())
            nodeFlags |= ImGuiTreeNodeFlags_Leaf;

        // Get the Row-Clicked/Held/Hovered state. 
        // bool isRowHovered, isRowHeld;
        // bool isRowClicked = ImGui::ButtonBehavior(ImRect(rowAreaMin, rowAreaMax), ImGui::GetID(strID.c_str()),
        //                                            &isRowHovered, &isRowHeld, ImGuiButtonFlags_AllowOverlap | ImGuiButtonFlags_PressedOnClickRelease | ImGuiButtonFlags_MouseButtonLeft | ImGuiButtonFlags_MouseButtonRight);

        // Tree Node:
        const bool nodeOpen = ImGui::TreeNodeEx("", nodeFlags, "%s", name.c_str());
        if (ImGui::IsItemFocused())
        {
            editor::SelectionManager::SelectGlobalUnique(idComp.GetID());
        }
        
        if (nodeOpen)
        {
            for (auto& childID : nodeComp.m_childrenIDs)
            {
                auto childHandle = registry.GetEntity(childID);
                if (childHandle != kInvalidEntityHandle)
                {
                    Entity childEntity(registry, childHandle);
                    DrawEntityNode(registry, childEntity);
                }
            }
            
            ImGui::TreePop();
        }
        
        // [TODO]: This rowClicked flags was never set true.
        // Update the Selection:
        // if (isRowClicked)
        // {
        //     // [TODO]: Handle Shift to select many, or ctrl to deselect on or more.
        //     
        //     
        //     
        //     ImGui::FocusWindow(ImGui::GetCurrentWindow());
        // }

        // // Drag and Drop
        // if (ImGui::BeginDragDropSource(ImGuiDragDropFlags_SourceAllowNullID))
        // {
        //     const auto& selectedEntities = editor::SelectionManager::GetSelections(editor::SelectionManager::kGlobalContext);
        //     auto entityID = idComp.GetID();
        //     
        //     if (editor::SelectionManager::IsSelectedGlobal(entityID))
        //     {
        //         ImGui::TextUnformatted(name.c_str());
        //         ImGui::SetDragDropPayload("entity_hierarchy", &entityID, 1 * sizeof(EntityID));
        //     }
        //     else
        //     {
        //         for (const auto& selectedID : selectedEntities)
        //         {
        //             const auto handle = registry.GetEntity(selectedID);
        //             const auto& selectedIDComp = registry.GetComponent<IDComponent>(handle);
        //             ImGui::TextUnformatted(selectedIDComp.GetName().c_str());
        //         }
        //
        //         ImGui::SetDragDropPayload("entity_hierarchy", selectedEntities.data(), selectedEntities.size() * sizeof(UUID));
        //     }
        //     
        //     ImGui::EndDragDropSource();
        // }
        //
        // if (ImGui::BeginDragDropTarget())
        // {
        //     const ImGuiPayload* payload = ImGui::AcceptDragDropPayload("entity_hierarchy", ImGuiDragDropFlags_AcceptNoDrawDefaultRect);
        //
        //     if (payload)
        //     {
        //         const size_t count = payload->DataSize / sizeof(uint64);
        //
        //         for (size_t i = 0; i < count; ++i)
        //         {
        //             EntityID droppedID = *(static_cast<uint64*>(payload->Data) + i);
        //             [[maybe_unused]] auto droppedHandle = registry.GetEntity(droppedID);
        //
        //             // [TODO]:
        //             // Parent the Entity
        //             // m_pWorld->ParentEntity(droppedEntity, entity);
        //         }
        //     }
        //     
        //     ImGui::EndDragDropTarget();
        // }

        // [TODO]: Handle Deletion:
    }

    bool HierarchyWindow::NameSearchRecursive(EntityRegistry& registry, Entity& entity, const uint32 maxSearchDepth, const uint32 currentDepth)
    {
        if (!m_filter.IsActive() || currentDepth >= maxSearchDepth)
            return false;

        auto& nodeComp = entity.GetComponent<NodeComponent>();
                    
        for (auto childID : nodeComp.m_childrenIDs)
        {
            auto childHandle = registry.GetEntity(childID);
            Entity childEntity(registry, childHandle);
            if (childHandle != kInvalidEntityHandle)
            {
                auto& idComp = registry.GetComponent<IDComponent>(childHandle);
                if (m_filter.PassFilter(idComp.GetName().c_str()))
                    return true;
            }
                                                                    
            // Return found results.
            if (NameSearchRecursive(registry, childEntity, maxSearchDepth, currentDepth + 1))
                return true;
        }
        
        return false;
    }
}
