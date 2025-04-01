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
        pWorld->RegisterTickToWorldTickGroup(&m_tickFunction, TickStage::PrePhysics);
        
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
        m_inputMovement = Vector3::GetZeroVector();
        m_inputRotation = Vector2::GetZeroVector();

        // Process Movement:
        if (nes::InputManager::IsKeyDown(nes::KeyCode::W))
            m_inputMovement.z += 1.f;
            
        if (nes::InputManager::IsKeyDown(nes::KeyCode::S))
            m_inputMovement.z += -1.f;
            
        if (nes::InputManager::IsKeyDown(nes::KeyCode::A))
            m_inputMovement.x += -1.f;
        
        if (nes::InputManager::IsKeyDown(nes::KeyCode::D))
            m_inputMovement.x += 1.f;
        
        if (nes::InputManager::IsKeyDown(nes::KeyCode::Space))
            m_inputMovement.y += 1.f;    
            
        if (nes::InputManager::IsKeyDown(nes::KeyCode::LeftControl) || nes::InputManager::IsKeyDown(nes::KeyCode::RightControl))
            m_inputMovement.y += -1.f;

        // Normalize movement vector;
        m_inputMovement.Normalize();
        
        // Process Rotation:
        if (m_rotationEnabled)
        {
            const nes::Vector2 mouseDelta = nes::InputManager::GetCursorDelta();
            m_inputRotation.y = mouseDelta.x; // Yaw
            m_inputRotation.x = mouseDelta.y; // Pitch
        
            m_inputRotation = m_inputRotation.Normalize();
        }
    }

    void FreeCamMovementComponent::OnEvent(Event& event)
    {
        if (event.GetEventID() == nes::MouseButtonEvent::GetStaticEventID())
        {
            auto& mouseButtonEvent = checked_cast<nes::MouseButtonEvent&>(event);
            if (mouseButtonEvent.GetButton() == nes::MouseButton::Right)
            {
                if (mouseButtonEvent.GetAction() == nes::MouseAction::Pressed)
                {
                    m_rotationEnabled = true;
                    nes::InputManager::SetCursorMode(nes::CursorMode::Disabled);
                }
                    
                else if (mouseButtonEvent.GetAction() == nes::MouseAction::Released)
                {
                    m_rotationEnabled = false;
                    nes::InputManager::SetCursorMode(nes::CursorMode::Visible);
                }
            }
        }
    }

    void FreeCamMovementComponent::Tick(const float deltaTime)
    {
        //NES_LOG("Ticked. Delta Time: ", deltaTime);
        ProcessInput();
        const Vector3 deltaPitchYawRoll = Vector3(m_inputRotation.x * m_turnSpeedPitch, m_inputRotation.y * m_turnSpeedYaw, 0.f) * deltaTime;
        const Vector3 deltaMovement = m_inputMovement * (m_moveSpeed * deltaTime);
        
        Entity3D* pOwner = GetOwner();
        if (deltaPitchYawRoll.SquaredMagnitude() > 0.f || deltaMovement.SquaredMagnitude() > 0.f)
        {
            // Apply Rotation:
            Rotation localRotation = pOwner->GetLocalRotation();
            if (deltaPitchYawRoll.SquaredMagnitude() > 0.f)
            {
                const Rotation deltaRotation(deltaPitchYawRoll);
                localRotation += deltaRotation;
            }

            // Translation:
            // - Add the Delta XZ movement in our local orientation.
            // - Add the Delta Y movement on the world Y axis.
            Vector3 localLocation = pOwner->GetLocalLocation();
            localLocation += localRotation.RotatedVector(Vector3(deltaMovement.x, 0.f, deltaMovement.z));
            localLocation.y += deltaMovement.y;

            // Set the new Transform
            pOwner->SetLocalTransform(localLocation, localRotation, Vector3::GetUnitVector());
        }
    }
}
