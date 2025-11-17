// ComponentInspector.h
#pragma once
#include "Nessie/Editor/EditorInspector.h"
#include "Nessie/World.h"

//----------------------------------------------------------------------------------------------------
/// @brief : Helper macro to define a basic internal inspector that has no draw capabilities. Used for
/// tag classes with no internal data. By defining this Inspector class, it can still be shown in
/// debugging situations.
///
/// The name of the class is 'Type+Inspector', so for NES_DEFINE_INTERNAL_COMPONENT_INSPECTOR(DisabledComponent) will
/// define a class called 'DisabledComponentInspector'.
//----------------------------------------------------------------------------------------------------
#define NES_DEFINE_INTERNAL_COMPONENT_INSPECTOR(type) \
    class type##Inspector : public EditorInspector<type> \
    { \
        public: \
        type##Inspector() : EditorInspector(EInspectorLevel::Internal) {} \
    }

namespace nes
{
    NES_DEFINE_INTERNAL_COMPONENT_INSPECTOR(DisabledComponent);
    NES_DEFINE_INTERNAL_COMPONENT_INSPECTOR(PendingInitialization);
    NES_DEFINE_INTERNAL_COMPONENT_INSPECTOR(PendingDestruction);
    NES_DEFINE_INTERNAL_COMPONENT_INSPECTOR(PendingEnable);
    NES_DEFINE_INTERNAL_COMPONENT_INSPECTOR(PendingDisable);
    
    class IDComponentInspector final : public EditorInspector<IDComponent>
    {
    public:
        IDComponentInspector() : EditorInspector(EInspectorLevel::Internal) {}

    private:
        virtual void    DrawImpl(TargetType* pComponent, const InspectorContext& context) override;
    };

    class NodeComponentInspector final : public EditorInspector<NodeComponent>
    {
    public:
        NodeComponentInspector() : EditorInspector(EInspectorLevel::Internal) {}

    private:
        virtual void    DrawImpl(TargetType* pComponent, const InspectorContext& context) override;

    private:
        size_t          m_currentSelectedChild = std::numeric_limits<size_t>::max();
    };
}