// FreeCamSystem.cpp
#include "FreeCamSystem.h"
#include "TransformSystem.h"
#include "Nessie/World.h"
#include "Nessie/Input/InputManager.h"

namespace nes
{
    void FreeCamMovementComponent::Serialize(YamlOutStream& out, const FreeCamMovementComponent& component)
    {
        out.Write("MoveSpeed", component.m_moveSpeed);
        out.Write("Sensitivity", component.m_sensitivity);
    }

    void FreeCamMovementComponent::Deserialize(const YamlNode& in, FreeCamMovementComponent& component)
    {
        in["MoveSpeed"].Read(component.m_moveSpeed, 50.f);
        in["Sensitivity"].Read(component.m_sensitivity, 1.25f);
    }

    bool FreeCamSystem::Init()
    {
        m_rotationEnabled = false;
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
            const float pitch = math::Clamp(inputRotation.x * freeCam.m_sensitivity, -0.49f * math::Pi(), 0.49f * math::Pi());
            const Vec3 deltaPitchYawRoll = Vec3(pitch, heading, 0.f);

            // Delta Movement.
            const Vec3 deltaMovement = inputMovement * speed;

            // If there is enough change:
            if (deltaPitchYawRoll.LengthSqr() > 0.f || deltaMovement.LengthSqr() > 0.f)
            {
                // calculate local rotation:
                Rotation localRotation = transform.GetLocalRotation();
                if (deltaPitchYawRoll.LengthSqr() > 0.f)
                {
                    const Rotation deltaRotation(deltaPitchYawRoll);
                    localRotation += deltaRotation;
                    localRotation.m_roll = 0.f;
                }

                // Calculate local position:
                Vec3 localPosition = transform.GetLocalPosition();
                localPosition += localRotation.RotatedVector(Vec3(deltaMovement.x, 0.f, deltaMovement.z));
                localPosition.y += deltaMovement.y;

                // Apply the transformation:
                m_pTransformSystem->SetLocalTransform(entity, localPosition, localRotation, transform.GetLocalScale());
            }
        }
    }

    void FreeCamSystem::ProcessInput(Vec3& outInputMovement, Vec2& outInputRotation, bool& outShiftDown) const
    {
        outShiftDown = InputManager::IsKeyDown(EKeyCode::LeftShift) || InputManager::IsKeyDown(EKeyCode::RightShift);
        
        outInputMovement = Vec3::Zero();
        outInputRotation = Vec2::Zero();

        // Process Movement:
        if (InputManager::IsKeyDown(EKeyCode::W))
            outInputMovement.z += 1.f;
            
        if (InputManager::IsKeyDown(EKeyCode::S))
            outInputMovement.z -= 1.f;
            
        if (InputManager::IsKeyDown(EKeyCode::A))
            outInputMovement.x -= 1.f;
        
        if (InputManager::IsKeyDown(EKeyCode::D))
            outInputMovement.x += 1.f;
        
        if (InputManager::IsKeyDown(EKeyCode::Space))
            outInputMovement.y += 1.f;  
            
        if (InputManager::IsKeyDown(EKeyCode::LeftControl) || InputManager::IsKeyDown(EKeyCode::RightControl))
            outInputMovement.y -= 1.f;

        // Normalize movement vector
        outInputMovement.NormalizedOr(Vec3::Zero());

        // Process Rotation:
        if (m_rotationEnabled)
        {
            const Vec2 delta = InputManager::GetCursorDelta();
            outInputRotation.x = delta.y;
            outInputRotation.y = delta.x;
            outInputRotation.Normalize();
        }
    }
}
