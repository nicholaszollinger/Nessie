// EntityInspector.cpp
#include "EntityInspector.h"
#include "ComponentInspector.h"
#include "Components/TransformComponentInspector.h"
#include "Nessie/Editor/PropertyTable.h"
#include "Nessie/Graphics/ImGui/ImGuiUtils.h"

namespace nes
{
    EntityInspector::EntityInspector()
    {
        // Register the core component inspectors.
        EditorInspectorRegistry::RegisterInspector<IDComponentInspector>();
        EditorInspectorRegistry::RegisterInspector<NodeComponentInspector>();
        EditorInspectorRegistry::RegisterInspector<PendingDestructionInspector>();
        EditorInspectorRegistry::RegisterInspector<PendingInitializationInspector>();
        EditorInspectorRegistry::RegisterInspector<PendingEnableInspector>();
        EditorInspectorRegistry::RegisterInspector<PendingDisableInspector>();
        EditorInspectorRegistry::RegisterInspector<DisabledComponentInspector>();

        // For debugging:
        //m_inspectorShowFlags = EInspectorLevel::Internal | EInspectorLevel::DebugOnly;
    }

    void EntityInspector::DrawImpl(EntityHandle* pTarget, const InspectorContext& context)
    {
        // Validate the target:
        if (pTarget == nullptr || context.m_pWorld == nullptr || *pTarget == kInvalidEntityHandle)
        {
            // If the last selected entity is still valid, render it.
            auto* pRegistry = context.m_pWorld->GetEntityRegistry();
            if (m_lastSelected != kInvalidEntityID && pRegistry && pRegistry->IsValidEntity(m_lastSelected))
            {
                auto entity = pRegistry->GetEntity(m_lastSelected);
                DrawComponentList(*pRegistry, entity);
                DrawSelectedComponentDetails(*pRegistry, entity, context);
            }
            else
            {
                // Current target and last target are invalid, reset.
                m_lastSelected = kInvalidEntityID;
                m_selectedComponentType = std::numeric_limits<size_t>::max();
            }
            
            return;
        }

        // Render the new entity:
        EntityHandle& entity = *pTarget;

        if (auto* pRegistry = context.m_pWorld->GetEntityRegistry())
        {
            auto id = pRegistry->GetComponent<IDComponent>(entity).GetID();
            if (m_lastSelected != id)
            {
                m_selectedComponentType = std::numeric_limits<size_t>::max();
                m_lastSelected = id;
                AssembleComponentInspectors();
            }

            DrawComponentList(*pRegistry, entity);
            DrawSelectedComponentDetails(*pRegistry, entity, context);
        }
    }

    void EntityInspector::DrawComponentList(EntityRegistry& registry, EntityHandle entity)
    {
        // Render "Add" button that opens a dropdown of all attachable components.
        // [TODO]: Add a green '+' icon, to make it look nicer.
        if (ImGui::Button("+Add"))
        {
            m_searchFilter.Clear();
            ImGui::OpenPopup("AddComponentPopup");
        }
        
        DrawAddComponentDropdown(registry, entity);

        // Track the available components, so that if none are currently selected, the first potential type is auto selected.
        std::vector<size_t> selectableComponents;
        selectableComponents.reserve(m_componentInspectors.size());

        NES_UI_SCOPED_STYLE(ImGuiStyleVar_WindowMinSize, ImVec2(0, 75));
        if (ImGui::BeginChild("##Component_Hierarchy_Section", ImVec2(0, 200), ImGuiChildFlags_ResizeY | ImGuiChildFlags_Border | ImGuiChildFlags_NavFlattened))
        {
            // Render the Table of Components.
            if (ImGui::BeginTable("##Component_Hierarchy_Table", 1, ImGuiTableFlags_ScrollY))
            {
                // Track component to remove (can't remove during iteration)
                size_t componentToRemove = std::numeric_limits<size_t>::max();
                
                for (size_t i = 0; i < m_componentInspectors.size(); i++)
                {
                    auto& pInspector = m_componentInspectors[i];
            #if NES_DEBUG
                    // Check Internal inspectors allowed.
                    if (pInspector->IsInternal() && (m_inspectorShowFlags & EInspectorLevel::Internal) == 0)
                        continue;
            #else
                    if (pInspector->IsInternal())
                        continue;
            #endif

                    // Check Debug-Only inspectors allowed.
                    if (pInspector->IsDebugOnly() && (m_inspectorShowFlags & EInspectorLevel::DebugOnly) == 0)
                        continue;
                    
                    // Check if the entity has this component:
                    if (!registry.HasComponent(pInspector->GetTargetTypeID(), entity))
                        continue;

                    // We can select this component type.
                    selectableComponents.push_back(i);
                    std::string name = pInspector->GetTargetShortTypename();
                    
                    ImGui::TableNextRow();
                    ImGui::TableNextColumn();
                    NES_ASSERT(!name.empty());
                    NES_UI_SCOPED_ID(name.c_str());
                    ImGuiTreeNodeFlags nodeFlags = ImGuiTreeNodeFlags_Leaf;

                    if (m_selectedComponentType == i)
                        nodeFlags |= ImGuiTreeNodeFlags_Selected;
                    
                    ImGui::TreeNodeEx("", nodeFlags, "%s", name.c_str());
                    if (ImGui::IsItemFocused())
                    {
                        m_selectedComponentType = i;
                    }

                    // Right-click context menu
                    // [TODO]: Have a way of determining if the Component can actually be removed or not, instead of
                    // these specific checks.
                    if (ImGui::IsItemClicked(ImGuiMouseButton_Right)
                        && m_componentInspectors[i]->GetTargetTypeID() != entt::type_id<IDComponent>().hash()
                        && m_componentInspectors[i]->GetTargetTypeID() != entt::type_id<NodeComponent>().hash()
                        && m_componentInspectors[i]->GetTargetTypeID() != entt::type_id<TransformComponent>().hash())
                    {
                        ImGui::OpenPopup("ComponentContextMenu");
                        m_selectedComponentType = i;
                    }
                
                    // Context menu popup
                    if (ImGui::BeginPopup("ComponentContextMenu"))
                    {
                        if (ImGui::MenuItem("Remove Component"))
                        {
                            componentToRemove = i;
                        }
                        ImGui::EndPopup();
                    }
                    
                    ImGui::TreePop();
                }

                // Remove component after iteration
                if (componentToRemove != std::numeric_limits<size_t>::max())
                {
                    auto& pInspector = m_componentInspectors[componentToRemove];
                    registry.RemoveComponent(pInspector->GetTargetTypeID(), entity);
                
                    // Clear selection if we removed the selected component
                    if (m_selectedComponentType == componentToRemove)
                    {
                        m_selectedComponentType = std::numeric_limits<size_t>::max();
                    }

                    // Update the first selectable if removing.
                    if (!selectableComponents.empty() && selectableComponents[0] == componentToRemove)
                    {
                        selectableComponents.erase(selectableComponents.begin());
                    }
                }
            }
            ImGui::EndTable();
        }
        ImGui::EndChild();
        
        if (m_selectedComponentType == std::numeric_limits<size_t>::max() && !selectableComponents.empty())
            m_selectedComponentType = selectableComponents[0];
    }

    void EntityInspector::DrawSelectedComponentDetails(EntityRegistry& registry, EntityHandle entity, const InspectorContext& context)
    {
        if (ImGui::BeginChild("Details", ImVec2(0, 0)))
        {
            // No selected component.
            if (m_selectedComponentType == std::numeric_limits<size_t>::max())
            {
                ImGui::EndChild();
                return;
            }
            
            NES_ASSERT(m_selectedComponentType < m_componentInspectors.size());
            auto pInspector = m_componentInspectors[m_selectedComponentType];
            
            if (editor::CollapsableHeader(pInspector->GetTargetShortTypename()))
            {
                if (editor::BeginPropertyTable())
                {
                    void* pComponent = registry.TryGetComponentRaw(pInspector->GetTargetTypeID(), entity);
                    NES_ASSERT(pComponent != nullptr);
                    pInspector->Draw(pComponent, context);    
                }
                editor::EndPropertyTable();
            }
        }
        ImGui::EndChild();
    }

    void EntityInspector::AssembleComponentInspectors()
    {
        auto& componentRegistry = ComponentRegistry::Get();
        auto componentTypes = componentRegistry.GetAllComponentTypes();

        m_componentInspectors.clear();
        for (auto& componentType : componentTypes)
        {
            auto pInspector = EditorInspectorRegistry::GetInspector(componentType.m_typeID);
            if (pInspector != nullptr)
            {
                m_componentInspectors.emplace_back(pInspector);
            }
        }
    }

    void EntityInspector::DrawAddComponentDropdown(EntityRegistry& registry, EntityHandle entity)
    {
        auto& componentRegistry = ComponentRegistry::Get();

        ImGui::SetNextWindowSize(ImVec2(300, 400), ImGuiCond_FirstUseEver);

        if (ImGui::BeginPopup("AddComponentPopup"))
        {
            // Search bar at the top
            ImGui::SetNextItemWidth(-1);
            if (ImGui::IsWindowAppearing())
            {
                ImGui::SetKeyboardFocusHere();
            }
            m_searchFilter.Draw("##ComponentSearch", -1);
        
            ImGui::Separator();

            // Scrollable list of components
            if (ImGui::BeginChild("##ComponentList", ImVec2(0, 0), ImGuiChildFlags_None))
            {
                for (size_t i = 0; i < m_componentInspectors.size(); i++)
                {
                    auto& pInspector = m_componentInspectors[i];
                    
                #if NES_DEBUG
                    // Check Internal inspectors allowed.
                    if (pInspector->IsInternal() && (m_inspectorShowFlags & EInspectorLevel::Internal) == 0)
                        continue;
                #else
                    if (pInspector->IsInternal())
                        continue;
                #endif

                    // Check Debug-Only inspectors allowed.
                    if (pInspector->IsDebugOnly() && (m_inspectorShowFlags & EInspectorLevel::DebugOnly) == 0)
                        continue;
                    
                    // Skip if entity already has this component
                    if (registry.HasComponent(pInspector->GetTargetTypeID(), entity))
                        continue;
                    
                    std::string name = pInspector->GetTargetShortTypename();
                    
                    // Filter by search using ImGuiTextFilter
                    if (!m_searchFilter.PassFilter(name.c_str()))
                        continue;
                    
                    // Selectable component item
                    if (ImGui::Selectable(name.c_str()))
                    {
                        // Add the component to the entity
                        const ComponentTypeDesc* typeDesc = componentRegistry.GetComponentDescByTypeID(pInspector->GetTargetTypeID());
                        NES_ASSERT(typeDesc != nullptr, "Inspector Component type not registered to ComponentRegistry!");
                        typeDesc->m_addFunction(registry, entity);
                        
                        // Select the newly added component
                        m_selectedComponentType = i;
                        
                        // Clear filter and close popup
                        m_searchFilter.Clear();
                        ImGui::CloseCurrentPopup();
                    }
                }
            }
            
            ImGui::EndChild();
            ImGui::EndPopup();
        }
    }
}
