// PropertyGrid.h
#pragma once
#include "EditorCore.h"
#include "Nessie/Asset/AssetBase.h"
#include "Nessie/Asset/AssetManager.h"
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
    }

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
    ///	@param value : Value to edit.
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
    ///	@param value : Value to edit.
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
    ///	@param vec : Value to edit.
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
    ///	@param rotation : Value to edit.
    ///	@param toolTip : Optional tooltip that will show up when hovering over the name of the property.
    //----------------------------------------------------------------------------------------------------
    void Property(const char* label, const Rotation& rotation, const char* toolTip = "");

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
        const std::string currentName = pAsset != nullptr ? pAsset.GetMetadata().m_path.filename().string() : "None";
    
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
            std::string assetName;
            std::vector<AssetPtr<Type>> assets = AssetManager::GetAllAssetsOfType<Type>();
            
            for (const auto& asset : assets)
            {
                if (asset == nullptr)
                    continue;

                const AssetMetadata& metadata = asset.GetMetadata(); 
            
                assetName = metadata.m_path.filename().string();
                bool isSelected = (assetID == metadata.m_assetID);
            
                if (ImGui::Selectable(assetName.c_str(), isSelected))
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
    
}
