// NodeComponent.cpp
#include "NodeComponent.h"

namespace nes
{
    void NodeComponent::Serialize(YamlOutStream& out, const NodeComponent& component)
    {
        out.Write("Parent", component.m_parentID);
        out.Write("Children", component.m_childrenIDs);
    }

    void NodeComponent::Deserialize(const YamlNode& in, NodeComponent& component)
    {
        in["Parent"].Read(component.m_parentID, kInvalidEntityID);
        in["Children"].Read(component.m_childrenIDs, std::vector<uint64>{});
    }
}