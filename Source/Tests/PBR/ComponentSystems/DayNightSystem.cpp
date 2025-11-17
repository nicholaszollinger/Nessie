// DayNightSystem.cpp
#include "DayNightSystem.h"

#include "Components/LightComponents.h"
#include "Nessie/Input/Cursor.h"
#include "Nessie/Input/InputEvents.h"
#include "Nessie/Input/InputManager.h"
#include "Nessie/World/ComponentSystems/TransformSystem.h"
#include "Nessie/FileIO/YAML/Serializers/YamlMathSerializers.h"

namespace pbr
{
    void DayNightSimComponent::Serialize(nes::YamlOutStream& out, const DayNightSimComponent& component)
    {
        out.Write("DayColor", component.m_dayColor);
        out.Write("NightColor", component.m_nightColor);
        out.Write("DayDuration", component.m_dayDuration);
        out.Write("SunMaxLux", component.m_sunMaxLux);
    }

    void DayNightSimComponent::Deserialize(const nes::YamlNode& in, DayNightSimComponent& component)
    {
        in["DayColor"].Read(component.m_dayColor, nes::Vec3(1.f, 1.f, 0.95f));
        in["NightColor"].Read(component.m_nightColor, nes::Vec3(1.f, 0.6f, 0.3f));
        in["DayDuration"].Read(component.m_dayDuration, 20.f);
        in["SunMaxLux"].Read(component.m_sunMaxLux, 120'000.f);
    }

    void DayNightSystem::RegisterComponentTypes()
    {
        NES_REGISTER_COMPONENT(DayNightSimComponent);
        NES_REGISTER_COMPONENT(DirectionalLightComponent);
    }

    void DayNightSystem::Tick(const float deltaTime)
    {
        auto* pRegistry = GetEntityRegistry();
        if (!pRegistry)
            return;

        m_accumulatedTime += deltaTime;

        auto view = pRegistry->GetAllEntitiesWith<DirectionalLightComponent, DayNightSimComponent>();
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

    void DayNightSystem::OnBeginSimulation()
    {
        m_accumulatedTime = 0.f;
    }
}
