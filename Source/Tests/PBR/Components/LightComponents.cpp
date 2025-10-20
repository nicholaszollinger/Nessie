// LightComponents.cpp
#include "LightComponents.h"

namespace pbr
{
    void PointLightComponent::Serialize(YAML::Emitter&, const PointLightComponent&)
    {
        // [TODO]: 
    }

    void PointLightComponent::Deserialize(const YAML::Node& in, PointLightComponent& component)
    {
        component.m_intensity = in["Intensity"].as<float>(600.f);
        component.m_radius = in["Radius"].as<float>(30.f);

        // Color:
        {
            auto colorNode = in["Color"];
            component.m_color.r = colorNode[0].as<float>(1.f);
            component.m_color.g = colorNode[1].as<float>(1.f);
            component.m_color.b = colorNode[2].as<float>(1.f);
            component.m_color.a = 1.f;
        }
    }

    void DirectionalLightComponent::Serialize(YAML::Emitter&, const DirectionalLightComponent&)
    {
        // [TODO]: 
    }

    void DirectionalLightComponent::Deserialize(const YAML::Node& in, DirectionalLightComponent& component)
    {
        component.m_intensity = in["Intensity"].as<float>(100'000.f); // 100K lux by default.
        
        // Direction
        {
            auto directionNode = in["Direction"];
            component.m_direction.x = directionNode[0].as<float>(1.f);
            component.m_direction.y = directionNode[1].as<float>(-1.f);
            component.m_direction.z = directionNode[2].as<float>(1.f);
            component.m_direction.Normalize();
        }

        // Color
        {
            auto colorNode = in["Color"];
            component.m_color.r = colorNode[0].as<float>(1.f);
            component.m_color.g = colorNode[1].as<float>(1.f);
            component.m_color.b = colorNode[2].as<float>(1.f);
            component.m_color.a = 1.f;
        }
    }
}
