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
        auto* pRegistry = GetEntityRegistry();
        if (!pRegistry)
            return;
        
        auto view = pRegistry->GetAllEntitiesWith<TransformComponent, FreeCamMovementComponent>(entt::exclude<DisabledComponent>);

        Vec3 inputMovement = Vec3::Zero();
        Vec2 inputRotation = Vec2::Zero();
        bool shift = false;
        ProcessInput(inputMovement, inputRotation, shift);
        const float speedModifier = shift? 2.f : 1.f;
        
        for (auto entity : view)
        {
            auto& transform = view.get<TransformComponent>(entity);
            auto& freeCam = view.get<FreeCamMovementComponent>(entity);

            // Speed:
            const float speed = freeCam.m_moveSpeed * speedModifier * deltaTime;

            // Get the current forward and up vectors.
            auto worldTransform = transform.GetWorldTransformMatrix();
            Vec3 forward = worldTransform.GetForward();
            const Vec3 right = forward.Cross(Vec3::Up()).Normalized();
            
            // Calculate the new position:
            Vec3 newPosition = transform.GetLocalPosition();
            newPosition -= (right * speed * inputMovement.x);
            newPosition += (forward * speed * inputMovement.z);
            newPosition.y += (speed * inputMovement.y); // Up and down applied in world space.

            // Rotation:
            Rotation deltaRotation = Rotation::Zero();
            if (m_rotationEnabled)
            {
                // Delta Rotation:
                const float pitch = math::ToRadians(inputRotation.x  * freeCam.m_sensitivity);
                const float yaw = math::ToRadians(inputRotation.y * freeCam.m_sensitivity);
                deltaRotation = Rotation(math::ToDegrees(pitch), math::ToDegrees(yaw), 0.f);

                // Current Rotation:
                const auto oldRotation = transform.GetWorldRotation();
                
                // Clamp the delta pitch to prevent gimbal lock for the final rotation:
                const float maxNewPitch = math::Clamp(oldRotation.m_pitch + deltaRotation.m_pitch, -90.f + math::PrecisionDelta(), 90.f - math::PrecisionDelta());
                deltaRotation.m_pitch = maxNewPitch - oldRotation.m_pitch;
            }

            if (inputMovement.LengthSqr() > 0.f)
            {
                m_pTransformSystem->SetLocalPosition(entity, newPosition);
            }

            if (deltaRotation != Rotation::Zero())
            {
                m_pTransformSystem->RotateWorld(entity, deltaRotation);
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
        
        if (InputManager::IsKeyDown(EKeyCode::E))
            outInputMovement.y += 1.f;  
            
        if (InputManager::IsKeyDown(EKeyCode::Q))
            outInputMovement.y -= 1.f;

        // Normalize movement vector
        outInputMovement.NormalizedOr(Vec3::Zero());

        // Process Rotation:
        if (m_rotationEnabled)
        {
            const Vec2 delta = InputManager::GetCursorDelta();
            outInputRotation.x = delta.y;
            outInputRotation.y = delta.x;
        }
    }
}
