// HierarchyWindow.h
#pragma once
#include "Nessie/Core/Memory/StrongPtr.h"
#include "Nessie/Editor/EditorWindow.h"
#include "Nessie/World.h"

namespace nes
{
    struct NodeComponent;
    
    // [TODO]: Move to a better location:
    // This is a tag for accepting payloads from the hierarchy window.
    static constexpr const char* kEntityHierarchyDropPayloadName = "entityHierarchy";
    
    class HierarchyWindow final : public EditorWindow
    {
        NES_DEFINE_TYPE_INFO(HierarchyWindow)
        
    public:
        HierarchyWindow();
        virtual void    RenderImGui() override;

    private:
        //----------------------------------------------------------------------------------------------------
        /// @brief : Recursively draws the Entity Node hierarchy. 
        //----------------------------------------------------------------------------------------------------
        void            DrawEntityNode(EntityRegistry& registry, Entity& entity);

        //----------------------------------------------------------------------------------------------------
        /// @brief : Draw the drag target highlights for an entity node.
        //----------------------------------------------------------------------------------------------------
        void            DrawDragTargetForEntityNode(EntityRegistry& registry, Entity& entity, const ImVec2 treeNodeMin, const ImVec2 treeNodeMax, const bool removeInsertBelow);

        //----------------------------------------------------------------------------------------------------
        /// @brief : Draw a right-click context menu when *not* clicking on an Entity node in the hierarchy.
        //----------------------------------------------------------------------------------------------------
        void            DrawGlobalContextMenu(EntityRegistry& registry);

        //----------------------------------------------------------------------------------------------------
        /// @brief : Draw a right-click context menu when clicking on an entity node.
        //----------------------------------------------------------------------------------------------------
        void            DrawEntityContextMenu(EntityRegistry& registry, Entity& entity);

        //----------------------------------------------------------------------------------------------------
        /// @brief : Recursively search the entity and all children to check if the current text filter matches the
        /// entity name.
        //----------------------------------------------------------------------------------------------------
        bool            NameSearchRecursive(EntityRegistry& registry, Entity& entity, const uint32 maxSearchDepth, const uint32 currentDepth = 0);

        //----------------------------------------------------------------------------------------------------
        /// @brief : Create a new entity. If the parent is invalid, this will be a new root entity.
        //----------------------------------------------------------------------------------------------------
        void            CreateNewEntity(EntityRegistry& registry, const EntityID parent = kInvalidEntityID);

        //----------------------------------------------------------------------------------------------------
        /// @brief : Deletes an entity and all of its children. The default behavior when deleting entities in
        /// the hierarchy.
        //----------------------------------------------------------------------------------------------------
        void            DeleteEntityAndChildren(EntityRegistry& registry, const EntityID& entityID) const;

        //----------------------------------------------------------------------------------------------------
        /// @brief : Draw the set of options when selecting a single entity.
        //----------------------------------------------------------------------------------------------------
        void            DrawSingleSelectedEntityMenuOptions(EntityRegistry& registry, const IDComponent& idComp, const NodeComponent& nodeComp);

        //----------------------------------------------------------------------------------------------------
        /// @brief : Draw a set of options when selecting multiple entites within the hierarchy.
        //----------------------------------------------------------------------------------------------------
        void            DrawMultSelectedEntityMenuOptions(EntityRegistry& registry, const std::vector<uint64>& selected) const;
        
    private:
        static constexpr size_t kInputBufferSize = 256;
        
        ImGuiTextFilter         m_filter;
        EntityID                m_currentRenameEntity = kInvalidEntityID;
        EntityID                m_forceOpenEntity = kInvalidEntityID;
        char                    m_inputBuffer[256] = {};
        bool                    m_isFocused = false;
        bool                    m_isDraggingEntity = false;
        bool                    m_shouldFocusRename = false;
    };
}
