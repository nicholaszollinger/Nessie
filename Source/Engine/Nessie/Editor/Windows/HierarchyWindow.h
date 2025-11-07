// HierarchyWindow.h
#pragma once
#include "Nessie/Core/Memory/StrongPtr.h"
#include "Nessie/Editor/EditorWindow.h"
#include "Nessie/World.h"

namespace nes
{
    class WorldBase;
    struct NodeComponent;
    
    class HierarchyWindow final : public EditorWindow
    {
        NES_DEFINE_TYPE_INFO(HierarchyWindow)
        
    public:
        HierarchyWindow();

        void            SetWorld(const StrongPtr<WorldBase>& pWorld) { m_pWorld = pWorld; }
        virtual void    RenderImGui() override;

    private:
        void            DrawEntityNode(EntityRegistry& registry, Entity& entity);
        bool            NameSearchRecursive(EntityRegistry& registry, Entity& entity, const uint32 maxSearchDepth, const uint32 currentDepth = 0);
    private:
        StrongPtr<WorldBase> m_pWorld = nullptr;
        bool                 m_isFocused = false;
        ImGuiTextFilter      m_filter;
    };
}
