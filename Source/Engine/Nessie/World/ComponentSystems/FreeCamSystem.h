// FreeCamSystem.h
#pragma once
#include "Nessie/Core/Events/Event.h"
#include "Nessie/Math/Math.h"
#include "Nessie/World/ComponentSystem.h"
#include "Nessie/World/Component.h"

namespace nes
{
    class TransformSystem;
    
    struct FreeCamMovementComponent
    {
        float       m_moveSpeed = 50.f;      // m/s.
        float       m_sensitivity = 1.25f;

        static void Serialize(YAML::Emitter& out, const FreeCamMovementComponent& component);
        static void Deserialize(const YAML::Node& in, FreeCamMovementComponent& component);
    };

    //----------------------------------------------------------------------------------------------------
    /// @brief : This system moves Entities in 3D space who have both a FreeCamMovementComponent and a TransformComponent attached.
    /// Controls:
    /// - WASD to move the Entity left, right, forward and back.
    /// - Space to rise.
    /// - Ctrl to descend.
    /// - Hold right click to enable mouse rotation.
    /// - Holding shift will double the movement speed.
    ///
    /// @note : The system relies on the TransformSystem to move the Entity in space.
    //----------------------------------------------------------------------------------------------------
    class FreeCamSystem : public ComponentSystem
    {
    public:
        FreeCamSystem(WorldBase& world) : ComponentSystem(world) {}

        virtual bool    Init() override;
        virtual void    Shutdown() override;
        virtual void    RegisterComponentTypes() override;

        //----------------------------------------------------------------------------------------------------
        /// @brief : Checks for mouse-right-click events to enable/disable camera rotation.  
        //----------------------------------------------------------------------------------------------------
        void            OnEvent(Event& event);
        
        //----------------------------------------------------------------------------------------------------
        /// @brief : Updates the active camera's position based on input. 
        //----------------------------------------------------------------------------------------------------
        void            Tick(const float deltaTime) const;
    
    protected:
        //----------------------------------------------------------------------------------------------------
        /// @brief : Gets input vectors based on the controls.
        //----------------------------------------------------------------------------------------------------
        void            ProcessInput(Vec3& outInputMovement, Vec2& outInputRotation, bool& outShiftDown) const;

    protected:
        StrongPtr<TransformSystem> m_pTransformSystem = nullptr;
        bool            m_rotationEnabled      = false;
    };
}
