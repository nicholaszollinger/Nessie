// PropertyGrid.cpp
#include "PropertyTable.h"
#include "EditorCore.h"
#include "Nessie/Math/Axis.h"
#include "Nessie/World/EntityRegistry.h"
#include "Windows/HierarchyWindow.h"

namespace nes::editor
{
    // Space between Vector Axis controls (or other multi-component types).
    static constexpr float kAxisControlSpacing = 4.f;
    
    // Width of the colored indicator for axis controls.
    static constexpr float kAxisIndicatorWidth = 4.f;

    // Color for the X-Axis (Red).
    static constexpr auto kXAxisColor = IM_COL32(204, 26, 38, 255);

    // Color for the Y-Axis (Green).
    static constexpr auto kYAxisColor = IM_COL32(51, 179, 51, 255);

    // Color for the Z-Axis (Blue).
    static constexpr auto kZAxisColor = IM_COL32(26, 64, 224, 255);
    
    namespace internal
    {
        void BeginProperty(const char* label, const char* toolTip)
        {
            ImGui::TableNextRow();
            ImGui::PushID(label);
            ImGui::TableNextColumn();
            ImGui::AlignTextToFramePadding();
            ImGui::TextUnformatted(label);
        
            // Tool Tip
            if (std::strlen(toolTip) != 0)
            {
                SetToolTip(toolTip);
            }
        }

        void EndProperty()
        {
            ImGui::PopID();
        }

        bool BeginCollapsableProperty(const char* label, const char* toolTip)
        {
            ImGui::TableNextRow();
            ImGui::PushID(label);
            ImGui::TableNextColumn();

            ImGui::AlignTextToFramePadding();
            bool isOpen = ImGui::TreeNodeEx(label, ImGuiTreeNodeFlags_SpanFullWidth | ImGuiTreeNodeFlags_DefaultOpen);

            // Tool Tip
            if (std::strlen(toolTip) != 0)
            {
                SetToolTip(toolTip);
            }

            return isOpen;
        }

        void EndCollapsableProperty(const bool isOpen)
        {
            if (isOpen)
                ImGui::TreePop();
            
            ImGui::PopID();
        }

        void BeginPropertyValue(const bool isDisabled)
        {
            ImGui::TableNextColumn();
        
            if (isDisabled)
                ImGui::BeginDisabled();
        
            ImGui::PushItemWidth(-1.f);
        }

        void EndPropertyValue(const bool isDisabled)
        {
            ImGui::PopItemWidth();
        
            if (isDisabled)
                ImGui::EndDisabled();
        }
    }
    
    bool CollapsableHeader(const std::string& name, const bool openByDefault)
    {
        ImGuiTreeNodeFlags treeNodeFlags = ImGuiTreeNodeFlags_Framed
            | ImGuiTreeNodeFlags_SpanAvailWidth
            | ImGuiTreeNodeFlags_AllowOverlap
            | ImGuiTreeNodeFlags_FramePadding;

        if (openByDefault)
            treeNodeFlags |= ImGuiTreeNodeFlags_DefaultOpen;
        
        static constexpr float kFramePadding = 6.f;
        NES_UI_SCOPED_STYLE(ImGuiStyleVar_FrameRounding, 0.f);
        NES_UI_SCOPED_STYLE(ImGuiStyleVar_FramePadding, ImVec2(kFramePadding, kFramePadding));

        ImGui::PushID(name.c_str());
        bool isOpen = ImGui::CollapsingHeader(name.c_str(), treeNodeFlags);
        ImGui::PopID();
        return isOpen;
    }

    bool BeginPropertyTable()
    {
        ImGui::PushStyleColor(ImGuiCol_TableBorderLight, ImVec4(0.02f, 0.02f, 0.02f, 0.50f));
        
        static constexpr ImGuiTableFlags kFlags = ImGuiTableFlags_Resizable
            | ImGuiTableFlags_BordersInner
            | ImGuiTableFlags_NoSavedSettings
            | ImGuiTableFlags_ScrollY
            | ImGuiTableFlags_NoHostExtendY;
        
        bool isOpen = ImGui::BeginTable(GenerateID(), 2, kFlags);
        ImGui::TableSetupColumn("", ImGuiTableColumnFlags_WidthFixed, 100.f);
        ImGui::TableSetupColumn("", ImGuiTableColumnFlags_WidthStretch);

        PushID();
        
        return isOpen;
    }

    void EndPropertyTable()
    {
        PopID();
        ImGui::EndTable();
        ImGui::PopStyleColor(1);
    }

    bool Property(const char* label, std::string& value, const char* toolTip)
    {
        internal::BeginProperty(label, toolTip);
        
        // Value
        internal::BeginPropertyValue();
        static constexpr ImGuiInputTextFlags kInputTextFlags = ImGuiInputTextFlags_EnterReturnsTrue 
                | ImGuiInputTextFlags_AutoSelectAll;
        
        const bool modified = ImGui::InputText(std::format("##{0}", label).c_str(), &value, kInputTextFlags);
        
        internal::EndPropertyValue();
        internal::EndProperty();
        
        return modified;
    }

    void Property(const char* label, const std::string& value, const char* toolTip)
    {
        internal::BeginProperty(label, toolTip);

        // Value
        internal::BeginPropertyValue(true);
        ImGui::InputText(std::format("##{0}", label).c_str(), const_cast<char*>(value.data()), value.size(), ImGuiInputTextFlags_ReadOnly);
        internal::EndPropertyValue(true);

        internal::EndProperty();
    }

    bool Property(const char* label, uint64& value, const char* toolTip)
    {
        internal::BeginProperty(label, toolTip);
        internal::BeginPropertyValue();
        
        const bool modified = ImGui::InputScalar(std::format("##{0}", label).c_str(), ImGuiDataType_U64, &value);
        
        internal::EndPropertyValue();
        internal::EndProperty();
        
        return modified;
    }

    void Property(const char* label, const uint64& value, const char* toolTip)
    {
        internal::BeginProperty(label, toolTip);

        internal::BeginPropertyValue(true);
        ImGui::InputScalar(std::format("##{0}", label).c_str(), ImGuiDataType_U64, const_cast<uint64*>(&value));
        internal::EndPropertyValue(true);
        
        internal::EndProperty();
    }

    static bool DrawAxisControl(float& value, const EAxis axis, const float itemWidth, const float min = 0.f, const float max = 0.f, const float speed = 0.1f, const char* format = "%.3f")
    {
        ImU32 color;
        const char* indicatorLabel;
        const char* dragLabel;

        switch (axis)
        {
            case EAxis::X:
            {
                indicatorLabel = "##XIndicator";
                dragLabel = "##X";
                color = kXAxisColor;
                break;
            }

            case EAxis::Y:
            {
                indicatorLabel = "##YIndicator";
                dragLabel = "##Y";
                color = kYAxisColor;
                break;
            }

            default:
            {
                indicatorLabel = "##ZIndicator";
                dragLabel = "##Z";
                color = kZAxisColor;
                break;
            }
        }

        const float lineHeight = ImGui::GetFrameHeight();
        
        ImDrawList* drawList = ImGui::GetWindowDrawList();
        ImGui::InvisibleButton(indicatorLabel, ImVec2(kAxisIndicatorWidth + kAxisControlSpacing, lineHeight));
        ImVec2 rectMin = ImGui::GetItemRectMin();
        ImVec2 rectMax = ImGui::GetItemRectMax();
        rectMin.x += kAxisIndicatorWidth;
        drawList->AddRectFilled(rectMin, rectMax, color);
    
        ImGui::SameLine();
        ImGui::SetNextItemWidth(itemWidth);
        return ImGui::DragFloat(dragLabel, &value, speed, min, max, format);
    }

    bool Property(const char* label, Vec3& vec, const char* toolTip)
    {
        bool modified = false;
        internal::BeginProperty(label, toolTip);
        ImGui::TableNextColumn();
        ImGui::PushStyleVar(ImGuiStyleVar_ItemSpacing, ImVec2{0, 0});
        
        const float availWidth = ImGui::GetContentRegionAvail().x;
        const float dragWidth = (availWidth - kAxisIndicatorWidth * 3 - kAxisControlSpacing * 3) / 3.0f;

        modified |= DrawAxisControl(vec.x, EAxis::X, dragWidth);
        ImGui::SameLine();
        modified |= DrawAxisControl(vec.y, EAxis::Y, dragWidth);
        ImGui::SameLine();
        modified |= DrawAxisControl(vec.z, EAxis::Z, dragWidth);
        
        ImGui::PopStyleVar();
        internal::EndProperty();
        
        return modified;
    }

    void Property(const char* label, const Vec3& vec, const char* toolTip)
    {
        internal::BeginProperty(label, toolTip);
        ImGui::TableNextColumn();
        ImGui::PushStyleVar(ImGuiStyleVar_ItemSpacing, ImVec2{0, 0});
        
        const float availWidth = ImGui::GetContentRegionAvail().x;
        const float dragWidth = (availWidth - kAxisIndicatorWidth * 3 - kAxisControlSpacing * 3) / 3.0f;

        Vec3 copy = vec;
        ImGui::BeginDisabled();
        DrawAxisControl(copy.x, EAxis::X, dragWidth);
        ImGui::SameLine();
        DrawAxisControl(copy.y, EAxis::Y, dragWidth);
        ImGui::SameLine();
        DrawAxisControl(copy.z, EAxis::Z, dragWidth);
        ImGui::EndDisabled();
        
        ImGui::PopStyleVar();
        internal::EndProperty();
    }

    bool Property(const char* label, Rotation& rotation, const char* toolTip)
    {
        bool modified = false;
        
        internal::BeginProperty(label, toolTip);
        ImGui::TableNextColumn();
        ImGui::PushStyleVar(ImGuiStyleVar_ItemSpacing, ImVec2{0, 0});

        const float availWidth = ImGui::GetContentRegionAvail().x;
        const float dragWidth = (availWidth - kAxisIndicatorWidth * 3 - kAxisControlSpacing * 3) / 3.0f;

        static constexpr const char* kFormat = "%.1f°";
        static constexpr float kSpeed = 1.f;
        
        modified |= DrawAxisControl(rotation.m_pitch, EAxis::X, dragWidth, 0.f, 0.f, kSpeed, kFormat);
        ImGui::SameLine();
        modified |= DrawAxisControl(rotation.m_yaw, EAxis::Y, dragWidth, 0.f, 0.f, kSpeed, kFormat);
        ImGui::SameLine();
        modified |= DrawAxisControl(rotation.m_roll, EAxis::Z, dragWidth, 0.f, 0.f, kSpeed, kFormat);

        ImGui::PopStyleVar();
        internal::EndProperty();

        if (modified)
            rotation.Normalize();

        return modified;
    }

    void Property(const char* label, const Rotation& rotation, const char* toolTip)
    {
        internal::BeginProperty(label, toolTip);
        ImGui::TableNextColumn();
        ImGui::PushStyleVar(ImGuiStyleVar_ItemSpacing, ImVec2{0, 0});

        const float availWidth = ImGui::GetContentRegionAvail().x;
        const float dragWidth = (availWidth - kAxisIndicatorWidth * 3 - kAxisControlSpacing * 3) / 3.0f;

        static constexpr const char* kFormat = "%.1f°";
        static constexpr float kSpeed = 1.f;

        Rotation copy = rotation;
        ImGui::BeginDisabled();
        DrawAxisControl(copy.m_pitch, EAxis::X, dragWidth, 0.f, 0.f, kSpeed, kFormat);
        ImGui::SameLine();
        DrawAxisControl(copy.m_yaw, EAxis::Y, dragWidth, 0.f, 0.f, kSpeed, kFormat);
        ImGui::SameLine();
        DrawAxisControl(copy.m_roll, EAxis::Z, dragWidth, 0.f, 0.f, kSpeed, kFormat);
        ImGui::EndDisabled();
        
        ImGui::PopStyleVar();
        internal::EndProperty();
    }

    bool PropertyEntityID(const char* label, EntityID& entity, EntityRegistry& registry, const char* toolTip)
    {
        bool modified = false;
        internal::BeginProperty(label, toolTip);

        auto handle = registry.GetEntity(entity);
        std::string currentName = "None";
        if (handle != kInvalidEntityHandle)
        {
            auto& idComp = registry.GetComponent<IDComponent>(handle);
            currentName = idComp.GetName();
        }
        
        internal::BeginPropertyValue();

        // Drag-and-drop target
        if (ImGui::BeginDragDropTarget())
        {
            // Accept entity drag payload
            if (const ImGuiPayload* payload = ImGui::AcceptDragDropPayload(kEntityHierarchyDropPayloadName))
            {
                // Check if only a single entity is being dragged
                // Assuming payload contains entity count and ID(s)
                if (payload->DataSize == sizeof(EntityID))
                {
                    EntityID droppedEntity = *static_cast<EntityID*>(payload->Data);
                
                    // Verify the entity is valid
                    if (registry.GetEntity(droppedEntity) != kInvalidEntityHandle)
                    {
                        entity = droppedEntity;
                        modified = true;
                    }
                }
            }
        
            ImGui::EndDragDropTarget();
        }

        // Drop down for all entities.
        if (ImGui::BeginCombo("##EntityCombo", currentName.c_str()))
        {
            // [TODO]: Search bar using the ImGui Filter:
            
            // Add "None" option to clear the entity reference.
            if (ImGui::Selectable("None", entity == kInvalidEntityID))
            {
                entity = kInvalidEntityID;
                modified = true;
            }
            
            auto view = registry.GetAllEntitiesWith<IDComponent>();
            for (auto entityHandle : view)
            {
                auto& idComp = view.get<IDComponent>(entityHandle);
                const bool isSelected = (idComp.GetID() == entity);
            
                if (ImGui::Selectable(idComp.GetName().c_str(), isSelected))
                {
                    entity = idComp.GetID();
                    modified = true;
                }
            
                // Set initial focus on currently selected item
                if (isSelected)
                {
                    ImGui::SetItemDefaultFocus();
                }
            }
            ImGui::EndCombo();
        }

        internal::EndPropertyValue();
        internal::EndProperty();
        
        return modified;
    }

    void PropertyEntityID(const char* label, const EntityID& entity, EntityRegistry& registry, const char* toolTip)
    {
        internal::BeginProperty(label, toolTip);

        auto handle = registry.GetEntity(entity);
        std::string currentName = "None";
        if (handle != kInvalidEntityHandle)
        {
            auto& idComp = registry.GetComponent<IDComponent>(handle);
            currentName = idComp.GetName();
        }
        
        internal::BeginPropertyValue(true);
        
        ImGui::InputText("##EntityField", &currentName, ImGuiInputTextFlags_ReadOnly);
        
        internal::EndPropertyValue(true);
        internal::EndProperty();
    }
}
