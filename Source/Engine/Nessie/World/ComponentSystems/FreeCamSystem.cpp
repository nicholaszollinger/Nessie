// FreeCamSystem.cpp
#include "FreeCamSystem.h"
#include "TransformSystem.h"
#include "Nessie/World.h"
#include "Nessie/Input/InputManager.h"

namespace nes
{
    bool FreeCamSystem::Init()
    {
        m_pTransformSystem = GetWorld().GetSystem<TransformSystem>();
        if (!m_pTransformSystem)
        {
            NES_ERROR("Failed to setup FreeCamSystem! No Transform System present!");
            return false;
        }

        return true;
    }

    void FreeCamSystem::Shutdown()
    {
        m_pTransformSystem = nullptr;
    }

    void FreeCamSystem::RegisterComponentTypes()
    {
        NES_REGISTER_COMPONENT(TransformComponent);
        NES_REGISTER_COMPONENT(FreeCamMovementComponent);
    }

    void FreeCamSystem::OnEvent(Event& event)
    {
        // When right click is down, allow camera turning.
        if (auto* pMouseButtonEvent = event.Cast<nes::MouseButtonEvent>())
        {
            if (pMouseButtonEvent->GetButton() == nes::EMouseButton::Right)
            {
                if (pMouseButtonEvent->GetAction() == nes::EMouseAction::Pressed)
                {
                    m_rotationEnabled = true;
                    nes::InputManager::SetCursorMode(nes::ECursorMode::Disabled);
                }
                    
                else if (pMouseButtonEvent->GetAction() == nes::EMouseAction::Released)
                {
                    m_rotationEnabled = false;
                    nes::InputManager::SetCursorMode(nes::ECursorMode::Visible);
                }
            }
        }
    }

    void FreeCamSystem::Tick(const float deltaTime) const
    {
        auto view = GetRegistry().GetAllEntitiesWith<TransformComponent, FreeCamMovementComponent>(entt::exclude<DisabledComponent>);

        Vec3 inputMovement;
        Vec2 inputRotation;
        bool shift;
        ProcessInput(inputMovement, inputRotation, shift);
        
        for (auto entity : view)
        {
            auto& transform = view.get<TransformComponent>(entity);
            auto& freeCam = view.get<FreeCamMovementComponent>(entity);

            // Speed:
            float speed = freeCam.m_moveSpeed * deltaTime;
            if (shift)
                speed *= 2.f;

            // Delta Rotation:
            const float heading = inputRotation.y * freeCam.m_sensitivity;
            const float pitch = nes::math::Clamp(inputRotation.x * freeCam.m_sensitivity, -0.49f * nes::math::Pi(), 0.49f * nes::math::Pi());
            const nes::Vec3 deltaPitchYawRoll = nes::Vec3(pitch, heading, 0.f);

            // Delta Movement.
            const nes::Vec3 deltaMovement = inputMovement * speed;

            // If there is enough change:
            if (deltaPitchYawRoll.LengthSqr() > 0.f || deltaMovement.LengthSqr() > 0.f)
            {
                // calculate local rotation:
                nes::Rotation localRotation = transform.GetLocalRotation();
                if (deltaPitchYawRoll.LengthSqr() > 0.f)
                {
                    const nes::Rotation deltaRotation(deltaPitchYawRoll);
                    localRotation += deltaRotation;
                }

                // Calculate local position:
                nes::Vec3 localPosition = transform.GetLocalPosition();
                localPosition += localRotation.RotatedVector(nes::Vec3(deltaMovement.x, 0.f, deltaMovement.z));
                localPosition.y += deltaMovement.y;

                // Apply the transformation:
                m_pTransformSystem->SetLocalTransform(entity, localPosition, localRotation, transform.GetLocalScale());
            }
        }
    }

    void FreeCamSystem::ProcessInput(Vec3& outInputMovement, Vec2& outInputRotation, bool& outShiftDown) const
    {
        outShiftDown = nes::InputManager::IsKeyDown(nes::EKeyCode::LeftShift) || nes::InputManager::IsKeyDown(nes::EKeyCode::RightShift);
        
        outInputMovement = nes::Vec3::Zero();
        outInputRotation = nes::Vec2::Zero();

        // Process Movement:
        if (nes::InputManager::IsKeyDown(nes::EKeyCode::W))
            outInputMovement.z += 1.f;
            
        if (nes::InputManager::IsKeyDown(nes::EKeyCode::S))
            outInputMovement.z -= 1.f;
            
        if (nes::InputManager::IsKeyDown(nes::EKeyCode::A))
            outInputMovement.x -= 1.f;
        
        if (nes::InputManager::IsKeyDown(nes::EKeyCode::D))
            outInputMovement.x += 1.f;
        
        if (nes::InputManager::IsKeyDown(nes::EKeyCode::Space))
            outInputMovement.y += 1.f;  
            
        if (nes::InputManager::IsKeyDown(nes::EKeyCode::LeftControl) || nes::InputManager::IsKeyDown(nes::EKeyCode::RightControl))
            outInputMovement.y -= 1.f;

        // Normalize movement vector
        outInputMovement.NormalizedOr(nes::Vec3::Zero());

        // Process Rotation:
        if (m_rotationEnabled)
        {
            const nes::Vec2 delta = nes::InputManager::GetCursorDelta();
            outInputRotation.x = delta.y;
            outInputRotation.y = delta.x;
            outInputRotation.Normalize();
        }
    }
}
