// FreeCamMovementComponent.cpp
#include "FreeCamMovementComponent.h"

#include "Input/InputManager.h"
#include "Math/Quaternion.h"
#include "Scene/Scene.h"
#include "World/Entity3D.h"

namespace nes
{
    bool FreeCamMovementComponent::Init()
    {
        return true;
    }

    void FreeCamMovementComponent::OnEnabled()
    {
        TickFunction tick;
        tick.m_function = [this](const float deltaTime) -> void
        {
            ProcessCameraMovement(deltaTime);            
        };

        auto* pScene = GetOwner()->GetScene();
        pScene->RegisterTickFunction(tick);

        EventHandler handler;
        handler.m_callback = [this](Event& event) -> void
        {
            OnEvent(event);  
        };
        pScene->RegisterEventHandler(handler);
    }

    void FreeCamMovementComponent::OnDisabled()
    {
        // [TODO]: Unregister for Tick.
        // Right now I am punting on this, no ticks will be unregistered
        // in the lifetime of the application for now.
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

    void FreeCamMovementComponent::ProcessCameraMovement(const float deltaTime)
    {
        ProcessInput();
        const Vector3 deltaPitchYawRoll = Vector3(m_inputRotation.x * m_turnSpeedPitch, m_inputRotation.y * m_turnSpeedYaw, 0.f) * deltaTime;
        const Vector3 deltaMovement = m_inputMovement * (m_moveSpeed * deltaTime);

        // If input was detected:
        if (deltaPitchYawRoll.SquaredMagnitude() > 0.f && deltaMovement.SquaredMagnitude() > 0.f)
        {
            Entity3D* pOwner = GetOwner();
            
            // Apply Rotation:
            Quat localOrientation = pOwner->GetLocalOrientation();
            localOrientation = Quat::MakeFromEuler(deltaPitchYawRoll) * localOrientation;

            // Translation:
            // - Add the Delta XZ movement in our local orientation.
            // - Add the Delta Y movement on the world Y axis.
            Vector3 localLocation = pOwner->GetLocalLocation();
            localLocation += localOrientation.RotatedVector(Vector3(deltaMovement.x, 0.f, deltaMovement.z));
            localLocation.y += deltaMovement.y;

            // Set the new Transform
            pOwner->SetLocalTransform(localLocation, localOrientation, Vector3::GetUnitVector());
        }
    }
}
