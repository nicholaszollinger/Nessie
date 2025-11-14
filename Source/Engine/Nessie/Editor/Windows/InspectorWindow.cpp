// InspectorWindow.cpp
#include "InspectorWindow.h"

#include "Nessie/Editor/SelectionManager.h"
#include "Nessie/Editor/Inspectors/EntityInspector.h"

namespace nes
{
    InspectorWindow::InspectorWindow()
    {
        m_desc.m_name = "Inspector";

        EditorInspectorRegistry::RegisterInspector<EntityInspector>();
    }

    void InspectorWindow::RenderImGui()
    {
        auto& registry = m_pWorld->GetRegistry();
        
        if (ImGui::Begin(m_desc.m_name.c_str(), &m_desc.m_isOpen, 0))
        {
            // [TODO]: Render the Lock Button to lock the window to the current selected item.
            // [TODO]: Check if we are locked to a selection and just render that.
            
            // Get the current selection:
            // [TODO]: Handle different contexts, i.e. Asset information as well.
            //  - For now, I am just going to assume that entities are always being selected.
            InspectorContext context;
            context.m_pWorld = m_pWorld;
            context.m_selectionIDs = editor::SelectionManager::GetSelections(editor::SelectionManager::kGlobalContext);
            if (context.m_selectionIDs.empty() || context.m_selectionIDs.size() > 1)
            {
                // Don't render anything on multiple selections.
                ImGui::End();
                return;
            }
            
            // [FOR NOW]: I am assuming that these are all always entity ids.
            // - They could also be AssetIDs. I probably need to unify these in some way.
            // [TODO]: I should render the last selected. Otherwise, if you misclick on the hierarchy window, it will
            // undo the inspection that you had previously.
            const EntityID entityID = context.m_selectionIDs.front();
            if (registry.IsValidEntity(entityID))
            {
                if (auto pEntityInspector = EditorInspectorRegistry::GetInspector<EntityHandle>())
                {
                    EntityHandle handle = registry.GetEntity(entityID);
                    pEntityInspector->Draw(&handle, context);
                }
            }
        }
        ImGui::End();
    }
}
