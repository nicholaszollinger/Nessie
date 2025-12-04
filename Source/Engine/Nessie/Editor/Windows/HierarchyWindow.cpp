// HierarchyWindow.cpp
#include "HierarchyWindow.h"

#include "imgui_internal.h"
#include "Nessie/Editor/EditorWorld.h"
#include "Nessie/Editor/PropertyTable.h"
#include "Nessie/Editor/SelectionManager.h"
#include "Nessie/Graphics/ImGui/ImGuiUtils.h"
#include "Nessie/World/ComponentSystems/TransformSystem.h"
#include "Nessie/Core/MoveElement.h"

namespace nes
{
    static constexpr float kRowHeight = 21.f;
    static constexpr auto kDividerLineThickness = 2.f;
        
    HierarchyWindow::HierarchyWindow()
    {
        m_desc.m_name = "Hierarchy";
    }

    void HierarchyWindow::RenderImGui()
    {
        // Cache the drag payload once at the top level
        const ImGuiPayload* dragPayload = ImGui::GetDragDropPayload();
        m_isDraggingEntity = dragPayload && strcmp(dragPayload->DataType, kEntityHierarchyDropPayloadName) == 0;

        EntityRegistry* pRegistry = m_pWorld ? m_pWorld->GetEntityRegistry() : nullptr;
        
        if (ImGui::Begin(m_desc.m_name.c_str(), &m_desc.m_isOpen, m_desc.m_flags))
        {
            // Add entity button:
            ImGui::BeginDisabled(pRegistry == nullptr);
            
            // Render "Add" button that opens a dropdown of all attachable components.
            // [TODO]: Add a green '+' icon, to make it look nicer.
            if (ImGui::Button("+Add"))
            {
                ImGui::OpenPopup("##AddEntityPopup");
            }

            if (pRegistry != nullptr)
                DrawAddEntityDropdown(*pRegistry);
            
            // Search Bar:
            ImGui::SameLine();
            ImGui::SetNextItemWidth(-FLT_MIN);
            ImGui::SetNextItemShortcut(ImGuiMod_Ctrl | ImGuiKey_F, ImGuiInputFlags_Tooltip);
            ImGui::PushItemFlag(ImGuiItemFlags_NoNavDefaultFocus, true);
            static constexpr ImGuiInputTextFlags kInputFlags = ImGuiInputTextFlags_EscapeClearsAll | ImGuiInputTextFlags_ElideLeft;
            if (ImGui::InputTextWithHint("##Filter", "incl,-excl", m_filter.InputBuf, IM_ARRAYSIZE(m_filter.InputBuf), kInputFlags))
            {
                m_filter.Build();
            }
            ImGui::PopItemFlag();
            ImGui::Separator();
            ImGui::Spacing();
            
            static constexpr ImGuiTableFlags kTableFlags = ImGuiTableFlags_NoPadInnerX
                | ImGuiTableFlags_Resizable
                | ImGuiTableFlags_Reorderable
                | ImGuiTableFlags_ScrollY
                | ImGuiTableFlags_NoSavedSettings; 
            
            if (ImGui::BeginTable("##HierarchyTree", 1, kTableFlags))
            {
                // [TODO]: Different columns for name and visibility:
                //ImGui::TableSetupColumn("Label");
                //ImGui::TableSetupColumn("Visibility");
                if (pRegistry)
                {
                    const auto& rootEntities = *m_pWorld->GetRootEntities();
                    for (size_t i = 0; i < rootEntities.size(); ++i)
                    {
                        auto entity = pRegistry->GetEntity(rootEntities[i]);
                        Entity entityWrapper(*pRegistry, entity);
                        
                        if (auto* pNodeComp = pRegistry->TryGetComponent<NodeComponent>(entity))
                        {
                            // This entity exists in the world.
                            // [Consider]: If you have UI objects, I would have to check for a transform, not a Node Component.
                            if (pNodeComp->m_parentID == kInvalidEntityID)
                            {
                                DrawWorldEntityNode(*pRegistry, entityWrapper);
                            }
                        }
                        else
                        {
                            DrawGlobalEntityNode(*pRegistry, entityWrapper);
                        }
                    }
                    
                    // Draw a context menu for when the user right clicks in the open space of the hierarchy.
                    DrawGlobalContextMenu(*pRegistry);
                }
                
                ImGui::EndTable();
            }

            // Remove the selection if clicking on empty space:
            if (ImGui::IsWindowHovered(ImGuiHoveredFlags_ChildWindows) && ImGui::IsMouseClicked(ImGuiMouseButton_Left))
            {
                // Check if we clicked on empty space (no item was hovered)
                if (!ImGui::IsAnyItemHovered())
                {
                    editor::SelectionManager::DeselectAll(editor::SelectionManager::kGlobalContext);
                    m_selectionContainsGlobalEntity = false;
                }
            }
            ImGui::EndDisabled();
        }
        
        ImGui::End();
    }

    void HierarchyWindow::DrawAddEntityDropdown(EntityRegistry& registry)
    {
        ImGui::SetNextWindowSize(ImVec2(300, 400), ImGuiCond_FirstUseEver);
        
        if (ImGui::BeginPopup("##AddEntityPopup"))
        {
            if (ImGui::MenuItem("Global"))
            {
                CreateNewGlobalEntity(registry);
            }
            if (ImGui::IsItemHovered(ImGuiHoveredFlags_DelayNormal))
            {
                ImGui::SetTooltip("Creates an Entity without a Transform. This can be used for Managers, Settings, Script-only Entities, etc.");
            }

            if (ImGui::MenuItem("World"))
            {
                CreateNewWorldEntity(registry);
            }
            if (ImGui::IsItemHovered(ImGuiHoveredFlags_DelayNormal))
            {
                ImGui::SetTooltip("Creates an Entity that can be placed in the world.");
            }

            ImGui::EndPopup();
        }
    }

    void HierarchyWindow::DrawWorldEntityNode(EntityRegistry& registry, Entity& entity)
    {
        auto& idComp = entity.GetComponent<IDComponent>();
        auto& nodeComp = entity.GetComponent<NodeComponent>();
        const auto& name = idComp.GetName();
        
        // Check if this entity matches the search
        const bool matchesSearch = m_filter.PassFilter(name.c_str());
        
        // Check to see if this entity has an immediate child matching the current filter.
        static constexpr uint32 kMaxNameDepth = 10;
        const bool hasChildMatchingSearch = NameSearchRecursive(registry, entity, kMaxNameDepth);
        
        // If this node or any children of this node don't match the current search filter, skip.
        if (!hasChildMatchingSearch && !matchesSearch)
            return;
        
        auto* pImGuiWindow = ImGui::GetCurrentWindow();
        pImGuiWindow->DC.CurrLineSize.y = kRowHeight;
        
        ImGui::TableNextRow();
        ImGui::TableNextColumn();
        
        ui::ScopedID scopedID(idComp.GetID());
        pImGuiWindow->DC.CurrLineTextBaseOffset = 3.f;

        // Is the Entity Selected:
        const bool isSelected = editor::SelectionManager::IsSelectedGlobal(idComp.GetID());
        const bool isRenaming = (m_currentRenameEntity == idComp.GetID());
        
        ImGuiTreeNodeFlags nodeFlags = ImGuiTreeNodeFlags_SpanAvailWidth
            | ImGuiTreeNodeFlags_OpenOnDoubleClick | ImGuiTreeNodeFlags_OpenOnArrow;

        if (isSelected)
            nodeFlags |= ImGuiTreeNodeFlags_Selected;

        if (hasChildMatchingSearch)
            ImGui::SetNextItemOpen(true);

        if (nodeComp.m_childrenIDs.empty())
            nodeFlags |= ImGuiTreeNodeFlags_Leaf;

        // Ensure that the nodes don't auto highlight when dragging.
        if (m_isDraggingEntity)
        {
            ImGui::PushStyleColor(ImGuiCol_HeaderHovered, ImVec4(0, 0, 0, 0));
        }
        
        if (m_forceOpenEntity == idComp.GetID())
        {
            ImGui::SetNextItemOpen(true);
            m_forceOpenEntity = kInvalidEntityID;
        }
        
        bool nodeOpen;
        if (isRenaming)
        {
            DrawEntityNodeRename(idComp, nodeOpen, nodeFlags);
        }
        else
        {
            // Not renaming, normal Tree Node:
            nodeOpen = ImGui::TreeNodeEx("", nodeFlags, "%s", name.c_str());
            
            if (ImGui::IsItemClicked(ImGuiMouseButton_Left))
            {
                const bool ctrlHeld = ImGui::GetIO().KeyCtrl;

                if (ctrlHeld)
                {
                    // Ctrl+Click: Toggle this entity in selection
                    if (editor::SelectionManager::IsSelectedGlobal(idComp.GetID()))
                        editor::SelectionManager::DeselectGlobal(idComp.GetID());
                    else
                        editor::SelectionManager::Select(editor::SelectionManager::kGlobalContext, idComp.GetID());
                }
                else
                {
                    // Normal click, select only this entity.
                    editor::SelectionManager::SelectGlobalUnique(idComp.GetID());
                    m_selectionContainsGlobalEntity = false;
                }
            }

            if (isSelected && ImGui::IsWindowFocused())
            {
                // F2 to rename the current selected entity.
                if (ImGui::IsKeyPressed(ImGuiKey_F2))
                {
                    m_currentRenameEntity = idComp.GetID();
                    m_shouldFocusRename = true;
                }

                // Delete to delete the current selected entity.
                if (ImGui::IsKeyPressed(ImGuiKey_Delete))
                {
                    DeleteEntityAndChildren(registry, idComp.GetID());
                }
            }
        }
        
        // Pop the highlight disable styling.
        if (m_isDraggingEntity)
        {
            ImGui::PopStyleColor();
        }

        const ImVec2 treeNodeMin = ImGui::GetItemRectMin();
        const ImVec2 treeNodeMax = ImGui::GetItemRectMax();
        
        // Drag Source - must come immediately after the tree node.
        if (ImGui::BeginDragDropSource(ImGuiDragDropFlags_SourceAllowNullID))
        {
            const auto& selectedEntities = editor::SelectionManager::GetSelections(editor::SelectionManager::kGlobalContext);
            auto entityID = idComp.GetID();
            
            if (editor::SelectionManager::IsSelectedGlobal(entityID))
            {
                ImGui::TextUnformatted(name.c_str());
                ImGui::SetDragDropPayload(kEntityHierarchyDropPayloadName, &entityID, 1 * sizeof(EntityID));
            }
            else
            {
                for (const auto& selectedID : selectedEntities)
                {
                    const auto handle = registry.GetEntity(selectedID);
                    const auto& selectedIDComp = registry.GetComponent<IDComponent>(handle);
                    ImGui::TextUnformatted(selectedIDComp.GetName().c_str());
                }
        
                ImGui::SetDragDropPayload(kEntityHierarchyDropPayloadName, selectedEntities.data(), selectedEntities.size() * sizeof(UUID));
            }
            
            ImGui::EndDragDropSource();
        }

        if (m_isDraggingEntity)
        {
            if (!m_selectionContainsGlobalEntity)
            {
                // Get the dragged entity ID(s) to check against
                const ImGuiPayload* payload = ImGui::GetDragDropPayload();
                const size_t count = payload->DataSize / sizeof(uint64);
                const uint64* draggedIDs = static_cast<uint64*>(payload->Data);
    
                // Make sure that the dragged node is not our self or parent.
                bool isDraggingSelfOrChild = false;
                for (size_t i = 0; i < count; ++i)
                {
                    if (m_pWorld->IsDescendantOf(idComp.GetID(), draggedIDs[i]))
                    {
                        isDraggingSelfOrChild = true;
                        break;
                    }
                }

                if (!isDraggingSelfOrChild)
                {
                    const bool removeInsertAfter = nodeOpen && !nodeComp.m_childrenIDs.empty();
                    if (!nodeComp.HasParent())
                        DrawDragTargetForRootEntity(registry, entity, treeNodeMin, treeNodeMax, removeInsertAfter);
                    else
                        DrawDragTargetForWorldEntity(registry, entity, treeNodeMin, treeNodeMax, removeInsertAfter);
                }    
            }
            else if (!nodeComp.HasParent())
            {
                // The selection might have a global entity, but we are reordering the root of the hierarchy.
                DrawDragTargetForRootEntity(registry, entity, treeNodeMin, treeNodeMax, nodeOpen);
            }
        }

        // Right click context menu:
        if (!isRenaming && registry.IsValidEntity(entity))
            DrawEntityContextMenu(registry, entity);
        
        if (nodeOpen)
        {
            for (auto& childID : nodeComp.m_childrenIDs)
            {
                auto childHandle = registry.GetEntity(childID);
                if (childHandle != kInvalidEntityHandle)
                {
                    Entity childEntity(registry, childHandle);
                    DrawWorldEntityNode(registry, childEntity);
                }
            }
            
            ImGui::TreePop();
        }
    }

    void HierarchyWindow::DrawGlobalEntityNode(EntityRegistry& registry, Entity& entity)
    {
        auto& idComp = entity.GetComponent<IDComponent>();
        const auto& name = idComp.GetName();
        
        // Check if this entity matches the search
        const bool matchesSearch = m_filter.PassFilter(name.c_str());
        
        // If this node or any children of this node don't match the current search filter, skip.
        if (!matchesSearch)
            return;
        
        auto* pImGuiWindow = ImGui::GetCurrentWindow();
        pImGuiWindow->DC.CurrLineSize.y = kRowHeight;
        
        ImGui::TableNextRow();
        ImGui::TableNextColumn();
        
        ui::ScopedID scopedID(idComp.GetID());
        pImGuiWindow->DC.CurrLineTextBaseOffset = 3.f;

        // Is the Entity Selected:
        const bool isSelected = editor::SelectionManager::IsSelectedGlobal(idComp.GetID());
        const bool isRenaming = (m_currentRenameEntity == idComp.GetID());

        ImGuiTreeNodeFlags nodeFlags = ImGuiTreeNodeFlags_SpanAvailWidth
            | ImGuiTreeNodeFlags_Leaf;

        if (isSelected)
            nodeFlags |= ImGuiTreeNodeFlags_Selected;

        // Ensure that the nodes don't auto highlight when dragging.
        if (m_isDraggingEntity)
        {
            ImGui::PushStyleColor(ImGuiCol_HeaderHovered, ImVec4(0, 0, 0, 0));
        }

        bool nodeOpen;
        if (isRenaming)
        {
            DrawEntityNodeRename(idComp, nodeOpen, nodeFlags);
        }
        else
        {
            // Not renaming, normal Tree Node:
            nodeOpen = ImGui::TreeNodeEx("", nodeFlags, "%s", name.c_str());
            
            if (ImGui::IsItemClicked(ImGuiMouseButton_Left))
            {
                const bool ctrlHeld = ImGui::GetIO().KeyCtrl;

                if (ctrlHeld)
                {
                    // Ctrl+Click: Toggle this entity in selection
                    if (editor::SelectionManager::IsSelectedGlobal(idComp.GetID()))
                        editor::SelectionManager::DeselectGlobal(idComp.GetID());
                    else
                        editor::SelectionManager::Select(editor::SelectionManager::kGlobalContext, idComp.GetID());
                }
                else
                {
                    // Normal click, select only this entity.
                    editor::SelectionManager::SelectGlobalUnique(idComp.GetID());
                }

                m_selectionContainsGlobalEntity = true;
            }

            if (isSelected && ImGui::IsWindowFocused())
            {
                // F2 to rename the current selected entity.
                if (ImGui::IsKeyPressed(ImGuiKey_F2))
                {
                    m_currentRenameEntity = idComp.GetID();
                    m_shouldFocusRename = true;
                }

                // Delete to delete the current selected entity.
                if (ImGui::IsKeyPressed(ImGuiKey_Delete))
                {
                    DeleteEntityAndChildren(registry, idComp.GetID());
                }
            }
        }

        // Pop the highlight disable styling.
        if (m_isDraggingEntity)
        {
            ImGui::PopStyleColor();
        }

        const ImVec2 treeNodeMin = ImGui::GetItemRectMin();
        const ImVec2 treeNodeMax = ImGui::GetItemRectMax();

        // Drag Source - must come immediately after the tree node.
        if (ImGui::BeginDragDropSource(ImGuiDragDropFlags_SourceAllowNullID))
        {
            const auto& selectedEntities = editor::SelectionManager::GetSelections(editor::SelectionManager::kGlobalContext);
            auto entityID = idComp.GetID();
            
            if (editor::SelectionManager::IsSelectedGlobal(entityID))
            {
                ImGui::TextUnformatted(name.c_str());
                ImGui::SetDragDropPayload(kEntityHierarchyDropPayloadName, &entityID, 1 * sizeof(EntityID));
            }
            else
            {
                for (const auto& selectedID : selectedEntities)
                {
                    const auto handle = registry.GetEntity(selectedID);
                    const auto& selectedIDComp = registry.GetComponent<IDComponent>(handle);
                    ImGui::TextUnformatted(selectedIDComp.GetName().c_str());
                }
        
                ImGui::SetDragDropPayload(kEntityHierarchyDropPayloadName, selectedEntities.data(), selectedEntities.size() * sizeof(UUID));
            }
            
            ImGui::EndDragDropSource();
        }

        // Drag Target. No parenting allowed, but you can reorder the hierarchy.
        if (m_isDraggingEntity)
        {
            DrawDragTargetForRootEntity(registry, entity, treeNodeMin, treeNodeMax, nodeOpen);
        }

        // Right click context menu:
        if (!isRenaming && registry.IsValidEntity(entity))
            DrawEntityContextMenu(registry, entity);

        // End the Node
        if (nodeOpen)
        {
            ImGui::TreePop();
        }
    }

    void HierarchyWindow::DrawEntityNodeRename(IDComponent& idComp, bool& nodeOpen, const int treeNodeFlags)
    {
        // Save cursor position before tree node
        const ImVec2 labelPos = ImGui::GetCursorScreenPos();
            
        // Show input field instead of tree node label
        // - Still need the tree node for hierarchy structure, but the label is just space. It can't be an empty string because ImGui will not save
        //   the open state for it.
        nodeOpen = ImGui::TreeNodeEx("", treeNodeFlags, " ");

        // Restore cursor to where the label would be drawn
        ImGui::SetCursorScreenPos(labelPos);
        
        // Offset cursor to account for tree node arrow/indent
        const float indent = ImGui::GetTreeNodeToLabelSpacing();
        ImGui::SetCursorScreenPos(ImVec2(labelPos.x + indent, labelPos.y));
        ImGui::SetNextItemWidth(ImGui::GetContentRegionAvail().x);

        // Initialize buffer on first frame
        if (m_shouldFocusRename)
        {
            strncpy_s(m_inputBuffer, idComp.GetName().c_str(), kInputBufferSize - 1);
            m_inputBuffer[kInputBufferSize - 1] = '\0';
            ImGui::SetKeyboardFocusHere();
            m_shouldFocusRename = false;
        }

        static constexpr ImGuiInputTextFlags kInputTextFlags = ImGuiInputTextFlags_EnterReturnsTrue 
            | ImGuiInputTextFlags_AutoSelectAll
            | ImGuiInputTextFlags_CharsNoBlank; // Prevents spaces.

        if (ImGui::InputText("##rename_input", m_inputBuffer, kInputBufferSize, kInputTextFlags))
        {
            // Only apply new name if buffer is not empty
            if (m_inputBuffer[0] != '\0')
                idComp.SetName(m_inputBuffer);
            
            // If empty, just cancel the rename
            m_currentRenameEntity = kInvalidEntityID;
        }

        // Cancel on Escape or lost focus (but only after the initial focus is set).
        if (ImGui::IsKeyPressed(ImGuiKey_Escape) || (!m_shouldFocusRename && ImGui::IsItemDeactivated() && !ImGui::IsItemDeactivatedAfterEdit()))
        {
            m_currentRenameEntity = kInvalidEntityID;
        }
    }

    void HierarchyWindow::DrawDragTargetForWorldEntity(EntityRegistry& registry, Entity& entity, const ImVec2 treeNodeMin, const ImVec2 treeNodeMax, const bool removeInsertBelow)
    {
        const ImVec2 mousePos = ImGui::GetMousePos();

        // Check if mouse is within the horizontal AND vertical bounds of the node
        if (mousePos.x < treeNodeMin.x
            || mousePos.x > treeNodeMax.x
            || mousePos.y < treeNodeMin.y
            || mousePos.y > treeNodeMax.y)
        {
            return;
        }

        // Split the node into thirds:
        static constexpr float kReorderBoundsRatio = 0.15f;
        
        const float nodeReorderButtonSize = (treeNodeMax.y - treeNodeMin.y) * kReorderBoundsRatio;
        float nodeParentButtonSize = (treeNodeMax.y - treeNodeMin.y) - (nodeReorderButtonSize * 2.f);
        
        if (removeInsertBelow)
        {
            // Remove the bottom third by expanding the middle zone.
            nodeParentButtonSize += nodeReorderButtonSize;
        }
        
        const float nodeDividerTop = treeNodeMin.y + nodeReorderButtonSize;
        const float nodeDividerBottom = nodeDividerTop + nodeParentButtonSize;
        
        ImDrawList* drawList = ImGui::GetWindowDrawList();
        const ImU32 highlightColor = ImGui::GetColorU32(ImGuiCol_DragDropTarget);

        // Upper third, move the entity to be above the hovered node.
        if (mousePos.y < nodeDividerTop)
        {
            // Create invisible button for the drop target
            ImGui::SetCursorScreenPos(ImVec2(treeNodeMin.x, treeNodeMin.y));
            ui::ScopedID dividerID("divider_before");
            ImGui::InvisibleButton("##drop_before", ImVec2(treeNodeMax.x - treeNodeMin.x, nodeReorderButtonSize));
            
            if (ImGui::BeginDragDropTarget())
            {
                const ImGuiPayload* payload = ImGui::AcceptDragDropPayload(kEntityHierarchyDropPayloadName, ImGuiDragDropFlags_AcceptNoDrawDefaultRect);
                m_isDraggingEntity = false;
                if (payload)
                {
                    // Insert BEFORE this entity
                    const size_t count = payload->DataSize / sizeof(uint64);
                    for (size_t i = 0; i < count; ++i)
                    {
                        EntityID droppedID = *(static_cast<uint64*>(payload->Data) + i);
                        auto droppedEntity = registry.GetEntity(droppedID);
                        auto& entityNodeComp = registry.GetComponent<NodeComponent>(entity);

                        // Reparent - this will update the root status.
                        m_pWorld->ParentEntity(droppedEntity, entity);
                        
                        if (entityNodeComp.m_parentID != kInvalidEntityID)
                        {
                            auto parentHandle = registry.GetEntity(entityNodeComp.m_parentID);
                            auto& parentNodeComp = registry.GetComponent<NodeComponent>(parentHandle);

                            // Find current entity and insert AFTER it
                            auto current = std::ranges::find(parentNodeComp.m_childrenIDs, droppedID);
                            auto target = std::ranges::find(parentNodeComp.m_childrenIDs, entity.GetID());

                            NES_ASSERT(current != parentNodeComp.m_childrenIDs.end());
                            NES_ASSERT(target != parentNodeComp.m_childrenIDs.end());
                            MoveElement(parentNodeComp.m_childrenIDs, current, target, false);
                        }
                    }
                }
                ImGui::EndDragDropTarget();

                // Draw a divider line.
                drawList->AddLine(ImVec2(treeNodeMin.x, treeNodeMin.y), ImVec2(treeNodeMax.x, treeNodeMin.y), highlightColor, kDividerLineThickness);
            }
        }

        // Middle third, parent the entity to the node.
        else if (mousePos.y < nodeDividerBottom)
        {
            ImGui::SetCursorScreenPos(treeNodeMin);
            ui::ScopedID parentID("parent_drop");
            ImGui::InvisibleButton("##drop_parent", ImVec2(treeNodeMax.x - treeNodeMin.x, treeNodeMax.y - treeNodeMin.y));

            if (ImGui::BeginDragDropTarget())
            {
                const ImGuiPayload* payload = ImGui::AcceptDragDropPayload(kEntityHierarchyDropPayloadName, ImGuiDragDropFlags_AcceptNoDrawDefaultRect);
                m_isDraggingEntity = false;
                if (payload)
                {
                    const size_t count = payload->DataSize / sizeof(uint64);
                    for (size_t i = 0; i < count; ++i)
                    {
                        const EntityID droppedID = *(static_cast<uint64*>(payload->Data) + i);
                        m_pWorld->ParentEntity(registry.GetEntity(droppedID), entity);
                    }

                    m_forceOpenEntity = entity.GetID();                    
                }
                ImGui::EndDragDropTarget();
                
                // Highlight the whole node.
                drawList->AddRect(treeNodeMin, treeNodeMax, highlightColor, 0.f, 0, kDividerLineThickness);
            }
        }

        // Bottom third, move the entity to be below the hovered node.
        else if (!removeInsertBelow)
        {
            // Create invisible button for the drop target
            ImGui::SetCursorScreenPos(ImVec2(treeNodeMin.x, nodeDividerBottom));
            ui::ScopedID dividerID("divider_after");
            ImGui::InvisibleButton("##drop_after", ImVec2(treeNodeMax.x - treeNodeMin.x, nodeReorderButtonSize));

            if (ImGui::BeginDragDropTarget())
            {
                const ImGuiPayload* payload = ImGui::AcceptDragDropPayload(kEntityHierarchyDropPayloadName, ImGuiDragDropFlags_AcceptNoDrawDefaultRect);
                m_isDraggingEntity = false;
                if (payload)
                {
                    // Insert AFTER this entity
                    const size_t count = payload->DataSize / sizeof(uint64);
                    for (size_t i = 0; i < count; ++i)
                    {
                        EntityID droppedID = *(static_cast<uint64*>(payload->Data) + i);
                        auto& entityNodeComp = registry.GetComponent<NodeComponent>(entity);
                        m_pWorld->ParentEntity(droppedID, entityNodeComp.m_parentID);
                        
                        if (entityNodeComp.m_parentID != kInvalidEntityID)
                        {
                            auto parentHandle = registry.GetEntity(entityNodeComp.m_parentID);
                            auto& parentNodeComp = registry.GetComponent<NodeComponent>(parentHandle);

                            // Find current entity and insert AFTER it
                            auto current = std::ranges::find(parentNodeComp.m_childrenIDs, droppedID);
                            auto target = std::ranges::find(parentNodeComp.m_childrenIDs, entity.GetID());

                            NES_ASSERT(current != parentNodeComp.m_childrenIDs.end());
                            NES_ASSERT(target != parentNodeComp.m_childrenIDs.end());
                            MoveElement(parentNodeComp.m_childrenIDs, current, target, true);
                        }
                    }
                }
                ImGui::EndDragDropTarget();

                // Draw a divider line.
                drawList->AddLine(ImVec2(treeNodeMin.x, treeNodeMax.y), ImVec2(treeNodeMax.x, treeNodeMax.y), highlightColor, kDividerLineThickness);
            }
        }
    }

    void HierarchyWindow::DrawDragTargetForRootEntity(EntityRegistry& registry, Entity& entity, const ImVec2 treeNodeMin, const ImVec2 treeNodeMax, const bool removeInsertBelow)
    {
        const ImVec2 mousePos = ImGui::GetMousePos();

        // Check if mouse is within the horizontal AND vertical bounds of the node
        if (mousePos.x < treeNodeMin.x
            || mousePos.x > treeNodeMax.x
            || mousePos.y < treeNodeMin.y
            || mousePos.y > treeNodeMax.y)
        {
            return;
        }

        auto& idComp = entity.GetIDComponent();
        auto* pNodeComponent = registry.TryGetComponent<NodeComponent>(entity);
        const bool canParent = !m_selectionContainsGlobalEntity && (pNodeComponent != nullptr);

        // Split the node into thirds:
        static constexpr float kReorderBoundsRatio = 0.15f;
        
        const float nodeReorderButtonSize = (treeNodeMax.y - treeNodeMin.y) * kReorderBoundsRatio;
        float nodeParentButtonSize = (treeNodeMax.y - treeNodeMin.y) - (nodeReorderButtonSize * 2.f);
        
        if (removeInsertBelow)
        {
            // Remove the bottom third by expanding the middle zone.
            nodeParentButtonSize += nodeReorderButtonSize;
        }
        
        const float nodeDividerTop = treeNodeMin.y + nodeReorderButtonSize;
        const float nodeDividerBottom = nodeDividerTop + nodeParentButtonSize;
        ImDrawList* drawList = ImGui::GetWindowDrawList();
        const ImU32 highlightColor = ImGui::GetColorU32(ImGuiCol_DragDropTarget);

        // Upper third, move the entity to be above the hovered node in the root hierarchy:
        if (mousePos.y < nodeDividerTop)
        {
            // Create invisible button for the drop target
            ImGui::SetCursorScreenPos(ImVec2(treeNodeMin.x, treeNodeMin.y));
            ui::ScopedID dividerID("divider_before");
            ImGui::InvisibleButton("##drop_before", ImVec2(treeNodeMax.x - treeNodeMin.x, nodeReorderButtonSize));

            if (ImGui::BeginDragDropTarget())
            {
                const ImGuiPayload* payload = ImGui::AcceptDragDropPayload(kEntityHierarchyDropPayloadName, ImGuiDragDropFlags_AcceptNoDrawDefaultRect);
                m_isDraggingEntity = false;
                
                if (payload)
                {
                    const size_t count = payload->DataSize / sizeof(uint64);
                    for (size_t i = 0; i < count; ++i)
                    {
                        const EntityID droppedID = *(static_cast<uint64*>(payload->Data) + i);
                        auto droppedEntity = registry.GetEntity(droppedID);
                        if (registry.HasComponent<NodeComponent>(droppedEntity))
                        {
                            m_pWorld->RemoveParent(droppedEntity);
                        }
                        
                        m_pWorld->ReorderRootEntity(droppedID, idComp.GetID(), false);
                    }
                }
                ImGui::EndDragDropTarget();

                // Draw a divider line.
                drawList->AddLine(ImVec2(treeNodeMin.x, treeNodeMin.y), ImVec2(treeNodeMax.x, treeNodeMin.y), highlightColor, kDividerLineThickness);
            }
        }

        // Middle third, parent the entity to the node, if allowed.
        else if (canParent && mousePos.y < nodeDividerBottom)
        {
            ImGui::SetCursorScreenPos(treeNodeMin);
            ui::ScopedID parentID("parent_drop");
            ImGui::InvisibleButton("##drop_parent", ImVec2(treeNodeMax.x - treeNodeMin.x, treeNodeMax.y - treeNodeMin.y));

            if (ImGui::BeginDragDropTarget())
            {
                const ImGuiPayload* payload = ImGui::AcceptDragDropPayload(kEntityHierarchyDropPayloadName, ImGuiDragDropFlags_AcceptNoDrawDefaultRect);
                m_isDraggingEntity = false;
                if (payload)
                {
                    const size_t count = payload->DataSize / sizeof(uint64);
                    for (size_t i = 0; i < count; ++i)
                    {
                        const EntityID droppedID = *(static_cast<uint64*>(payload->Data) + i);
                        m_pWorld->ParentEntity(registry.GetEntity(droppedID), entity);
                    }

                    m_forceOpenEntity = entity.GetID();
                }
                ImGui::EndDragDropTarget();
                
                // Highlight the whole node.
                drawList->AddRect(treeNodeMin, treeNodeMax, highlightColor, 0.f, 0, kDividerLineThickness);
            }
        }

        // Bottom third, move the entity after the hovered node.
        else if (!removeInsertBelow)
        {
            // Create invisible button for the drop target
            ImGui::SetCursorScreenPos(ImVec2(treeNodeMin.x, nodeDividerBottom));
            ui::ScopedID dividerID("divider_after");
            ImGui::InvisibleButton("##drop_after", ImVec2(treeNodeMax.x - treeNodeMin.x, nodeReorderButtonSize));

            if (ImGui::BeginDragDropTarget())
            {
                const ImGuiPayload* payload = ImGui::AcceptDragDropPayload(kEntityHierarchyDropPayloadName, ImGuiDragDropFlags_AcceptNoDrawDefaultRect);
                m_isDraggingEntity = false;
                if (payload)
                {
                    const size_t count = payload->DataSize / sizeof(uint64);
                    for (size_t i = 0; i < count; ++i)
                    {
                        const EntityID droppedID = *(static_cast<uint64*>(payload->Data) + i);
                        auto droppedEntity = registry.GetEntity(droppedID);
                        if (registry.HasComponent<NodeComponent>(droppedEntity))
                        {
                            m_pWorld->RemoveParent(droppedEntity);
                        }
                        m_pWorld->ReorderRootEntity(droppedID, idComp.GetID(), true);
                    }
                }
                ImGui::EndDragDropTarget();

                // Draw a divider line.
                drawList->AddLine(ImVec2(treeNodeMin.x, treeNodeMax.y), ImVec2(treeNodeMax.x, treeNodeMax.y), highlightColor, kDividerLineThickness);
            }
        }
    }

    void HierarchyWindow::DrawGlobalContextMenu(EntityRegistry& registry)
    {
        if (ImGui::BeginPopupContextWindow("##global_context_menu", ImGuiPopupFlags_MouseButtonRight | ImGuiPopupFlags_NoOpenOverItems))
        {
            auto& selected = editor::SelectionManager::GetSelections(editor::SelectionManager::kGlobalContext);

            // Create Menu:
            if (ImGui::BeginMenu("Create"))
            {
                if (ImGui::MenuItem("Global"))
                {
                    CreateNewGlobalEntity(registry);
                }
                if (ImGui::IsItemHovered(ImGuiHoveredFlags_DelayNormal))
                {
                    ImGui::SetTooltip("Creates an Entity without a Transform. This can be used for Managers, Settings, Script-only Entities, etc.");
                }

                if (ImGui::MenuItem("World"))
                {
                    CreateNewWorldEntity(registry);
                }
                if (ImGui::IsItemHovered(ImGuiHoveredFlags_DelayNormal))
                {
                    ImGui::SetTooltip("Creates an Entity that can be placed in the world.");
                }

                ImGui::EndMenu();
            }

            if (!selected.empty())
            {
                ImGui::Spacing();
                ImGui::Separator();
                ImGui::Spacing();
        
                // Disabled text as non-interactive header
                ImGui::BeginDisabled();
                if (selected.size() == 1)
                {
                    const auto handle = registry.GetEntity(selected[0]);
                    auto& idComp = registry.GetComponent<IDComponent>(handle);
                    ImGui::TextUnformatted(idComp.GetName().c_str());
                }
                else
                {
                    ImGui::Text("%zu Entities", selected.size());
                }
                ImGui::EndDisabled();
                ImGui::Spacing();
            }
            
            if (selected.size() == 1ull)
            {
                const EntityID selectedID = selected[0];
                if (!registry.IsValidEntity(selectedID))
                {
                    editor::SelectionManager::DeselectGlobal(selectedID);
                    ImGui::EndPopup();
                    return;
                }
                                
                const auto handle = registry.GetEntity(selectedID);
                auto& idComp = registry.GetComponent<IDComponent>(handle);
                auto* pNodeComp = registry.TryGetComponent<NodeComponent>(handle);
                DrawSingleSelectedEntityMenuOptions(registry, idComp, pNodeComp);
            }
            else if (selected.size() > 1)
            {
                DrawMultSelectedEntityMenuOptions(registry, selected);
            }
            ImGui::EndPopup();
        }
    }

    void HierarchyWindow::DrawEntityContextMenu(EntityRegistry& registry, Entity& entity)
    {
        auto& idComp = entity.GetIDComponent();
        auto* pNodeComp = entity.TryGetComponent<NodeComponent>();
        
        if (ImGui::BeginPopupContextItem("##entity_context_menu"))
        {
            // Set the entity as the selected one:
            if (!editor::SelectionManager::IsSelectedGlobal(idComp.GetID()))
            {
                editor::SelectionManager::SelectGlobalUnique(idComp.GetID());
                
                if (pNodeComp != nullptr)
                    m_selectionContainsGlobalEntity = true;
            }

            auto& selected = editor::SelectionManager::GetSelections(editor::SelectionManager::kGlobalContext);

            if (selected.size() == 1ull)
            {
                DrawSingleSelectedEntityMenuOptions(registry, idComp, pNodeComp);
            }
            else if (selected.size() > 1ull)
            {
                // Disabled text as non-interactive header
                ImGui::BeginDisabled();
                ImGui::Text("%zu Entities", selected.size());
                ImGui::EndDisabled();
                ImGui::Spacing();

                DrawMultSelectedEntityMenuOptions(registry, selected);
            }
            
            ImGui::EndPopup();
        }
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

                // Return found results.
                if (NameSearchRecursive(registry, childEntity, maxSearchDepth, currentDepth + 1))
                    return true;
            }
        }
        
        return false;
    }

    void HierarchyWindow::CreateNewGlobalEntity(EntityRegistry& registry)
    {
        auto newEntity = registry.CreateEntity("NewEntity");
        auto& idComp = registry.GetComponent<IDComponent>(newEntity);
        
        // Select the new entity:
        editor::SelectionManager::SelectGlobalUnique(idComp.GetID());

        // Immediately start the rename:
        m_currentRenameEntity = idComp.GetID();
        m_shouldFocusRename = true;
    }

    void HierarchyWindow::CreateNewWorldEntity(EntityRegistry& registry, const EntityID parent)
    {
        // Create an Entity.
        auto newChild = m_pWorld->CreateEntity("NewEntity");
        auto& newChildIDComp = registry.GetComponent<IDComponent>(newChild);

        // Parent to the current entity:
        if (parent != kInvalidEntityID)
        {
            // [Consider]: Instead of this check, you can have the World object have an overload that takes in
            // a Parent ID, which can handle this case. That way, we don't need to worry about the Transform Component
            // at all. We simply pass the parent on, and it handles it.
            const auto parentEntity = registry.GetEntity(parent);
            if (auto* pParentTransform = registry.TryGetComponent<TransformComponent>(parentEntity))
            {
                // Match the parent's world transform exactly.
                registry.AddComponent<TransformComponent>(newChild, *pParentTransform);
            }
            
            m_pWorld->ParentEntity(newChild, parentEntity);

            // Force the entity to be open on the next draw.
            m_forceOpenEntity = parent;
        }
        
        // Select the new child:
        editor::SelectionManager::SelectGlobalUnique(newChildIDComp.GetID());

        // Immediately start the rename:
        m_currentRenameEntity = newChildIDComp.GetID();
        m_shouldFocusRename = true;
    }
    
    void HierarchyWindow::DeleteEntityAndChildren(EntityRegistry& registry, const EntityID& entityID) const
    {
        if (entityID == kInvalidEntityID)
            return;

        auto entityHandle = registry.GetEntity(entityID);

        // Delete all children recursively.
        if (auto* pNodeComp = registry.TryGetComponent<NodeComponent>(entityHandle))
        {
            for (auto child : pNodeComp->m_childrenIDs)
            {
                DeleteEntityAndChildren(registry, child);
            }    
        }

        NES_ASSERT(m_pWorld != nullptr);
        m_pWorld->DestroyEntity(entityHandle);

        // Deselect the entity from all contexts:
        if (editor::SelectionManager::IsSelectedGlobal(entityID))
        {
            editor::SelectionManager::DeselectGlobal(entityID);
        }
    }

    void HierarchyWindow::DrawSingleSelectedEntityMenuOptions(EntityRegistry& registry, const IDComponent& idComp, const NodeComponent* pNodeComp)
    {
        if (pNodeComp && ImGui::MenuItem("Add Child"))
        {
            CreateNewWorldEntity(registry, idComp.GetID());
        }

        if (pNodeComp && pNodeComp->HasParent() && ImGui::MenuItem("Unparent"))
        {
            m_pWorld->RemoveParent(idComp.GetID());
        }
                
        if (ImGui::MenuItem("Rename", "F2"))
        {
            m_currentRenameEntity = idComp.GetID();
            m_shouldFocusRename = true;
        }
                
        if (ImGui::MenuItem("Delete", "Del"))
        {
            DeleteEntityAndChildren(registry, idComp.GetID());
        }
    }

    void HierarchyWindow::DrawMultSelectedEntityMenuOptions(EntityRegistry& registry, const std::vector<uint64>& selected) const
    {
        if (!m_selectionContainsGlobalEntity && ImGui::MenuItem("Unparent All"))
        {
            for (auto selectedID : selected)
            {
                m_pWorld->RemoveParent(selectedID);
            }
        }

        if (ImGui::MenuItem("Deselect"))
        {
            editor::SelectionManager::DeselectAll(editor::SelectionManager::kGlobalContext);
        }
            
        if (ImGui::MenuItem("Delete All", "Del"))
        {
            // Making a copy since this will modify the array.
            std::vector<uint64> selectedCopy = selected;
            for (auto selectedID : selectedCopy)
            {
                DeleteEntityAndChildren(registry, selectedID);
            }
        }
    }
}
