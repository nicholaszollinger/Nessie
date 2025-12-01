// TransformSystem.cpp
#include "TransformSystem.h"
#include "Nessie/World.h"
#include "Nessie/FileIO/YAML/Serializers/YamlMathSerializers.h"

namespace nes
{
    const Mat44& TransformComponent::GetLocalTransformMatrix() const
    {
        return Mat44::ComposeTransform(m_localPosition, m_localRotation, m_localScale);
    }

    const Mat44& TransformComponent::GetWorldTransformMatrix() const
    {
        return Mat44::ComposeTransform(m_worldPosition, m_worldRotation, m_worldScale);
    }

    Mat44 TransformComponent::GetWorldToLocalTransformMatrix() const
    {
        return GetWorldTransformMatrix().Inversed();
    }

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
        
        //component.m_localMatrix = Mat44::ComposeTransform(component.m_localPosition, component.m_localRotation, component.m_localScale);
        //component.m_worldMatrix = component.m_localMatrix;
        component.m_worldPosition = component.m_localPosition;
        component.m_worldRotation = component.m_localRotation;
        component.m_worldScale = component.m_localScale;
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
        auto* pRegistry = GetEntityRegistry();
        if (!pRegistry)
            return;
        
        auto view = pRegistry->GetAllEntitiesWith<TransformComponent, PendingInitialization>();
        
        for (auto entity : view)
        {
            m_needsRebuild = true;

            // Ensure that this entity has a NodeComponent.
            if (!pRegistry->HasComponent<NodeComponent>(entity))
            {
                pRegistry->AddComponent<NodeComponent>(entity);
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

        auto* registry = GetEntityRegistry();
        if (!registry)
            return;
        
        auto view = registry->GetAllEntitiesWith<TransformComponent, PendingDestruction>();

        for (auto entity : view)
        {
            auto& node = registry->GetComponent<NodeComponent>(entity);
            const auto parentID = node.m_parentID;
            
            // If the parent is valid, we need to remove it from the tree.
            // We are orphaning all children.
            if (node.m_parentID != kInvalidEntityID)
            {
                RemoveParent(*registry, entity); // This will set the 'm_needsRebuild' flag.
            }

            const auto parentEntity = registry->GetEntity(parentID);

            // Re-parent to the deleted entity's parent.
            for (auto childID : node.m_childrenIDs)
            {
                const auto otherChildEntity = registry->GetEntity(childID);
                
                // If the child is also being destroyed, skip.
                if (view.contains(otherChildEntity))
                    continue;
                
                SetParent(*registry, otherChildEntity, parentEntity);
            }
        }
    }

    void TransformSystem::UpdateHierarchy()
    {
        auto* pRegistry = GetEntityRegistry();
        if (!pRegistry)
            return;
        
        if (m_needsRebuild)
            RebuildHierarchyCache(*pRegistry);


        // [TODO]: Consider threading.
        for (const auto& [entity, _] : m_depthOrderedEntities)
        {
            UpdateSingleTransform(*pRegistry, entity);
        }
    }

    void TransformSystem::RebuildHierarchyCache(EntityRegistry& registry)
    {
        m_depthOrderedEntities.clear();
        
        auto view = registry.GetAllEntitiesWith<TransformComponent>();
        for (auto entity : view)
        {
            const auto& node = registry.GetComponent<NodeComponent>(entity);

            // If the parent is invalid, then it is a root - we can now compute the depth
            // for all child transforms.
            if (node.m_parentID == kInvalidEntityID)
            {
                ComputeDepthRecursively(registry, entity, 0);
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
        auto* pRegistry = GetEntityRegistry();
        if (!pRegistry)
            return;

        MarkDirty(*pRegistry, entity);
    }

    void TransformSystem::SetParent(const EntityID childID, const EntityID parentID)
    {
        if (childID == parentID)
            return;
        
        auto* pRegistry = GetEntityRegistry();
        if (!pRegistry)
            return;
        
        const EntityHandle child = pRegistry->GetEntity(childID);
        const EntityHandle parent = pRegistry->GetEntity(parentID);
        SetParent(*pRegistry, child, parent);
    }

    void TransformSystem::SetParent(const EntityHandle child, const EntityHandle parent)
    {
        auto* pRegistry = GetEntityRegistry();
        if (!pRegistry)
            return;

        SetParent(*pRegistry, child, parent);
    }

    void TransformSystem::RemoveParent(const EntityID childID)
    {
        auto* pRegistry = GetEntityRegistry();
        if (!pRegistry || !pRegistry->IsValidEntity(childID))
            return;

        RemoveParent(*pRegistry, pRegistry->GetEntity(childID));
    }

    void TransformSystem::RemoveParent(const EntityHandle child)
    {
        auto* pRegistry = GetEntityRegistry();
        if (!pRegistry || !pRegistry->IsValidEntity(child))
            return;

        RemoveParent(*pRegistry, child);
    }

    void TransformSystem::TransformLocal(const EntityHandle entity, const Vec3& translation, const Rotation& rotation, const Vec3& scale)
    {
        auto* pRegistry = GetEntityRegistry();
        if (!pRegistry || !pRegistry->IsValidEntity(entity))
            return;
        
        auto& transform = pRegistry->GetComponent<TransformComponent>(entity);
        transform.m_localPosition += translation;
        transform.m_localRotation += rotation;
        transform.m_localScale *= scale;
        MarkDirty(*pRegistry, entity);
    }

    void TransformSystem::TranslateLocal(const EntityHandle entity, const Vec3& translation)
    {
        auto* pRegistry = GetEntityRegistry();
        if (!pRegistry || !pRegistry->IsValidEntity(entity))
            return;
        
        auto& transform = pRegistry->GetComponent<TransformComponent>(entity);
        transform.m_localPosition += translation;
        MarkDirty(*pRegistry, entity);
    }

    void TransformSystem::RotateLocal(const EntityHandle entity, const Rotation& rotation)
    {
        auto* pRegistry = GetEntityRegistry();
        if (!pRegistry || !pRegistry->IsValidEntity(entity))
            return;
        
        auto& transform = pRegistry->GetComponent<TransformComponent>(entity);
        transform.m_localRotation += rotation;
        MarkDirty(*pRegistry, entity);
    }

    void TransformSystem::RotateLocal(const EntityHandle entity, const float angle, const Vec3& axis)
    {
        auto* pRegistry = GetEntityRegistry();
        if (!pRegistry || !pRegistry->IsValidEntity(entity))
            return;
        
        auto& transform = pRegistry->GetComponent<TransformComponent>(entity);
        transform.m_localRotation = Quat::FromAxisAngle(axis, angle).ToEulerAngles() * math::RadiansToDegrees();
        MarkDirty(*pRegistry, entity);
    }

    void TransformSystem::ScaleLocal(const EntityHandle entity, const float uniformScale)
    {
        auto* pRegistry = GetEntityRegistry();
        if (!pRegistry || !pRegistry->IsValidEntity(entity))
            return;

        auto& transform = pRegistry->GetComponent<TransformComponent>(entity);
        transform.m_localScale *= uniformScale;
        MarkDirty(*pRegistry, entity);
    }

    void TransformSystem::ScaleLocal(const EntityHandle entity, const Vec3& scale)
    {
        auto* pRegistry = GetEntityRegistry();
        if (!pRegistry || !pRegistry->IsValidEntity(entity))
            return;
        
        auto& transform = pRegistry->GetComponent<TransformComponent>(entity);
        transform.m_localScale *= scale;
        MarkDirty(*pRegistry, entity);
    }

    void TransformSystem::SetLocalPosition(const EntityHandle entity, const Vec3& position)
    {
        auto* pRegistry = GetEntityRegistry();
        if (!pRegistry || !pRegistry->IsValidEntity(entity))
            return;
        
        auto& transform = pRegistry->GetComponent<TransformComponent>(entity);
        transform.m_localPosition = position;
        MarkDirty(*pRegistry, entity);
    }

    void TransformSystem::SetLocalRotation(const EntityHandle entity, const Rotation& rotation)
    {
        auto* pRegistry = GetEntityRegistry();
        if (!pRegistry || !pRegistry->IsValidEntity(entity))
            return;
        
        auto& transform = pRegistry->GetComponent<TransformComponent>(entity);
        transform.m_localRotation = rotation;
        MarkDirty(*pRegistry, entity);
    }

    void TransformSystem::SetLocalScale(const EntityHandle entity, const Vec3& scale)
    {
        auto* pRegistry = GetEntityRegistry();
        if (!pRegistry || !pRegistry->IsValidEntity(entity))
            return;
        
        auto& transform = pRegistry->GetComponent<TransformComponent>(entity);
        transform.m_localScale = scale;
        MarkDirty(*pRegistry, entity);
    }

    void TransformSystem::SetLocalTransform(const EntityHandle entity, const Vec3 position, const Rotation rotation, const Vec3 scale)
    {
        auto* pRegistry = GetEntityRegistry();
        if (!pRegistry || !pRegistry->IsValidEntity(entity))
            return;
        
        auto& transform = pRegistry->GetComponent<TransformComponent>(entity);
        transform.m_localPosition = position;
        transform.m_localRotation = rotation;
        transform.m_localScale = scale;
        MarkDirty(*pRegistry, entity);
    }

    void TransformSystem::SetWorldTransform(const EntityHandle entity, const Vec3 position, const Rotation rotation, const Vec3 scale)
    {
        auto* pRegistry = GetEntityRegistry();
        if (!pRegistry || !pRegistry->IsValidEntity(entity))
            return;
        
        SetWorldTransform(*pRegistry, entity, position, rotation, scale);
    }

    void TransformSystem::SetWorldPosition(const EntityHandle entity, const Vec3 position)
    {
        auto* pRegistry = GetEntityRegistry();
        if (!pRegistry || !pRegistry->IsValidEntity(entity))
            return;

        SetWorldPosition(*pRegistry, entity, position);
    }

    void TransformSystem::SetWorldRotation(const EntityHandle entity, const Rotation rotation)
    {
        auto* pRegistry = GetEntityRegistry();
        if (!pRegistry || !pRegistry->IsValidEntity(entity))
            return;

        SetWorldRotation(*pRegistry, entity, rotation);
    }

    void TransformSystem::SetWorldScale(const EntityHandle entity, const Vec3 scale)
    {
        auto* pRegistry = GetEntityRegistry();
        if (!pRegistry || !pRegistry->IsValidEntity(entity))
            return;

        SetWorldScale(*pRegistry, entity, scale);
    }

    void TransformSystem::MarkDirty(EntityRegistry& registry, const EntityHandle entity)
    {
        auto& transform = registry.GetComponent<TransformComponent>(entity);
        transform.m_isDirty = true;

        // Mark all children dirty, recursively.
        const auto& node = registry.GetComponent<NodeComponent>(entity);
        MarkChildrenDirty(registry, node.m_childrenIDs);
    }

    void TransformSystem::SetParent(EntityRegistry& registry, const EntityHandle child, const EntityHandle parent)
    {
        if (!registry.IsValidEntity(child))
            return;

        // Parent is invalid; just unparent the child.
        if (!registry.IsValidEntity(parent))
        {
            RemoveParent(registry, child);
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

        // Calculate the child's new local transform based on the new parent.
        auto& childTransform = registry.GetComponent<TransformComponent>(child);
        auto& parentTransform = registry.GetComponent<TransformComponent>(parent);
        
        const Vec3 localPosition = childTransform.GetWorldPosition() - parentTransform.GetLocalPosition(); 
        const Rotation localRotation = childTransform.GetWorldRotation() - parentTransform.GetWorldRotation();
        const Vec3 localScale = childTransform.GetWorldScale() / parentTransform.GetWorldScale();
        
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

        // Set the calculated local transform.
        childTransform.m_localPosition = localPosition;
        childTransform.m_localRotation = localRotation;
        childTransform.m_localScale = localScale;
        
        MarkDirty(registry, child);

        // Hierarchy changed:
        m_needsRebuild = true;
    }

    void TransformSystem::RemoveParent(EntityRegistry& registry, const EntityHandle entity)
    {
        auto& childNode = registry.GetComponent<NodeComponent>(entity);
        if (childNode.m_parentID != kInvalidEntityID)
        {
            EntityHandle parent = registry.GetEntity(childNode.m_parentID);
            auto& parentNode = registry.GetComponent<NodeComponent>(parent);
            
            const auto childID = registry.GetComponent<IDComponent>(entity).GetID();
            auto it = std::find(parentNode.m_childrenIDs.begin(), parentNode.m_childrenIDs.end(), childID);
            if (it != parentNode.m_childrenIDs.end())
            {
                parentNode.m_childrenIDs.erase(it);
            }
            
            childNode.m_parentID = kInvalidEntityID;

            // The child has no parent, so its local space is the new world space.
            auto& transform = registry.GetComponent<TransformComponent>(entity);
            transform.m_localPosition = transform.m_worldPosition;
            transform.m_localRotation = transform.m_worldRotation;
            transform.m_localScale = transform.m_worldScale;

            // The hierarchy has changed; needs to be updated.
            MarkDirty(registry, entity);
            m_needsRebuild = true;
        }
    }

    void TransformSystem::SetWorldTransform(EntityRegistry& registry, const EntityHandle entity, const Vec3 position, const Rotation rotation, const Vec3 scale)
    {
        auto& transform = registry.GetComponent<TransformComponent>(entity);
        auto& node = registry.GetComponent<NodeComponent>(entity);
        
        // If no parent, then world space = local space
        if (node.m_parentID == kInvalidEntityID)
        {
            transform.m_localPosition = position;
            transform.m_localRotation = rotation;
            transform.m_localScale = scale;
            MarkDirty(registry, entity);
        }
        else
        {
            // Convert to local space.
            EntityHandle parent = registry.GetEntity(node.m_parentID);
            auto& parentTransform = registry.GetComponent<TransformComponent>(parent);
            const Mat44 worldToLocalSpace = parentTransform.GetWorldToLocalTransformMatrix();
            
            transform.m_localPosition = worldToLocalSpace.TransformPoint(position);
            transform.m_localRotation = (parentTransform.m_worldRotation - rotation).Normalized();
            transform.m_localScale = scale / parentTransform.GetWorldScale();
        }
        
        MarkDirty(registry, entity);
    }

    void TransformSystem::SetWorldPosition(EntityRegistry& registry, const EntityHandle entity, const Vec3 position)
    {
        auto& transform = registry.GetComponent<TransformComponent>(entity);
        auto& node = registry.GetComponent<NodeComponent>(entity);

        // If no parent, then world position = local position
        if (node.m_parentID == kInvalidEntityID)
        {
            transform.m_localPosition = position;
            MarkDirty(registry, entity);
        }
        else
        {
            // Convert to local space.
            EntityHandle parent = registry.GetEntity(node.m_parentID);

            auto& parentTransform = registry.GetComponent<TransformComponent>(parent);
            Mat44 worldToLocalSpace = parentTransform.GetWorldToLocalTransformMatrix();
            transform.m_localPosition = worldToLocalSpace.TransformPoint(position);
        }
        
        MarkDirty(registry, entity);
    }

    void TransformSystem::SetWorldRotation(EntityRegistry& registry, const EntityHandle entity, const Rotation rotation)
    {
        auto& transform = registry.GetComponent<TransformComponent>(entity);
        auto& node = registry.GetComponent<NodeComponent>(entity);

        // Update the local rotation: 
        if (node.m_parentID == kInvalidEntityID)
        {
            // If no parent, then world rotation = local rotation
            transform.m_localRotation = rotation;
            MarkDirty(registry, entity);
        }
        else
        {
            // Convert to local space.
            EntityHandle parent = registry.GetEntity(node.m_parentID);
            auto& parentTransform = registry.GetComponent<TransformComponent>(parent);
            transform.m_localRotation = (parentTransform.m_worldRotation - rotation).Normalized();
        }

        MarkDirty(registry, entity);
    }

    void TransformSystem::SetWorldScale(EntityRegistry& registry, const EntityHandle entity, const Vec3 scale)
    {
        auto& transform = registry.GetComponent<TransformComponent>(entity);
        auto& node = registry.GetComponent<NodeComponent>(entity);

        // If no parent, then world scale = local scale
        if (node.m_parentID == kInvalidEntityID)
        {
            transform.m_localScale = scale;
            MarkDirty(registry, entity);
        }
        else
        {
            // Convert to local space.
            EntityHandle parent = registry.GetEntity(node.m_parentID);
            auto& parentTransform = registry.GetComponent<TransformComponent>(parent);
            transform.m_localScale = scale / parentTransform.GetWorldScale();
        }

        MarkDirty(registry, entity);
    }

    void TransformSystem::TranslateWorld(const EntityHandle entity, const Vec3& translation)
    {
        auto* pRegistry = GetEntityRegistry();
        if (!pRegistry || !pRegistry->IsValidEntity(entity))
            return;
        
        auto& transform = pRegistry->GetComponent<TransformComponent>(entity);
        SetWorldPosition(*pRegistry, entity, transform.GetWorldPosition() + translation);
    }

    void TransformSystem::RotateWorld(const EntityHandle entity, const Rotation& rotation)
    {
        auto* pRegistry = GetEntityRegistry();
        if (!pRegistry || !pRegistry->IsValidEntity(entity))
            return;
        
        auto& transform = pRegistry->GetComponent<TransformComponent>(entity);
        SetWorldRotation(*pRegistry, entity, transform.GetWorldRotation() + rotation);
    }

    void TransformSystem::ScaleWorld(const EntityHandle entity, const float uniformScale)
    {
        auto* pRegistry = GetEntityRegistry();
        if (!pRegistry || !pRegistry->IsValidEntity(entity))
            return;
        
        auto& transform = pRegistry->GetComponent<TransformComponent>(entity);
        SetWorldScale(*pRegistry, entity, transform.GetWorldScale() * uniformScale);
    }

    void TransformSystem::ScaleWorld(const EntityHandle entity, const Vec3& scale)
    {
        auto* pRegistry = GetEntityRegistry();
        if (!pRegistry || !pRegistry->IsValidEntity(entity))
            return;
        
        auto& transform = pRegistry->GetComponent<TransformComponent>(entity);
        SetWorldScale(*pRegistry, entity, transform.GetWorldScale() * scale);
    }

    void TransformSystem::ComputeDepthRecursively(EntityRegistry& registry, const EntityHandle entity, const uint32 depth)
    {
        auto& transform = registry.GetComponent<TransformComponent>(entity);
        transform.m_hierarchyDepth = depth;

        auto& node = registry.GetComponent<NodeComponent>(entity);
        for (EntityID childID : node.m_childrenIDs)
        {
            auto childEntity = registry.GetEntity(childID);
            if (registry.IsValidEntity(childID))
            {
                ComputeDepthRecursively(registry, childEntity, depth + 1);
            }
        }
    }

    void TransformSystem::MarkChildrenDirty(EntityRegistry& registry, const std::vector<EntityID>& childIDs)
    {
        for (const EntityID childID : childIDs)
        {
            EntityHandle entity = registry.GetEntity(childID);
            if (!registry.IsValidEntity(entity))
                continue;

            auto& transform = registry.GetComponent<TransformComponent>(entity);
            transform.m_isDirty = true;

            auto& node = registry.GetComponent<NodeComponent>(entity);
            MarkChildrenDirty(registry, node.m_childrenIDs);
        }
    }

    void TransformSystem::UpdateSingleTransform(EntityRegistry& registry, const EntityHandle entity) const
    {
        if (!registry.IsValidEntity(entity))
            return;

        auto& transform = registry.GetComponent<TransformComponent>(entity);

        // Skip unchanging transforms.
        if (!transform.m_isDirty)
            return;
        
        // Compute the world matrix.
        const auto& node = registry.GetComponent<NodeComponent>(entity);
        if (node.m_parentID != 0)
        {
            auto parent = registry.GetEntity(node.m_parentID);
            if (registry.IsValidEntity(parent))
            {
                const auto& parentTransform = registry.GetComponent<TransformComponent>(parent);
                transform.m_worldPosition = parentTransform.m_worldPosition + parentTransform.m_worldRotation.RotatedVector(transform.m_localPosition);
                transform.m_worldRotation = (parentTransform.m_worldRotation + transform.m_localRotation).Normalized();
                transform.m_worldScale = parentTransform.m_worldScale * transform.m_localScale;
            }
            else
            {
                // Parent is invalid, treat as a root.
                transform.m_worldPosition = transform.m_localPosition;
                transform.m_worldRotation = transform.m_localRotation;
                transform.m_worldScale = transform.m_localScale;
            }
        }
        else
        {
            // This is a root:
            transform.m_worldPosition = transform.m_localPosition;
            transform.m_worldRotation = transform.m_localRotation;
            transform.m_worldScale = transform.m_localScale;
        }

        // Transform is updated.
        transform.m_isDirty = false;
    }
}