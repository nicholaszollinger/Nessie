// FreeCamMoveComponent.h
#pragma once
#include "WorldComponent.h"
#include "Core/Memory/StrongPtr.h"
#include "Math/Vector3.h"

namespace nes
{
    class Event;
    class Actor;
    class WorldComponent;

    //----------------------------------------------------------------------------------------------------
    ///		@brief : Actor Component for moving around in 3D space as a free cam.
    //----------------------------------------------------------------------------------------------------
    class FreeCamMovementComponent : public ActorComponent
    {
        NES_DEFINE_COMPONENT_TYPE(FreeCamMovementComponent)

        StrongPtr<WorldComponent> m_pUpdatedComponent = nullptr;
        Vector3 m_inputMovement = Vector3::GetZeroVector();
        Vector2 m_inputRotation = Vector2::GetZeroVector();
        float m_moveSpeed = 10.f;
        float m_turnSpeedYaw = 30.f;
        float m_turnSpeedPitch = 20.f;
        bool m_rotationEnabled = true;

    public:
        void SetUpdatedComponent(const StrongPtr<WorldComponent>& pUpdatedComponent);
        void SetMoveSpeed(const float speed)        { m_moveSpeed = speed; }
        void SetTurnSpeedYaw(const float speed)     { m_turnSpeedYaw = speed; }
        void SetTurnSpeedPitch(const float speed)   { m_turnSpeedPitch = speed; }

        [[nodiscard]] StrongPtr<WorldComponent> GetUpdatedComponent() const   { return m_pUpdatedComponent; }
        [[nodiscard]] float GetMoveSpeed() const                              { return m_moveSpeed; }
        [[nodiscard]] float GetTurnSpeedYaw() const                           { return m_turnSpeedYaw; }
        [[nodiscard]] float GetTurnSpeedPitch() const                         { return m_turnSpeedPitch; }
        
        //virtual EntityDomain GetDomain() const override final { return EntityDomain::Physical3D; }
        
    protected:
        virtual bool Init() override;
        virtual void OnEnabled() override;
        virtual void OnDisabled() override;

        void ProcessInput();
        void OnEvent(Event& event);
        void ProcessCameraMovement(const float deltaTime);
    };
}
