// EntityInspector.h
#pragma once
#include "Nessie/Editor/EditorInspector.h"
#include "Nessie/World.h"

namespace nes
{
    //----------------------------------------------------------------------------------------------------
    /// @brief : Draws an Entity's component information.
    /// @note : For a component to show up in the Entity inspector, it must have an EditorInspector registered
    /// for it, and the current inspector level must match.
    //----------------------------------------------------------------------------------------------------
    class EntityInspector final : public EditorInspector<EntityHandle>
    {
    public:
        EntityInspector();
        
    private:
        virtual void    DrawImpl(EntityHandle* pTarget, const InspectorContext& context) override;

        //----------------------------------------------------------------------------------------------------
        /// @brief : Draws a list of all components that are attached to the entity, as well as the button to
        ///     add components to the entity.
        //----------------------------------------------------------------------------------------------------
        void            DrawComponentList(EntityRegistry& registry, EntityHandle entity);

        //----------------------------------------------------------------------------------------------------
        /// @brief : Uses the Component Inspector to draw the current selected component's information.
        //----------------------------------------------------------------------------------------------------
        void            DrawSelectedComponentDetails(EntityRegistry& registry, EntityHandle entity, const InspectorContext& context);

        //----------------------------------------------------------------------------------------------------
        /// @brief : Creates the array of possible ComponentInspectors that can be used.
        //----------------------------------------------------------------------------------------------------
        void            AssembleComponentInspectors();

        //----------------------------------------------------------------------------------------------------
        /// @brief : Draws a popup that contains a search field and a list of Components that can be added to the
        ///  entity. Selecting an option adds it to the entity. 
        //----------------------------------------------------------------------------------------------------
        void            DrawAddComponentDropdown(EntityRegistry& registry, EntityHandle entity);

    private:
        std::vector<std::shared_ptr<EditorInspectorBase>> m_componentInspectors{};
        ImGuiTextFilter m_searchFilter{};
        size_t          m_selectedComponentType = 0;
        EntityHandle    m_lastSelected = kInvalidEntityHandle;
        EInspectorLevel m_inspectorShowFlags = EInspectorLevel::None;
    };
}