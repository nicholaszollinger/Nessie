// SelectionManager.cpp
#include "SelectionManager.h"

namespace nes::editor
{
    bool SelectionManager::IsSelected(const uint64 contextID, const uint64 selectionID)
    {
        const auto& selections = GetOrInitSelections(contextID);
        return std::ranges::find(selections.begin(), selections.end(), selectionID) != selections.end();
    }

    bool SelectionManager::IsSelectedGlobal(const uint64 selectionID)
    {
        return IsSelected(kGlobalContext, selectionID);
    }

    void SelectionManager::Select(const uint64 contextID, const uint64 selectionID)
    {
        auto& selections = GetOrInitSelections(contextID);
        AddIfNotFound(selections, selectionID);
    }

    void SelectionManager::SelectUnique(const uint64 contextID, const uint64 selectionID)
    {
        auto& selections = GetOrInitSelections(contextID);
        selections.clear();
        selections.emplace_back(selectionID);
    }

    void SelectionManager::SelectGlobalUnique(const uint64 selectionID)
    {
        SelectUnique(kGlobalContext, selectionID);
    }

    void SelectionManager::Deselect(const uint64 contextID, const uint64 selectionID)
    {
        auto& selections = GetOrInitSelections(contextID);
        RemoveIfFound(selections, selectionID);
    }

    void SelectionManager::DeselectGlobal(const uint64 selectionID)
    {
        Deselect(kGlobalContext, selectionID);
    }

    void SelectionManager::DeselectFromAll(const uint64 selectionID)
    {
        for (auto& [_, selections] : s_selectionStates)
        {
            RemoveIfFound(selections, selectionID);
        }
    }

    void SelectionManager::DeselectAll(const uint64 contextID)
    {
        auto& selections = GetOrInitSelections(contextID);
        selections.clear();
    }

    void SelectionManager::DeselectAll()
    {
        for (auto& [_, selections] : s_selectionStates)
        {
            selections.clear();
        }
    }

    const std::vector<uint64>& SelectionManager::GetSelections(const uint64 contextID)
    {
        return GetOrInitSelections(contextID);
    }

    std::vector<uint64>& SelectionManager::GetOrInitSelections(const uint64 contextID)
    {
        static constexpr size_t kReserveSize = 16;
        if (!s_selectionStates.contains(contextID))
        {
            auto& selections = s_selectionStates[contextID];
            selections.reserve(kReserveSize);
        }

        return s_selectionStates[contextID];
    }

    void SelectionManager::RemoveIfFound(std::vector<uint64>& selections, const uint64 selectionID)
    {
        auto it = std::ranges::find(selections, selectionID);
        if (it != selections.end())
        {
            selections.erase(it);
        }
    }

    void SelectionManager::AddIfNotFound(std::vector<uint64>& selections, const uint64 selectionID)
    {
        auto it = std::ranges::find(selections, selectionID);
        if (it == selections.end())
        {
            selections.emplace_back(selectionID);
        }
    }
}
