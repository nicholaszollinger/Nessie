// EditorInspector.cpp
#include "EditorInspector.h"

namespace nes
{
    std::string EditorInspectorBase::GetTargetShortTypename() const
    {
        std::string fullName(GetTargetTypename());
        auto firstLetter = fullName.find_last_of(' ');
        if (firstLetter == std::string::npos)
            firstLetter = 0;
        else
            firstLetter++;
        
        return StripNamespaceFromTypename(fullName.substr(firstLetter));
    }

    bool EditorInspectorRegistry::HasInspector(const entt::id_type typeID)
    {
        return s_inspectors.contains(typeID);
    }

    std::shared_ptr<EditorInspectorBase> EditorInspectorRegistry::GetInspector(const entt::id_type typeID)
    {
        if (HasInspector(typeID))
            return s_inspectors.at(typeID);

        return nullptr;
    }
}
