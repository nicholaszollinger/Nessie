// SelectionManager.h
#pragma once
#include <unordered_map>
#include "Nessie/Core/TypeInfo.h"

namespace nes::editor
{
    //----------------------------------------------------------------------------------------------------
    // [Note] : The selections are stored in an array because of ImGui specific reasons. Otherwise, I would have
    // them in an unordered_set.
    //
    /// @brief : The selection manager manages the selected object state in the Editor.
    /// - Selection IDs are any unique uint64_t value. You can use EntityID, AssetID, etc.
    /// - Context IDs are any unique uint64_t value. For example, you can use a TypeID of a Window to manage the selection
    ///   state across all Window instances of that type. Or, you can pass the Window Instance pointer cast to a uint64_t
    ///   to manage the selections of a single instance.
    //----------------------------------------------------------------------------------------------------
    class SelectionManager
    {
    public:
        // Global context ID.
        static constexpr uint64 kGlobalContext = std::numeric_limits<uint64>::max();
        
        //----------------------------------------------------------------------------------------------------
        /// @brief : Check if the given selectionID is selected for a given context.
        //----------------------------------------------------------------------------------------------------
        static bool IsSelected(const uint64 contextID, const uint64 selectionID);

        //----------------------------------------------------------------------------------------------------
        /// @brief : Checks if the given selectionID is selected in the global context.
        //----------------------------------------------------------------------------------------------------
        static bool IsSelectedGlobal(const uint64 selectionID);

        //----------------------------------------------------------------------------------------------------
        /// @brief : Add a selection to a given context.
        //----------------------------------------------------------------------------------------------------
        static void Select(const uint64 contextID, const uint64 selectionID);

        //----------------------------------------------------------------------------------------------------
        /// @brief : Adds the selection, and clears all other selections for this context.
        //----------------------------------------------------------------------------------------------------
        static void SelectUnique(const uint64 contextID, const uint64 selectionID);

        //----------------------------------------------------------------------------------------------------
        /// @brief : Adds the selection to the global context, and clears other selections.
        //----------------------------------------------------------------------------------------------------
        static void SelectGlobalUnique(const uint64 selectionID);

        //----------------------------------------------------------------------------------------------------
        /// @brief : Deselect from the given context.
        //----------------------------------------------------------------------------------------------------
        static void Deselect(const uint64 contextID, const uint64 selectionID);

        //----------------------------------------------------------------------------------------------------
        /// @brief : Remove the selection from the global context. 
        //----------------------------------------------------------------------------------------------------
        static void DeselectGlobal(const uint64 selectionID);

        //----------------------------------------------------------------------------------------------------
        /// @brief : Remove the selection from all contexts.
        //----------------------------------------------------------------------------------------------------
        static void DeselectFromAll(const uint64 selectionID);

        //----------------------------------------------------------------------------------------------------
        /// @brief : Remove all selections from the given context.
        //----------------------------------------------------------------------------------------------------
        static void DeselectAll(const uint64 contextID);

        //----------------------------------------------------------------------------------------------------
        /// @brief : Remove all selections from all contexts. 
        //----------------------------------------------------------------------------------------------------
        static void DeselectAll();

        //----------------------------------------------------------------------------------------------------
        /// @brief : Get all the selections for a given context.
        //----------------------------------------------------------------------------------------------------
        static const std::vector<uint64>& GetSelections(const uint64 contextID);

    private:
        //----------------------------------------------------------------------------------------------------
        /// @brief : Returns the selections for a given context.
        /// @note : If the selection set has not been created yet, it reserves values for it.  
        //----------------------------------------------------------------------------------------------------
        static std::vector<uint64>& GetOrInitSelections(const uint64 contextID);

        static void RemoveIfFound(std::vector<uint64>& selections, const uint64 selectionID);
        static void AddIfNotFound(std::vector<uint64>& selections, const uint64 selectionID);
        
        inline static std::unordered_map<uint64, std::vector<uint64>> s_selectionStates{};
    };
}
