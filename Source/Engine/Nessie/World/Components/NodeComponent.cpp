// NodeComponent.cpp
#include "NodeComponent.h"

namespace nes
{
    void NodeComponent::Serialize(YAML::Emitter&, const NodeComponent&)
    {
        // [TODO]: 
    }

    void NodeComponent::Deserialize(const YAML::Node& in, NodeComponent& component)
    {
        component.m_parentID = in["Parent"].as<uint64>(kInvalidEntityID);
        component.m_childrenIDs = in["Children"].as<std::vector<uint64>>(std::vector<uint64>{});
    }
}