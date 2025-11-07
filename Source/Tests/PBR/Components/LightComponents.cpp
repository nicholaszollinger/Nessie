// LightComponents.cpp
#include "LightComponents.h"
#include "Nessie/FileIO/YAML/Serializers/YamlMathSerializers.h"

namespace pbr
{
    void PointLightComponent::Serialize(nes::YamlOutStream& out, const PointLightComponent& component)
    {
        out.Write("Color", component.m_color);
        out.Write("Intensity", component.m_intensity);
        out.Write("Radius", component.m_radius);
    }

    void PointLightComponent::Deserialize(const nes::YamlNode& in, PointLightComponent& component)
    {
        in["Color"].Read(component.m_color, nes::LinearColor::White());
        in["Intensity"].Read(component.m_intensity, 600.f);
        in["Radius"].Read(component.m_radius, 30.f);
    }

    void DirectionalLightComponent::Serialize(nes::YamlOutStream& out, const DirectionalLightComponent& component)
    {
        out.Write("Color", component.m_color);
        out.Write("Direction", component.m_direction);
        out.Write("Intensity", component.m_intensity);
    }

    void DirectionalLightComponent::Deserialize(const nes::YamlNode& in, DirectionalLightComponent& component)
    {
        in["Color"].Read(component.m_color, nes::LinearColor::White());
        in["Direction"].Read(component.m_direction, nes::Vec3(1.f, -1.f, 1.f));
        component.m_direction.Normalize();
        in["Intensity"].Read(component.m_intensity, 100'000.f); // 100K lux by default.
    }
}
