// ComponentRegistry.cpp
#include "Nessie/World.h"

namespace nes
{
    std::vector<ComponentTypeDesc> ComponentRegistry::GetAllComponentTypes() const
    {
        std::vector<ComponentTypeDesc> componentTypes;
        componentTypes.reserve(m_componentTypes.size());

        for (auto& [_, desc] : m_componentTypes)
        {
            componentTypes.emplace_back(desc);
        }

        return componentTypes;
    }
}
