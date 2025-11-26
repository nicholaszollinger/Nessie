// PropertyGrid.h
#pragma once
#include "EditorCore.h"
#include "Nessie/Asset/AssetBase.h"
#include "Nessie/Asset/AssetManager.h"
#include "Nessie/Core/Color.h"
#include "Nessie/Math/Math.h"
#include "Nessie/World/Components/IDComponent.h"

namespace nes::editor
{
    namespace internal
    {
        //----------------------------------------------------------------------------------------------------
        /// @brief : Begins the next row in the Property Table, writes the label in the first column, and sets
        ///  an optional tooltip if provided. Must be followed with a call to internal::EndProperty().
        //----------------------------------------------------------------------------------------------------
        void BeginProperty(const char* label, const char* toolTip);

        //----------------------------------------------------------------------------------------------------
        /// @brief : End the current Property scope.
        //----------------------------------------------------------------------------------------------------
        void EndProperty();

        //----------------------------------------------------------------------------------------------------
        /// @brief : Begins a property that can be collapsed itself. Used for arrays.
        /// Begins the next row in the Property Table, writes the label in the first column, and sets
        /// an optional tooltip if provided. Must be followed with a call to internal::EndCollapsableProperty().
        //----------------------------------------------------------------------------------------------------
        bool BeginCollapsableProperty(const char* label, const char* toolTip);

        //----------------------------------------------------------------------------------------------------
        /// @brief : End the current collapsable Property scope.
        //----------------------------------------------------------------------------------------------------
        void EndCollapsableProperty(const bool isOpen);

        //----------------------------------------------------------------------------------------------------
        /// @brief : Begin the value scope for a property. This moves the value column, and begins a disabled
        /// scope if needed. Must be followed by a call to internal::EndPropertyValue().
        /// This should be called within the scope of the overall property.
        //----------------------------------------------------------------------------------------------------
        void BeginPropertyValue(const bool isDisabled = false);

        //----------------------------------------------------------------------------------------------------
        /// @brief : End the current Property Value scope.
        //----------------------------------------------------------------------------------------------------
        void EndPropertyValue(const bool isDisabled = false);

        //----------------------------------------------------------------------------------------------------
        /// @brief : Creates a unique label for a property based on the string.
        //----------------------------------------------------------------------------------------------------
        std::string CreateHiddenPropertyValueLabel(const char* label);
    }

    //----------------------------------------------------------------------------------------------------
    /// @brief : Draws a header that spans the entire width.
    //----------------------------------------------------------------------------------------------------
    void Header(const std::string& name);

    //----------------------------------------------------------------------------------------------------
    /// @brief : Draws a header that spans the entire width, and can hide subsequent elements within it.
    //----------------------------------------------------------------------------------------------------
    bool CollapsableHeader(const std::string& name, const bool openByDefault = true);

    //----------------------------------------------------------------------------------------------------
    /// @brief : Begin a group of property calls. This sets up the table structure for subsequent properties.
    /// Must be followed by a call to EndPropertyGrid, regardless of the returned result.
    ///	@returns : Returns whether the property table is open.
    //----------------------------------------------------------------------------------------------------
    bool BeginPropertyTable();

    //----------------------------------------------------------------------------------------------------
    /// @brief : End a Property Table. This must be called for every call to BeginPropertyTable. 
    //----------------------------------------------------------------------------------------------------
    void EndPropertyTable();

    //----------------------------------------------------------------------------------------------------
    /// @brief : Render a float property that can be edited.
    ///	@param label : Name of the property.
    ///	@param value : Value to edit.
    ///	@param speed : Rate of change when dragging the float with the mouse.
    ///	@param min : Minimum value. If the min and max values are both equal to 0.f, then there will be no bounds.
    ///	@param max : Maximum value. If the min and max values are both equal to 0.f, then there will be no bounds.
    ///	@param format : Format of the float in the inspector. Default is "%.3f" which will make the precision up
    ///     to 3 decimal places. If you want to add a unit, just add the characters after the precision. For example:
    ///     "%.1f°" will render a value of 40.125 as "40.1°".
    ///	@param toolTip : Optional tooltip that will show up when hovering over the name of the property.
    ///	@returns : Returns true if the value has changed.
    //----------------------------------------------------------------------------------------------------
    bool Property(const char* label, float& value, const float speed = 0.1f, const float min = 0.f, const float max = 0.f, const char* format = "%.3f", const char* toolTip = "");

    //----------------------------------------------------------------------------------------------------
    /// @brief : Render a float property, but disable any edits.
    ///	@param label : Name of the property.
    ///	@param value : Value to show.
    ///	@param format : Format of the float in the inspector. Default is "%.3f" which will make the precision up
    ///     to 3 decimal places. If you want to add a unit, just add the characters after the precision. For example:
    ///     "%.1f°" will render a value of 40.125 as "40.1°".
    ///	@param toolTip : Optional tooltip that will show up when hovering over the name of the property.
    ///	@returns : Returns true if the value has changed.
    //----------------------------------------------------------------------------------------------------
    void Property(const char* label, const float& value, const char* format = "%.3f", const char* toolTip = "");

    //----------------------------------------------------------------------------------------------------
    /// @brief : Render a string property that can be edited.
    ///	@param label : Name of the property.
    ///	@param value : Value to edit.
    ///	@param toolTip : Optional tooltip that will show up when hovering over the name of the property.
    ///	@returns : Returns true if the value has changed.
    //----------------------------------------------------------------------------------------------------
    bool Property(const char* label, std::string& value, const char* toolTip = "");

    //----------------------------------------------------------------------------------------------------
    /// @brief : Render a string property, but disable any edits.
    ///	@param label : Name of the property.
    ///	@param value : Value to show.
    ///	@param toolTip : Optional tooltip that will show up when hovering over the name of the property.
    //----------------------------------------------------------------------------------------------------
    void Property(const char* label, const std::string& value, const char* toolTip = "");

    //----------------------------------------------------------------------------------------------------
    /// @brief : Render a uint64 property that can be edited.
    ///	@param label : Name of the property.
    ///	@param value : Value to edit.
    ///	@param toolTip : Optional tooltip that will show up when hovering over the name of the property.
    ///	@returns : Returns true if the value has changed.
    //----------------------------------------------------------------------------------------------------
    bool Property(const char* label, uint64& value, const char* toolTip = "");

    //----------------------------------------------------------------------------------------------------
    /// @brief : Render a uint64 property, but disable any edits.
    ///	@param label : Name of the property.
    ///	@param value : Value to show.
    ///	@param toolTip : Optional tooltip that will show up when hovering over the name of the property.
    //----------------------------------------------------------------------------------------------------
    void Property(const char* label, const uint64& value, const char* toolTip = "");

    //----------------------------------------------------------------------------------------------------
    /// @brief : Render a Vec3 property that can be edited.
    ///	@param label : Name of the property.
    ///	@param vec : Value to edit.
    ///	@param toolTip : Optional tooltip that will show up when hovering over the name of the property.
    ///	@returns : Returns true if the value has changed.
    //----------------------------------------------------------------------------------------------------
    bool Property(const char* label, Vec3& vec, const char* toolTip = "");

    //----------------------------------------------------------------------------------------------------
    /// @brief : Render a string property, but disable any edits.
    ///	@param label : Name of the property.
    ///	@param vec : Value to show.
    ///	@param toolTip : Optional tooltip that will show up when hovering over the name of the property.
    //----------------------------------------------------------------------------------------------------
    void Property(const char* label, const Vec3& vec, const char* toolTip = "");

    //----------------------------------------------------------------------------------------------------
    /// @brief : Render a Rotation property that can be edited.
    ///	@param label : Name of the property.
    ///	@param rotation : Value to edit.
    ///	@param toolTip : Optional tooltip that will show up when hovering over the name of the property.
    ///	@returns : Returns true if the value has changed.
    //----------------------------------------------------------------------------------------------------
    bool Property(const char* label, Rotation& rotation, const char* toolTip = "");

    //----------------------------------------------------------------------------------------------------
    /// @brief : Render a Rotation property, but disable any edits.
    ///	@param label : Name of the property.
    ///	@param rotation : Value to show.
    ///	@param toolTip : Optional tooltip that will show up when hovering over the name of the property.
    //----------------------------------------------------------------------------------------------------
    void Property(const char* label, const Rotation& rotation, const char* toolTip = "");

    bool PropertyColor(const char* label, LinearColor& color, const bool includeAlpha = true, const char* toolTip = "");
    void PropertyColor(const char* label, const LinearColor& color, const bool includeAlpha = true, const char* toolTip = "");

    bool PropertyColor(const char* label, Color& color, const bool includeAlpha = true, const char* toolTip = "");
    void PropertyColor(const char* label, const Color& color, const bool includeAlpha = true, const char* toolTip = "");

    bool PropertyColor(const char* label, Vec3& color, const char* toolTip = "");
    

    //----------------------------------------------------------------------------------------------------
    /// @brief : Renders the name of the Entity for the given ID, and a dropdown to select other Entities in
    ///     the scene.
    ///	@param label : Name of the property.
    ///	@param entity : EntityID value that can be edited.
    /// @param registry : The entity registry that this the entity is a part of. Used to populate a dropdown
    /// of selectable entities.
    ///	@param toolTip : Optional tooltip that will show up when hovering over the name of the property.
    ///	@returns : Returns true if the value has changed.
    //----------------------------------------------------------------------------------------------------
    bool PropertyEntityID(const char* label, EntityID& entity, EntityRegistry& registry, const char* toolTip = "");

    //---------------------------------------------------------------------------------------------------
    /// @brief : Renders the name of the Entity for the given ID, but does not allow any changes to be made.
    ///	@param label : Name of the property.
    ///	@param entity : EntityID value to be shown.
    ///	@param registry : The entity registry that this the entity is a part of.
    ///	@param toolTip : Optional tooltip that will show up when hovering over the name of the property.
    //----------------------------------------------------------------------------------------------------
    void PropertyEntityID(const char* label, const EntityID& entity, EntityRegistry& registry, const char* toolTip = "");
    
    template <EnumType Type>
    struct EnumPropertyValueDesc
    {
        Type m_value{};
        const char* m_label = "";
        const char* m_toolTip = "";
    };

    template <EnumType Type>
    bool PropertyEnum(const char* label, Type& value, const std::vector<EnumPropertyValueDesc<Type>>& options, const char* toolTip = "")
    {
        bool modified = false;

        internal::BeginProperty(label, toolTip);
        internal::BeginPropertyValue();

        // Find the current item name:
        const char* currentItemName = "";
        for (const auto& option : options)
        {
            if (option.m_value == value)
            {
                currentItemName = option.m_label;
                break;
            }
        }
        
        // Create combo dropdown
        if (ImGui::BeginCombo(internal::CreateHiddenPropertyValueLabel(label).c_str(), currentItemName))
        {
            for (const auto& option : options)
            {
                const bool isSelected = option.m_value == value;

                if (ImGui::Selectable(option.m_label, isSelected))
                {
                    value = option.m_value;
                    modified = true;
                }

                SetToolTip(option.m_toolTip);

                // Set initial focus on selected item
                if (isSelected)
                    ImGui::SetItemDefaultFocus();
            }
            ImGui::EndCombo();
        }

        internal::EndPropertyValue();
        internal::EndProperty();

        return modified;
    }
    
    //----------------------------------------------------------------------------------------------------
    // [TODO]: I am currently just rendering child properties as elements in the table, but I would prefer just
    //  a tree node with a "ListBox".
    // [TODO]: I am only supporting uint64 values right now. I obviously need to support more.
    //		
    /// @brief : [Under Development] Renders an array of values that cannot be edited.
    ///	@tparam Type : Element type of the array. 
    ///	@param label : Name of the vector property.
    ///	@param array : Array that will be shown.
    ///	@param selected : The passed in value will be the current selected element in the array. It will be
    /// updated with a current selection on return.
    ///	@param toolTip : Optional tooltip that will show up when hovering over the name of the property.
    //----------------------------------------------------------------------------------------------------
    template <typename Type>
    void PropertyArray(const char* label, const std::vector<Type>& array, size_t& selected, const char* toolTip = "")
    {
        if (selected > array.size())
            selected = 0;

        const bool hasElements = !array.empty();
        
        const bool isOpen = internal::BeginCollapsableProperty(label, toolTip);
        
        // Render the array size as the value:
        internal::BeginPropertyValue();
        ImGui::Text("Array Size: %zu", array.size());
        internal::EndPropertyValue();

        // Render the array elements.
        std::string name;
        if (isOpen && hasElements)
        {
            // Push indent
            ImGui::Indent();

            for (size_t i = 0; i < array.size(); ++i)
            {
                ImGui::PushID(static_cast<int>(i));
                name = std::format("Index[{}]", i);

                // [TODO]: I need to support other types.
                if constexpr (std::is_same_v<Type, uint64>)
                {
                    Property(name.c_str(), array[i]);
                }
                
                ImGui::PopID();
            }
            
            ImGui::Unindent();
        }
        
        internal::EndCollapsableProperty(isOpen);
    }

    //----------------------------------------------------------------------------------------------------
    /// @brief : Render an Asset's name for the given ID and Type. It the value will have a dropdown that will
    /// show all other loaded assets that share the same type.
    ///	@tparam Type : Type of Asset that ID is for.
    ///	@param label : Name of the property.
    ///	@param assetID : ID of the Asset that will be edited.
    ///	@param toolTip : Optional tooltip that will show up when hovering over the name of the property.
    ///	@returns : Returns true if the value has changed.
    //----------------------------------------------------------------------------------------------------
    template <ValidAssetType Type>
    bool PropertyAssetID(const char* label, AssetID& assetID, const char* toolTip = "")
    {
        bool modified = false;
        internal::BeginProperty(label, toolTip);

        internal::BeginPropertyValue();

        // Get current asset and its name
        auto pAsset = AssetManager::GetAsset<Type>(assetID);
        const std::string currentName = pAsset != nullptr ? pAsset.GetMetadata().m_assetName : "None";
    
        // Begin combo box
        if (ImGui::BeginCombo("##AssetCombo", currentName.c_str()))
        {
            // [TODO]: Search bar using the ImGui Filter:
            
            // Add "None" option to clear the asset
            if (ImGui::Selectable("None", assetID == kInvalidAssetID))
            {
                assetID = kInvalidAssetID;
                modified = true;
            }
        
            // Get all assets of this type
            std::vector<AssetPtr<Type>> assets = AssetManager::GetAllAssetsOfType<Type>();
            
            for (const auto& asset : assets)
            {
                if (asset == nullptr)
                    continue;

                const AssetMetadata& metadata = asset.GetMetadata(); 
                
                bool isSelected = (assetID == metadata.m_assetID);
            
                if (ImGui::Selectable(metadata.m_assetName.c_str(), isSelected))
                {
                    assetID = metadata.m_assetID;
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

        // [TODO]: Drag and drop assets onto the field:
        
        internal::EndPropertyValue();
        internal::EndProperty();

        return modified;
    }

    template <ValidAssetType Type>
    bool PropertyAssetIDArray(const char* label, std::vector<AssetID>& assetIDs, const char* toolTip = "")
    {
        bool modified = false;

        const bool hasElements = !assetIDs.empty();
        const bool isOpen = internal::BeginCollapsableProperty(label, toolTip);

        // Render the array size as the value:
        internal::BeginPropertyValue();
        ImGui::Text("Array Size: %zu", assetIDs.size());
        internal::EndPropertyValue();

        // Render the array elements.
        std::string name;
        if (isOpen && hasElements)
        {
            // Push indent
            ImGui::Indent();

            for (size_t i = 0; i < assetIDs.size(); ++i)
            {
                ImGui::PushID(static_cast<int>(i));
                name = std::format("Index[{}]", i);
                modified |= PropertyAssetID<Type>(name.c_str(), assetIDs[i]);
                ImGui::PopID();
            }
            
            ImGui::Unindent();
        }
        
        internal::EndCollapsableProperty(isOpen);

        return modified;
    }
    
}
