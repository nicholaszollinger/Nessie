// FreeCamMoveComponent.h
#pragma once
#include "Entity3DComponent.h"
#include "Math/Vector3.h"

namespace nes
{
    class Event;
    class Entity3D;
    
    //----------------------------------------------------------------------------------------------------
    ///		@brief : Actor Component for moving around in 3D space as a free cam.
    //----------------------------------------------------------------------------------------------------
    class FreeCamMovementComponent final : public TickableEntity3DComponent
    {
        NES_DEFINE_COMPONENT_TYPE(FreeCamMovementComponent)
        
        Vector3 m_inputMovement = Vector3::GetZeroVector();
        Vector2 m_inputRotation = Vector2::GetZeroVector();
        float m_moveSpeed = 10.f;
        float m_turnSpeedYaw = 30.f;
        float m_turnSpeedPitch = 20.f;
        bool m_rotationEnabled = true;

    public:
        void SetMoveSpeed(const float speed)        { m_moveSpeed = speed; }
        void SetTurnSpeedYaw(const float speed)     { m_turnSpeedYaw = speed; }
        void SetTurnSpeedPitch(const float speed)   { m_turnSpeedPitch = speed; }
        
        [[nodiscard]] float GetMoveSpeed() const                              { return m_moveSpeed; }
        [[nodiscard]] float GetTurnSpeedYaw() const                           { return m_turnSpeedYaw; }
        [[nodiscard]] float GetTurnSpeedPitch() const                         { return m_turnSpeedPitch; }
        
    protected:
        virtual bool Init() override;
        virtual void OnEnabled() override;
        virtual void OnDisabled() override;

        void ProcessInput();
        void OnEvent(Event& event);
        virtual void Tick(const float deltaTime);
    };
}
