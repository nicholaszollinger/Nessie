// LifetimeComponents.h
#pragma once
#include "Nessie/World/Component.h"

namespace nes
{
    //----------------------------------------------------------------------------------------------------
    // LIFETIME COMPONENTS
    //
    // Instead of having functions to override lifetime events, like OnInit(), or OnDestroy(), entities are
    // given special components to signify that they need to be initialized, are going to be destroyed, etc.
    //
    // Component Systems can query a set of entities that contain these special components to perform
    // the necessary logic. Example:
    //
    //      void PhysicsSystem::ProcessNewEntities()
    //      {
    //          auto view = m_pRegistry->GetAllEntitiesWith<RigidBodyComponent, TransformComponent, PendingInitialization>();
    //          for (auto entity : view)
    //          {
    //              // ... Add to the physics scene, etc.
    //          }
    //      }
    //
    //      void PhysicsSystem::ProcessDestroyedEntities()
    //      {
    //          auto view = m_pRegistry->GetAllEntitiesWith<RigidBodyComponent, TransformComponent, PendingDestruction>();
    //          for (auto entity : view)
    //          {
    //              // ... Add to the physics scene, etc.
    //          }
    //      }
    //    
    //
    // These lifetime component will be removed after all systems have processed that specific step. 
    //----------------------------------------------------------------------------------------------------
    
    //----------------------------------------------------------------------------------------------------
    /// @brief : Added to an entity when created. Removed after all systems have completed the initialization
    ///     step.
    //----------------------------------------------------------------------------------------------------
    struct PendingInitialization : MarkerComponentBase{};

    //----------------------------------------------------------------------------------------------------
    /// @brief : Added to an entity when queued to be destroyed. Removed when the entity is destroyed.
    //----------------------------------------------------------------------------------------------------
    struct PendingDestruction : MarkerComponentBase{};

    //----------------------------------------------------------------------------------------------------
    /// @brief : Component added to an entity whenever it is enabled from a disabled state. 
    //----------------------------------------------------------------------------------------------------
    struct PendingEnable : MarkerComponentBase{};

    //----------------------------------------------------------------------------------------------------
    /// @brief : Component added to an entity whenever it is disabled from an enabled state.
    //----------------------------------------------------------------------------------------------------
    struct PendingDisable : MarkerComponentBase{};

    //----------------------------------------------------------------------------------------------------
    /// @brief : If an entity has this component, it is considered disabled.
    //----------------------------------------------------------------------------------------------------
    struct DisabledComponent : MarkerComponentBase{};
}