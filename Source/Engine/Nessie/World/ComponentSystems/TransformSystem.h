// TransformSystem.h
#pragma once
#include "Nessie/World/Entity.h"
#include "Nessie/World/ComponentSystem.h"
#include "Nessie/Math/Math.h"

namespace nes
{
    //----------------------------------------------------------------------------------------------------
    /// @brief : Represents an Entity's 3D position, rotation and scale, both in local and world space.
    /// The entity's transform can only be updated using the TransformSystem; it cannot be updated directly.
    //----------------------------------------------------------------------------------------------------
    class TransformComponent
    {
    public:
        //----------------------------------------------------------------------------------------------------
        ///	@brief : Get the Entity's location, in world space. 
        //----------------------------------------------------------------------------------------------------
        Vec3                    GetWorldPosition() const        { return m_worldMatrix.GetTranslation(); }

        //----------------------------------------------------------------------------------------------------
        ///	@brief : Get the Entity's world orientation. 
        //----------------------------------------------------------------------------------------------------
        Rotation                GetWorldRotation() const        { return m_worldRotation; }

        //----------------------------------------------------------------------------------------------------
        ///	@brief : Get the Entity's total world scale. 
        //----------------------------------------------------------------------------------------------------
        Vec3                    GetWorldScale() const           { return m_worldMatrix.GetScale(); }

        //----------------------------------------------------------------------------------------------------
        ///	@brief : Get the Entity's position relative to its parent. 
        //----------------------------------------------------------------------------------------------------
        const Vec3&             GetLocalPosition() const        { return m_localPosition; }
        
        //----------------------------------------------------------------------------------------------------
        ///	@brief : Get the Entity's rotation relative to its parent. 
        //----------------------------------------------------------------------------------------------------
        const Rotation&         GetLocalRotation() const        { return m_localRotation; }

        //----------------------------------------------------------------------------------------------------
        ///	@brief : Get the Entity's scale relative to its parent. 
        //----------------------------------------------------------------------------------------------------
        const Vec3&             GetLocalScale() const           { return m_localScale; }

        //----------------------------------------------------------------------------------------------------
        ///	@brief : Get the Local Transform in its Matrix representation. 
        //----------------------------------------------------------------------------------------------------
        const Mat44&            GetLocalTransformMatrix() const { return m_localMatrix; }

        //----------------------------------------------------------------------------------------------------
        /// @brief : Get the World Transformation Matrix of this Entity. 
        //----------------------------------------------------------------------------------------------------
        const Mat44&            GetWorldTransformMatrix() const { return m_worldMatrix; }

        //----------------------------------------------------------------------------------------------------
        /// @brief : Get the transformation matrix that converts points/directions to local space.
        //----------------------------------------------------------------------------------------------------
        Mat44                   GetWorldToLocalTransformMatrix() const { return m_worldMatrix.Inversed(); }

        static void             Serialize(YamlOutStream& out, const TransformComponent& component);
        static void             Deserialize(const YamlNode& in, TransformComponent& component);
        
    private:
        friend class TransformSystem;
        
        Mat44                   m_worldMatrix = Mat44::Identity();  // Transforms a world position to this entity's space.
        Mat44                   m_localMatrix = Mat44::Identity();  // Transforms from parent space to the entity's space.
        Vec3                    m_localPosition = Vec3::Zero();     // Position relative to its Parent.
        Vec3                    m_localScale = Vec3::One();         // Scale relative to its Parent.
        Rotation                m_localRotation = Rotation::Zero(); // Rotation relative to its Parent.
        Rotation                m_worldRotation = Rotation::Zero(); // Cached world rotation in Rotation form, because converting from Matrix/Quat->Euler angles can result in bad results.
        uint32                  m_hierarchyDepth = 0;               // 0 = Root node.
        bool                    m_isDirty = false;                  // If true, then both the local and world matrices are out of date.
    };

    //----------------------------------------------------------------------------------------------------
    /// @brief : The Transform System updates a 3D hierarchy of Entities using both the TransformComponent
    ///     and NodeComponents. All transformations must be done through the system in order to update the
    ///     hierarchy correctly.
    //----------------------------------------------------------------------------------------------------
    class TransformSystem : public ComponentSystem
    {
    public:
        TransformSystem(WorldBase& world) : ComponentSystem(world){}
        
        virtual void            RegisterComponentTypes() override;
        virtual void            ProcessNewEntities() override;
        virtual void            ProcessDestroyedEntities(const bool clearingRegistry) override;

        //----------------------------------------------------------------------------------------------------
        /// @brief : Should be called every frame. Updates all changed transforms in the hierarchy.
        //----------------------------------------------------------------------------------------------------
        void                    UpdateHierarchy();

        //----------------------------------------------------------------------------------------------------
        /// @brief : Mark an entity's transform as dirty and all children. Should be called anytime the entity's
        ///     transform is updated.
        //----------------------------------------------------------------------------------------------------
        void                    MarkDirty(const EntityHandle entity);

        //----------------------------------------------------------------------------------------------------
        /// @brief : Set an Entity's parent.
        //----------------------------------------------------------------------------------------------------
        void                    SetParent(const EntityID childID, const EntityID parentID);

        //----------------------------------------------------------------------------------------------------
        /// @brief : Set an Entity's parent.
        //----------------------------------------------------------------------------------------------------
        void                    SetParent(const EntityHandle child, const EntityHandle parent);

        //----------------------------------------------------------------------------------------------------
        /// @brief : Remove an Entity's parent. 
        //----------------------------------------------------------------------------------------------------
        void                    RemoveParent(const EntityID childID);

        //----------------------------------------------------------------------------------------------------
        /// @brief : Remove an Entity's parent. Does not reparent to the Parent's parent.
        //----------------------------------------------------------------------------------------------------
        void                    RemoveParent(const EntityHandle child);
        
        //----------------------------------------------------------------------------------------------------
        /// @brief : Translate, Rotate, and Scale an Entity in local space. 
        //----------------------------------------------------------------------------------------------------
        void                    TransformLocal(const EntityHandle entity, const Vec3& translation, const Rotation& rotation, const Vec3& scale = Vec3::One());

        //----------------------------------------------------------------------------------------------------
        ///	@brief : Move an Entity in local space.  
        //----------------------------------------------------------------------------------------------------
        void                    TranslateLocal(const EntityHandle entity, const Vec3& translation);

        //----------------------------------------------------------------------------------------------------
        ///	@brief : Rotate an Entity in local space.  
        //----------------------------------------------------------------------------------------------------
        void                    RotateLocal(const EntityHandle entity, const Rotation& rotation);

        //----------------------------------------------------------------------------------------------------
        ///	@brief : Rotate an Entity in local space.  
        //----------------------------------------------------------------------------------------------------
        void                    RotateLocal(const EntityHandle entity, const float angle, const Vec3& axis);

        //----------------------------------------------------------------------------------------------------
        ///	@brief : Scale an Entity in local space. This will multiply the current scale by the uniform scale.   
        //----------------------------------------------------------------------------------------------------
        void                    ScaleLocal(const EntityHandle entity, const float uniformScale);

        //----------------------------------------------------------------------------------------------------
        ///	@brief : Scale an Entity in local space. This will multiply the current scale by the given scale.  
        //----------------------------------------------------------------------------------------------------
        void                    ScaleLocal(const EntityHandle entity, const Vec3& scale);

        //----------------------------------------------------------------------------------------------------
        /// @brief : Set the Entity's position relative to its parent.
        //----------------------------------------------------------------------------------------------------
        void                    SetLocalPosition(const EntityHandle entity, const Vec3& position);

        //----------------------------------------------------------------------------------------------------
        /// @brief : Set the Entity's rotation relative to its parent.
        //----------------------------------------------------------------------------------------------------
        void                    SetLocalRotation(const EntityHandle entity, const Rotation& rotation);

        //----------------------------------------------------------------------------------------------------
        /// @brief : Set the Entity's scale, relative to its parent.
        //----------------------------------------------------------------------------------------------------
        void                    SetLocalScale(const EntityHandle entity, const Vec3& scale);

        //----------------------------------------------------------------------------------------------------
        /// @brief : Set an Entity's transform, relative to its parent.
        //----------------------------------------------------------------------------------------------------
        void                    SetLocalTransform(const EntityHandle entity, const Vec3 position, const Rotation rotation, const Vec3 scale);

        //----------------------------------------------------------------------------------------------------
        /// @brief : Move an Entity, in world space.
        //----------------------------------------------------------------------------------------------------
        void                    TranslateWorld(const EntityHandle entity, const Vec3& translation);

        //----------------------------------------------------------------------------------------------------
        /// @brief : Rotate an Entity, in world space.
        //----------------------------------------------------------------------------------------------------
        void                    RotateWorld(const EntityHandle entity, const Rotation& rotation);

        //----------------------------------------------------------------------------------------------------
        /// @brief : Scale an Entity, in world space.
        //----------------------------------------------------------------------------------------------------
        void                    ScaleWorld(const EntityHandle entity, const float uniformScale);

        //----------------------------------------------------------------------------------------------------
        /// @brief : Scale an Entity, in world space.
        //----------------------------------------------------------------------------------------------------
        void                    ScaleWorld(const EntityHandle entity, const Vec3& scale);

        //----------------------------------------------------------------------------------------------------
        /// @brief : Set an Entity's world transform directly, regardless of the parent's position.
        //----------------------------------------------------------------------------------------------------
        void                    SetWorldTransform(const EntityHandle entity, const Vec3 position, const Rotation rotation, const Vec3 scale);

        //----------------------------------------------------------------------------------------------------
        /// @brief : Set an Entity's world space position, regardless of its parent. 
        //----------------------------------------------------------------------------------------------------
        void                    SetWorldPosition(const EntityHandle entity, const Vec3 position);

        //----------------------------------------------------------------------------------------------------
        /// @brief : Set an Entity's world rotation, regardless of its parent. 
        //----------------------------------------------------------------------------------------------------
        void                    SetWorldRotation(const EntityHandle entity, const Rotation rotation);

        //----------------------------------------------------------------------------------------------------
        /// @brief : Set an Entity's world scale, regarless of its parent.
        //----------------------------------------------------------------------------------------------------
        void                    SetWorldScale(const EntityHandle entity, const Vec3 scale);
        
    private:
        using EntityDepthPair = std::pair<EntityHandle, uint32>;
        
        //----------------------------------------------------------------------------------------------------
        /// @brief : Walks down a hierarchy, calculating the depth value of the transform component.
        //----------------------------------------------------------------------------------------------------
        void                    ComputeDepthRecursively(const EntityHandle, const uint32 depth);
        
        //----------------------------------------------------------------------------------------------------
        /// @brief : Marks all children transforms as dirty, recursively down the hierarchy.
        //----------------------------------------------------------------------------------------------------
        void                    MarkChildrenDirty(const std::vector<EntityID>& childIDs);

        //----------------------------------------------------------------------------------------------------
        /// @brief : Recalculates the Entity's world and local transform matrices.
        //----------------------------------------------------------------------------------------------------
        void                    UpdateSingleTransform(const EntityHandle entity) const;

        //----------------------------------------------------------------------------------------------------
        /// @brief : Rebuilds the depth ordered array of entities, to process efficiently. Must be called anytime
        /// the hierarchy changes: adding or removing a parent. NeedsHierarchyCacheRebuild() will return true.
        //----------------------------------------------------------------------------------------------------
        void                    RebuildHierarchyCache();

        //----------------------------------------------------------------------------------------------------
        /// @brief : Check to see if the current hierarchy is out of date.
        //----------------------------------------------------------------------------------------------------
        bool                    NeedsHierarchyCacheRebuild() const { return m_needsRebuild; }

    private:
        std::vector<EntityDepthPair> m_depthOrderedEntities;
        bool                    m_needsRebuild = true;
    };

}