// FreeCamMoveComponent.h
#pragma once
#include "Entity3DComponent.h"
#include "Math/Math.h"

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
        
    public:
        void            SetMoveSpeed(const float speed)        { m_moveSpeed = speed; }
        void            SetTurnSpeedYaw(const float speed)     { m_turnSpeedYaw = speed; }
        void            SetTurnSpeedPitch(const float speed)   { m_turnSpeedPitch = speed; }
        
        float           GetMoveSpeed() const                   { return m_moveSpeed; }
        float           GetTurnSpeedYaw() const                { return m_turnSpeedYaw; }
        float           GetTurnSpeedPitch() const              { return m_turnSpeedPitch; }
        
    protected:
        virtual bool    Init() override;
        virtual void    OnEnabled() override;
        virtual void    OnDisabled() override;

        void            ProcessInput();
        void            OnEvent(Event& event);
        virtual void    Tick(const float deltaTime);

    private:
        Vec3            m_inputMovement = Vec3::Zero();
        Vec2            m_inputRotation = Vec2::Zero();
        float           m_moveSpeed = 10.f;
        float           m_turnSpeedYaw = 30.f;
        float           m_turnSpeedPitch = 20.f;
        bool            m_rotationEnabled = true;
    };
}
