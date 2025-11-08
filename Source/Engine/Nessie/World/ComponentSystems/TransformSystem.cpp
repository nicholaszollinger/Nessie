// TransformSystem.cpp
#include "TransformSystem.h"
#include "Nessie/World.h"
#include "Nessie/FileIO/YAML/Serializers/YamlMathSerializers.h"

namespace nes
{
    void TransformComponent::Serialize(YamlOutStream& out, const TransformComponent& component)
    {
        out.Write("Position", component.m_localPosition);
        out.Write("Rotation", component.m_localRotation);
        out.Write("Scale", component.m_localScale);
    }

    void TransformComponent::Deserialize(const YamlNode& in, TransformComponent& component)
    {
        in["Position"].Read(component.m_localPosition, Vec3::Zero());
        in["Rotation"].Read(component.m_localRotation, Rotation::Zero());
        in["Scale"].Read(component.m_localScale, Vec3::Zero());
        
        component.m_localMatrix = Mat44::ComposeTransform(component.m_localPosition, component.m_localRotation, component.m_localScale);
        component.m_worldMatrix = component.m_localMatrix;
        component.m_worldRotation = component.m_localRotation;
        component.m_isDirty = true;
    }

    void TransformSystem::RegisterComponentTypes()
    {
        NES_REGISTER_COMPONENT(IDComponent);
        NES_REGISTER_COMPONENT(TransformComponent);
        NES_REGISTER_COMPONENT(NodeComponent);
        NES_REGISTER_COMPONENT(PendingInitialization);
        NES_REGISTER_COMPONENT(PendingDestruction);
    }

    void TransformSystem::ProcessNewEntities()
    {
        auto& registry = GetRegistry();
        auto view = registry.GetAllEntitiesWith<TransformComponent, PendingInitialization>();
        
        for (auto entity : view)
        {
            m_needsRebuild = true;

            // Ensure that this entity has a NodeComponent.
            if (!registry.HasComponent<NodeComponent>(entity))
            {
                registry.AddComponent<NodeComponent>(entity);
            }
        }
    }

    void TransformSystem::ProcessDestroyedEntities(const bool clearingRegistry)
    {
        if (clearingRegistry)
        {
            m_depthOrderedEntities.clear();
            m_needsRebuild = false;
            return;
        }
        
        auto& registry = GetRegistry();
        auto view = registry.GetAllEntitiesWith<TransformComponent, PendingDestruction>();

        for (auto entity : view)
        {
            auto& node = registry.GetComponent<NodeComponent>(entity);

            const auto parentID = node.m_parentID;
            
            // If the parent is valid, we need to remove it from the tree.
            // We are orphaning all children.
            if (node.m_parentID != kInvalidEntityID)
            {
                const auto& idComp = registry.GetComponent<IDComponent>(entity);
                RemoveParent(idComp.GetID()); // This will set the 'm_needsRebuild' flag.
            }

            // Re-parent to the deleted entity's parent.
            for (auto childID : node.m_childrenIDs)
            {
                auto childHandle = registry.GetEntity(childID);
                
                // If the child is also being destroyed, skip.
                if (view.contains(childHandle))
                    continue;
                
                SetParent(childID, parentID);
            }
        }
    }

    void TransformSystem::UpdateHierarchy()
    {
        if (m_needsRebuild)
            RebuildHierarchyCache();

        // [TODO]: Consider threading.
        for (const auto& [entity, _] : m_depthOrderedEntities)
        {
            UpdateSingleTransform(entity);
        }
    }

    void TransformSystem::RebuildHierarchyCache()
    {
        m_depthOrderedEntities.clear();
        auto& registry = GetRegistry();
        
        auto view = registry.GetAllEntitiesWith<TransformComponent>();
        for (auto entity : view)
        {
            const auto& node = registry.GetComponent<NodeComponent>(entity);

            // If the parent is invalid, then it is a root - we can now compute the depth
            // for all child transforms.
            if (node.m_parentID == kInvalidEntityID)
            {
                ComputeDepthRecursively(entity, 0);
            }
        }

        // Build the depth ordered array for better traversal.
        m_depthOrderedEntities.reserve(view.size());

        for (auto entity : view)
        {
            const auto& transform = view.get<TransformComponent>(entity);
            m_depthOrderedEntities.emplace_back(entity, transform.m_hierarchyDepth);
        }

        // Sort by depth (parents before children).
        std::sort(m_depthOrderedEntities.begin(), m_depthOrderedEntities.end(), [](const auto& a, const auto& b)
        {
            return a.second < b.second; 
        });
    }

    void TransformSystem::MarkDirty(const EntityHandle entity)
    {
        auto& registry = GetRegistry();
        auto& transform = registry.GetComponent<TransformComponent>(entity);
        transform.m_isDirty = true;

        // Mark all children dirty, recursively.
        const auto& node = registry.GetComponent<NodeComponent>(entity);
        MarkChildrenDirty(node.m_childrenIDs);
    }

    void TransformSystem::SetParent(const EntityID childID, const EntityID parentID)
    {
        auto& registry = GetRegistry();

        if (childID == parentID)
            return;
        
        const EntityHandle child = registry.GetEntity(childID);
        const EntityHandle parent = registry.GetEntity(parentID);
        SetParent(child, parent);
    }

    void TransformSystem::SetParent(const EntityHandle child, const EntityHandle parent)
    {
        auto& registry = GetRegistry();

        if (!registry.IsValidEntity(child))
            return;

        // Parent is invalid; just unparent the child.
        if (!registry.IsValidEntity(parent))
        {
            RemoveParent(child);
            return;
        }

        // Ensure both have Transform Components
        if (!registry.HasComponent<TransformComponent>(child) || !registry.HasComponent<TransformComponent>(parent))
            return;

        auto& childNode = registry.GetComponent<NodeComponent>(child);
        const auto childID = registry.GetComponent<IDComponent>(child).GetID();
        const auto parentID = registry.GetComponent<IDComponent>(parent).GetID();

        // Parenting to the same parent - no change to be made.
        if (childNode.m_parentID == parentID)
            return;

        // Remove from the old parent, if necessary:
        if (childNode.m_parentID != kInvalidEntityID)
        {
            EntityHandle oldParent = registry.GetEntity(childNode.m_parentID);
            auto& oldParentNode = registry.GetComponent<NodeComponent>(oldParent);
            
            auto it = std::ranges::find(oldParentNode.m_childrenIDs.begin(), oldParentNode.m_childrenIDs.end(), childID);
            if (it != oldParentNode.m_childrenIDs.end())
            {
                oldParentNode.m_childrenIDs.erase(it);
            }
        }

        // Set up the new relationship:
        auto& parentNode = registry.GetComponent<NodeComponent>(parent);
        childNode.m_parentID = parentID;
        parentNode.m_childrenIDs.push_back(childID);
        
        MarkDirty(child);

        // Hierarchy changed:
        m_needsRebuild = true;
    }

    void TransformSystem::RemoveParent(const EntityID childID)
    {
        auto& registry = GetRegistry();
        EntityHandle child = GetRegistry().GetEntity(childID);

        if (!registry.IsValidEntity(child))
            return;

        auto& childNode = registry.GetComponent<NodeComponent>(child);
        if (childNode.m_parentID != kInvalidEntityID)
        {
            EntityHandle parent = GetRegistry().GetEntity(childNode.m_parentID);
            auto& parentNode = registry.GetComponent<NodeComponent>(parent);
            
            auto it = std::find(parentNode.m_childrenIDs.begin(), parentNode.m_childrenIDs.end(), childID);
            if (it != parentNode.m_childrenIDs.end())
            {
                parentNode.m_childrenIDs.erase(it);
            }
            
            childNode.m_parentID = kInvalidEntityID;

            // The hierarchy has changed; needs to be updated.
            MarkDirty(child);
            m_needsRebuild = true;
        }
    }

    void TransformSystem::RemoveParent(const EntityHandle child)
    {
        auto& registry = GetRegistry();
        if (!registry.IsValidEntity(child))
            return;

        auto& childNode = registry.GetComponent<NodeComponent>(child);
        if (childNode.m_parentID != kInvalidEntityID)
        {
            EntityHandle parent = GetRegistry().GetEntity(childNode.m_parentID);
            auto& parentNode = registry.GetComponent<NodeComponent>(parent);
            
            const auto childID = registry.GetComponent<IDComponent>(child).GetID();
            auto it = std::find(parentNode.m_childrenIDs.begin(), parentNode.m_childrenIDs.end(), childID);
            if (it != parentNode.m_childrenIDs.end())
            {
                parentNode.m_childrenIDs.erase(it);
            }
            
            childNode.m_parentID = kInvalidEntityID;

            // The hierarchy has changed; needs to be updated.
            MarkDirty(child);
            m_needsRebuild = true;
        }
    }

    void TransformSystem::TransformLocal(const EntityHandle entity, const Vec3& translation, const Rotation& rotation, const Vec3& scale)
    {
        auto& registry = GetRegistry();
        if (!registry.IsValidEntity(entity))
            return;
        
        auto& transform = registry.GetComponent<TransformComponent>(entity);
        transform.m_localPosition += translation;
        transform.m_localRotation += rotation;
        transform.m_localScale *= scale;
        MarkDirty(entity);
    }

    void TransformSystem::TranslateLocal(const EntityHandle entity, const Vec3& translation)
    {
        auto& registry = GetRegistry();
        if (!registry.IsValidEntity(entity))
            return;
        
        auto& transform = registry.GetComponent<TransformComponent>(entity);
        transform.m_localPosition += translation;
        MarkDirty(entity);
    }

    void TransformSystem::RotateLocal(const EntityHandle entity, const Rotation& rotation)
    {
        auto& registry = GetRegistry();
        if (!registry.IsValidEntity(entity))
            return;
        
        auto& transform = registry.GetComponent<TransformComponent>(entity);
        transform.m_localRotation += rotation;
        MarkDirty(entity);
    }

    void TransformSystem::RotateLocal(const EntityHandle entity, const float angle, const Vec3& axis)
    {
        auto& registry = GetRegistry();
        if (!registry.IsValidEntity(entity))
            return;
        
        auto& transform = registry.GetComponent<TransformComponent>(entity);
        transform.m_localRotation = Quat::FromAxisAngle(axis, angle).ToEulerAngles() * math::RadiansToDegrees();
        MarkDirty(entity);
    }

    void TransformSystem::ScaleLocal(const EntityHandle entity, const float uniformScale)
    {
        auto& registry = GetRegistry();
        if (!registry.IsValidEntity(entity))
            return;

        auto& transform = registry.GetComponent<TransformComponent>(entity);
        transform.m_localScale *= uniformScale;
        MarkDirty(entity);
    }

    void TransformSystem::ScaleLocal(const EntityHandle entity, const Vec3& scale)
    {
        auto& registry = GetRegistry();
        if (!registry.IsValidEntity(entity))
            return;
        
        auto& transform = registry.GetComponent<TransformComponent>(entity);
        transform.m_localScale *= scale;
        MarkDirty(entity);
    }

    void TransformSystem::SetLocalPosition(const EntityHandle entity, const Vec3& position)
    {
        auto& registry = GetRegistry();
        if (!registry.IsValidEntity(entity))
            return;
        
        auto& transform = registry.GetComponent<TransformComponent>(entity);
        transform.m_localPosition = position;
        MarkDirty(entity);
    }

    void TransformSystem::SetLocalRotation(const EntityHandle entity, const Rotation& rotation)
    {
        auto& registry = GetRegistry();
        if (!registry.IsValidEntity(entity))
            return;
        
        auto& transform = registry.GetComponent<TransformComponent>(entity);
        transform.m_localRotation = rotation;
        MarkDirty(entity);
    }

    void TransformSystem::SetLocalScale(const EntityHandle entity, const Vec3& scale)
    {
        auto& registry = GetRegistry();
        if (!registry.IsValidEntity(entity))
            return;
        
        auto& transform = registry.GetComponent<TransformComponent>(entity);
        transform.m_localScale = scale;
        MarkDirty(entity);
    }

    void TransformSystem::SetLocalTransform(const EntityHandle entity, const Vec3 position, const Rotation rotation, const Vec3 scale)
    {
        auto& registry = GetRegistry();
        if (!registry.IsValidEntity(entity))
            return;
        
        auto& transform = registry.GetComponent<TransformComponent>(entity);
        transform.m_localPosition = position;
        transform.m_localRotation = rotation;
        transform.m_localScale = scale;
        MarkDirty(entity);
    }

    void TransformSystem::SetWorldTransform(const EntityHandle entity, const Vec3 position, const Rotation rotation, const Vec3 scale)
    {
        auto& registry = GetRegistry();
        if (!registry.IsValidEntity(entity))
            return;
        
        auto& transform = registry.GetComponent<TransformComponent>(entity);
        auto& node = registry.GetComponent<NodeComponent>(entity);
        
        // If no parent, then world space = local space
        if (node.m_parentID == kInvalidEntityID)
        {
            transform.m_localPosition = position;
            transform.m_localRotation = rotation;
            transform.m_localScale = scale;
            MarkDirty(entity);
        }
        else
        {
            // Convert to local space.
            EntityHandle parent = registry.GetEntity(node.m_parentID);

            auto& parentTransform = registry.GetComponent<TransformComponent>(parent);
            const Mat44 parentInverse = parentTransform.m_worldMatrix.Inversed();
            
            transform.m_localPosition = parentInverse.TransformPoint(position);
            transform.m_localRotation = (parentTransform.m_worldRotation - rotation).Normalized();
            transform.m_localScale = scale / parentTransform.GetWorldScale();
        }
        
        MarkDirty(entity);
    }

    void TransformSystem::SetWorldPosition(const EntityHandle entity, const Vec3 position)
    {
        auto& registry = GetRegistry();
        if (!registry.IsValidEntity(entity))
            return;
        
        auto& transform = registry.GetComponent<TransformComponent>(entity);
        auto& node = registry.GetComponent<NodeComponent>(entity);

        // If no parent, then world position = local position
        if (node.m_parentID == kInvalidEntityID)
        {
            transform.m_localPosition = position;
            MarkDirty(entity);
        }
        else
        {
            // Convert to local space.
            EntityHandle parent = GetRegistry().GetEntity(node.m_parentID);

            auto& parentTransform = registry.GetComponent<TransformComponent>(parent);
            Mat44 parentInverse = parentTransform.m_worldMatrix.Inversed();
            transform.m_localPosition = parentInverse.TransformPoint(position);
        }

        MarkDirty(entity);
    }

    void TransformSystem::SetWorldRotation(const EntityHandle entity, const Rotation rotation)
    {
        auto& registry = GetRegistry();
        if (!registry.IsValidEntity(entity))
            return;
        
        auto& transform = registry.GetComponent<TransformComponent>(entity);
        auto& node = registry.GetComponent<NodeComponent>(entity);

        // Update the local rotation: 
        if (node.m_parentID == kInvalidEntityID)
        {
            // If no parent, then world rotation = local rotation
            transform.m_localRotation = rotation;
            MarkDirty(entity);
        }
        else
        {
            // Convert to local space.
            EntityHandle parent = GetRegistry().GetEntity(node.m_parentID);
            auto& parentTransform = registry.GetComponent<TransformComponent>(parent);
            transform.m_localRotation = (parentTransform.m_worldRotation - rotation).Normalized();
        }

        MarkDirty(entity);
    }

    void TransformSystem::SetWorldScale(const EntityHandle entity, const Vec3 scale)
    {
        auto& registry = GetRegistry();
        if (!registry.IsValidEntity(entity))
            return;
        
        auto& transform = registry.GetComponent<TransformComponent>(entity);
        auto& node = registry.GetComponent<NodeComponent>(entity);

        // If no parent, then world scale = local scale
        if (node.m_parentID == kInvalidEntityID)
        {
            transform.m_localScale = scale;
            MarkDirty(entity);
        }
        else
        {
            // Convert to local space.
            EntityHandle parent = GetRegistry().GetEntity(node.m_parentID);
            auto& parentTransform = registry.GetComponent<TransformComponent>(parent);
            transform.m_localScale = scale / parentTransform.GetWorldScale();
        }

        MarkDirty(entity);
    }

    void TransformSystem::TranslateWorld(const EntityHandle entity, const Vec3& translation)
    {
        auto& registry = GetRegistry();
        if (!registry.IsValidEntity(entity))
            return;
        
        auto& transform = registry.GetComponent<TransformComponent>(entity);
        SetWorldPosition(entity, transform.GetWorldPosition() + translation);
    }

    void TransformSystem::RotateWorld(const EntityHandle entity, const Rotation& rotation)
    {
        auto& registry = GetRegistry();
        if (!registry.IsValidEntity(entity))
            return;
        
        auto& transform = registry.GetComponent<TransformComponent>(entity);
        SetWorldRotation(entity, transform.GetWorldRotation() + rotation);
    }

    void TransformSystem::ScaleWorld(const EntityHandle entity, const float uniformScale)
    {
        auto& registry = GetRegistry();
        if (!registry.IsValidEntity(entity))
            return;
        
        auto& transform = registry.GetComponent<TransformComponent>(entity);
        SetWorldScale(entity, transform.GetWorldScale() * uniformScale);
    }

    void TransformSystem::ScaleWorld(const EntityHandle entity, const Vec3& scale)
    {
        auto& registry = GetRegistry();
        if (!registry.IsValidEntity(entity))
            return;
        
        auto& transform = registry.GetComponent<TransformComponent>(entity);
        SetWorldScale(entity, transform.GetWorldScale() * scale);
    }

    void TransformSystem::ComputeDepthRecursively(const EntityHandle entity, const uint32 depth)
    {
        auto& registry = GetRegistry();
        auto& transform = registry.GetComponent<TransformComponent>(entity);
        transform.m_hierarchyDepth = depth;

        auto& node = registry.GetComponent<NodeComponent>(entity);
        for (EntityID childID : node.m_childrenIDs)
        {
            auto childEntity = GetRegistry().GetEntity(childID);
            if (registry.IsValidEntity(childID))
            {
                ComputeDepthRecursively(childEntity, depth + 1);
            }
        }
    }

    void TransformSystem::MarkChildrenDirty(const std::vector<EntityID>& childIDs)
    {
        auto& registry = GetRegistry();
        
        for (const EntityID childID : childIDs)
        {
            EntityHandle entity = registry.GetEntity(childID);
            if (!registry.IsValidEntity(entity))
                continue;

            auto& transform = registry.GetComponent<TransformComponent>(entity);
            transform.m_isDirty = true;

            auto& node = registry.GetComponent<NodeComponent>(entity);
            MarkChildrenDirty(node.m_childrenIDs);
        }
    }

    void TransformSystem::UpdateSingleTransform(const EntityHandle entity) const
    {
        auto& registry = GetRegistry();
        if (!registry.IsValidEntity(entity))
            return;

        auto& transform = registry.GetComponent<TransformComponent>(entity);

        // Skip unchanging transforms.
        if (!transform.m_isDirty)
            return;

        // Update the local matrix.
        transform.m_localMatrix = Mat44::ComposeTransform(transform.m_localPosition, transform.m_localRotation, transform.m_localScale);

        // Compute the world matrix.
        const auto& node = registry.GetComponent<NodeComponent>(entity);
        if (node.m_parentID != 0)
        {
            auto parent = GetRegistry().GetEntity(node.m_parentID);
            if (registry.IsValidEntity(parent))
            {
                const auto& parentTransform = registry.GetComponent<TransformComponent>(parent);
                transform.m_worldMatrix = parentTransform.m_worldMatrix * transform.m_localMatrix;
                transform.m_worldRotation = (parentTransform.m_worldRotation + transform.m_localRotation).Normalized();
            }
            else
            {
                transform.m_worldMatrix = transform.m_localMatrix;
                transform.m_worldRotation = transform.m_localRotation;
            }
        }
        else
        {
            // This is a root:
            transform.m_worldMatrix = transform.m_localMatrix;
            transform.m_worldRotation = transform.m_localRotation;
        }

        // Transform is updated.
        transform.m_isDirty = false;
    }
}