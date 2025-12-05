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
        /// @brief : Draws a popup window for adding global or world entities.
        //----------------------------------------------------------------------------------------------------
        void            DrawAddEntityDropdown(EntityRegistry& registry);
        
        //----------------------------------------------------------------------------------------------------
        /// @brief : Recursively draws the Entity Node hierarchy. 
        //----------------------------------------------------------------------------------------------------
        void            DrawWorldEntityNode(EntityRegistry& registry, Entity& entity);

        //----------------------------------------------------------------------------------------------------
        /// @brief : Draws an Entity Node without a transform. It cannot be parented.
        //----------------------------------------------------------------------------------------------------
        void            DrawGlobalEntityNode(EntityRegistry& registry, Entity& entity);

        //----------------------------------------------------------------------------------------------------
        /// @brief : Draws the current Entity Node during a rename operation.
        //----------------------------------------------------------------------------------------------------
        void            DrawEntityNodeRename(IDComponent& idComp, bool& nodeOpen, const int treeNodeFlags);

        //----------------------------------------------------------------------------------------------------
        /// @brief : Draw the drag target highlights for an entity with a transform.
        //----------------------------------------------------------------------------------------------------
        void            DrawDragTargetForWorldEntity(EntityRegistry& registry, Entity& entity, const ImVec2 treeNodeMin, const ImVec2 treeNodeMax, const bool removeInsertBelow);

        //----------------------------------------------------------------------------------------------------
        /// @brief : Draw the drag target highlights for a root entity.
        //----------------------------------------------------------------------------------------------------
        void            DrawDragTargetForRootEntity(EntityRegistry& registry, Entity& entity, const ImVec2 treeNodeMin, const ImVec2 treeNodeMax, const bool removeInsertBelow);

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
        void            CreateNewGlobalEntity(EntityRegistry& registry);

        //----------------------------------------------------------------------------------------------------
        /// @brief : Create a new entity. If the parent is invalid, this will be a new root entity.
        //----------------------------------------------------------------------------------------------------
        void            CreateNewWorldEntity(EntityRegistry& registry, const EntityID parent = kInvalidEntityID);

        //----------------------------------------------------------------------------------------------------
        /// @brief : Deletes an entity and all of its children, if it has any.
        /// This will check if the entity has a NodeComponent or not, so it can be used on Global Entities as well.
        //----------------------------------------------------------------------------------------------------
        void            DeleteEntityAndChildren(EntityRegistry& registry, const EntityID& entityID) const;

        //----------------------------------------------------------------------------------------------------
        /// @brief : Draw the set of options when selecting a single entity.
        //----------------------------------------------------------------------------------------------------
        void            DrawSingleSelectedEntityMenuOptions(EntityRegistry& registry, const IDComponent& idComp, const NodeComponent* pNodeComp);

        //----------------------------------------------------------------------------------------------------
        /// @brief : Draw a set of options when selecting multiple entities within the hierarchy.
        //----------------------------------------------------------------------------------------------------
        void            DrawMultSelectedEntityMenuOptions(EntityRegistry& registry, const std::vector<uint64>& selected) const;
    
    private:
        static constexpr size_t kInputBufferSize = 256;
        
        ImGuiTextFilter         m_filter;
        EntityID                m_currentRenameEntity = kInvalidEntityID;
        EntityID                m_forceOpenEntity = kInvalidEntityID;
        uint64                  m_newEntityCounter = 0;
        char                    m_inputBuffer[256] = {};
        bool                    m_isFocused = false;
        bool                    m_isDraggingEntity = false;
        bool                    m_shouldFocusRename = false;
        bool                    m_selectionContainsGlobalEntity = false;
    };
}
