// DayNightSystem.cpp
#include "DayNightSystem.h"

#include "Components/LightComponents.h"
#include "Nessie/Input/Cursor.h"
#include "Nessie/Input/InputEvents.h"
#include "Nessie/Input/InputManager.h"
#include "Nessie/World/ComponentSystems/TransformSystem.h"

namespace pbr
{
    void DayNightSimComponent::Serialize(YAML::Emitter&, const DayNightSimComponent&)
    {
        // [TODO]: 
    }

    void DayNightSimComponent::Deserialize(const YAML::Node& in, DayNightSimComponent& component)
    {
        component.m_dayDuration = in["DayDuration"].as<float>(20.f);
        component.m_sunMaxLux = in["SunMaxLux"].as<float>(120'000.f);

        // Day Color
        {
            const auto& colorNode = in["DayColor"];
            component.m_dayColor.x = colorNode[0].as<float>(1.f);
            component.m_dayColor.y = colorNode[1].as<float>(1.f);
            component.m_dayColor.z = colorNode[2].as<float>(0.95f);
        }

        // Night Color
        {
            const auto& colorNode = in["NightColor"];
            component.m_nightColor.x = colorNode[0].as<float>(1.f);
            component.m_nightColor.y = colorNode[1].as<float>(0.6f);
            component.m_nightColor.z = colorNode[2].as<float>(0.3f);
        }
    }

    void DayNightSystem::RegisterComponentTypes()
    {
        NES_REGISTER_COMPONENT(DayNightSimComponent);
        NES_REGISTER_COMPONENT(DirectionalLightComponent);
    }

    void DayNightSystem::Tick(const float deltaTime)
    {
        if (!m_shouldSimulate)
            return;
        
        auto& registry = GetRegistry();

        m_accumulatedTime += deltaTime;

        auto view = registry.GetAllEntitiesWith<DirectionalLightComponent, DayNightSimComponent>();
        for (auto entity : view)
        {
            auto& dayNightSimComp = view.get<DayNightSimComponent>(entity);
            auto& directionalLight = view.get<DirectionalLightComponent>(entity);
        
            const float dayProgress = nes::math::ModF(m_accumulatedTime, dayNightSimComp.m_dayDuration) / dayNightSimComp.m_dayDuration;
            const float angle = dayProgress * 2.f * nes::math::Pi(); // [0, 2pi]

            // Horizontal rotation (east to west)
            const float azimuth = angle;
    
            // Vertical arc (sun goes below horizon)
            const float elevation = (nes::math::Sin(angle) - 0.2f) * 1.2f;

            // Convert spherical coordinates to direction vector
            directionalLight.m_direction = nes::Vec3
            (
                nes::math::Cos(azimuth) * nes::math::Cos(elevation)
                , -nes::math::Sin(elevation)
                ,nes::math::Sin(azimuth) * nes::math::Cos(elevation)
            );
            directionalLight.m_direction.Normalize();

            // Calculate Intensity based on height
            const float sunHeight = -directionalLight.m_direction.y; // 1.0 = straight down, -1.0 = straight up
            directionalLight.m_intensity = nes::math::ClampNormalized(sunHeight * 2.f) * dayNightSimComp.m_sunMaxLux;
    
            // Add Warmer colors at sunrise/sunset.
            const float horizonFactor = 1.0f - nes::math::Abs(sunHeight);
            const nes::Vec3 color = nes::Vec3::Lerp(dayNightSimComp.m_dayColor, dayNightSimComp.m_nightColor, horizonFactor);
            directionalLight.m_color = nes::LinearColor(color.x, color.y, color.z);
        }
    }

    void DayNightSystem::OnEvent(nes::Event& e)
    {
        // Pressing 'P' will toggle sun simulation.
        if (auto* pKeyEvent = e.Cast<nes::KeyEvent>())
        {
            if (pKeyEvent->GetKeyCode() == nes::EKeyCode::P && pKeyEvent->GetAction() == nes::EKeyAction::Pressed)
            {
                m_shouldSimulate = !m_shouldSimulate;
            }
        }
    }
}
