// FreeCamMovementComponent.cpp
#include "FreeCamMovementComponent.h"

#include "Input/InputManager.h"
#include "Math/Quaternion.h"
#include "Scene/Scene.h"
#include "World/Entity3D.h"
#include "World/World.h"

namespace nes
{
    bool FreeCamMovementComponent::Init()
    {
        m_rotationEnabled = false;
        auto* pWorld = GetOwner()->GetWorld();
        //m_tickFunction.SetTickInterval(5.f);
        pWorld->RegisterTickToWorldTickGroup(&m_tickFunction, ETickStage::PrePhysics);
        
        return TickableEntity3DComponent::Init();
    }

    void FreeCamMovementComponent::OnEnabled()
    {
        TickableEntity3DComponent::OnEnabled();
        
        // TODO: Refactor this for specific Input Registration.
        auto* pWorld = GetOwner()->GetWorld();
        EventHandler handler;
        handler.m_callback = [this](Event& event) -> void
        {
             OnEvent(event);  
        };
        pWorld->RegisterEventHandler(handler);
    }

    void FreeCamMovementComponent::OnDisabled()
    {
        TickableEntity3DComponent::OnDisabled();
        
        // [TODO]: Unregister for Events.
    }

    void FreeCamMovementComponent::ProcessInput()
    {
        m_inputMovement = Vec3::Zero();
        m_inputRotation = Vec2::Zero();

        // Process Movement:
        if (nes::InputManager::IsKeyDown(nes::EKeyCode::W))
            m_inputMovement.z += 1.f;
            
        if (nes::InputManager::IsKeyDown(nes::EKeyCode::S))
            m_inputMovement.z -= 1.f;
            
        if (nes::InputManager::IsKeyDown(nes::EKeyCode::A))
            m_inputMovement.x -= 1.f;
        
        if (nes::InputManager::IsKeyDown(nes::EKeyCode::D))
            m_inputMovement.x += 1.f;
        
        if (nes::InputManager::IsKeyDown(nes::EKeyCode::Space))
            m_inputMovement.y += 1.f;  
            
        if (nes::InputManager::IsKeyDown(nes::EKeyCode::LeftControl) || nes::InputManager::IsKeyDown(nes::EKeyCode::RightControl))
            m_inputMovement.y -= 1.f;

        // Normalize movement vector
        m_inputMovement.NormalizedOr(Vec3::Zero());
        
        // Process Rotation:
        if (m_rotationEnabled)
        {
            const Vec2 delta = nes::InputManager::GetCursorDelta();
            m_inputRotation.x = delta.y;
            m_inputRotation.y = delta.x;
            m_inputRotation.Normalize();
        }
    }

    void FreeCamMovementComponent::OnEvent(Event& event)
    {
        if (event.GetEventID() == nes::MouseButtonEvent::GetStaticEventID())
        {
            auto& mouseButtonEvent = checked_cast<nes::MouseButtonEvent&>(event);
            if (mouseButtonEvent.GetButton() == nes::EMouseButton::Right)
            {
                if (mouseButtonEvent.GetAction() == nes::EMouseAction::Pressed)
                {
                    m_rotationEnabled = true;
                    nes::InputManager::SetCursorMode(nes::ECursorMode::Disabled);
                }
                    
                else if (mouseButtonEvent.GetAction() == nes::EMouseAction::Released)
                {
                    m_rotationEnabled = false;
                    nes::InputManager::SetCursorMode(nes::ECursorMode::Visible);
                }
            }
        }
    }

    void FreeCamMovementComponent::Tick(const float deltaTime)
    {
        //NES_LOG("Ticked. Delta Time: ", deltaTime);
        ProcessInput();
        const Vec3 deltaPitchYawRoll = Vec3(m_inputRotation.x * m_turnSpeedPitch, m_inputRotation.y * m_turnSpeedYaw, 0.f) * deltaTime;
        const Vec3 deltaMovement = m_inputMovement * (m_moveSpeed * deltaTime);
        
        Entity3D* pOwner = GetOwner();
        if (deltaPitchYawRoll.LengthSqr() > 0.f || deltaMovement.LengthSqr() > 0.f)
        {
            // Apply Rotation:
            Rotation localRotation = pOwner->GetLocalRotation();
            if (deltaPitchYawRoll.LengthSqr() > 0.f)
            {
                const Rotation deltaRotation(deltaPitchYawRoll);
                localRotation += deltaRotation;
            }

            // Translation:
            // - Add the Delta XZ movement in our local orientation.
            // - Add the Delta Y movement on the world Y axis.
            Vec3 localLocation = pOwner->GetLocalLocation();
            localLocation += localRotation.RotatedVector(Vec3(deltaMovement.x, 0.f, deltaMovement.z));
            localLocation.y += deltaMovement.y;

            // Set the new Transform
            pOwner->SetLocalTransform(localLocation, localRotation, Vec3::One());
        }
    }
}
