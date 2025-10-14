// NodeComponent.h
#pragma once
#include "Nessie/World/Component.h"
#include "Nessie/World/Entity.h"

namespace nes
{
    //----------------------------------------------------------------------------------------------------
    /// @brief : The Node Component contains ids for the "single parent to multiple children" entity hierarchy. 
    //----------------------------------------------------------------------------------------------------
    struct NodeComponent
    {
        EntityID                m_parentID = nes::kInvalidEntityID;
        std::vector<EntityID>   m_childrenIDs{};

        static void             Serialize(YAML::Emitter& out, const NodeComponent& component);
        static void             Deserialize(const YAML::Node& in, NodeComponent& component);
    };
}