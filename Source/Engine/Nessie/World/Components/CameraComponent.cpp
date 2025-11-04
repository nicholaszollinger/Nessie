// CameraComponent.cpp
#include "CameraComponent.h"

namespace nes
{
    void CameraComponent::Serialize(YAML::Emitter&, const CameraComponent&)
    {
        // [TODO]: 
    }

    void CameraComponent::Deserialize(const YAML::Node& in, CameraComponent& component)
    {
        component.m_camera.m_nearPlane = in["NearPlane"].as<float>(0.1f);
        component.m_camera.m_farPlane = in["FarPlane"].as<float>(1000.f);
        component.m_camera.m_perspectiveFOV = in["PerspectiveFOV"].as<float>(60.f);
        component.m_camera.m_orthographicSize = in["OrthographicSize"].as<float>(10.f);
        component.m_camera.m_aperture = in["Aperture"].as<float>(8.f);
        component.m_camera.m_shutterSpeed = 1.f / in["ShutterSpeed"].as<float>(125.f);
        component.m_camera.m_iso = in["ISO"].as<float>(100.f);
        component.m_camera.m_projectionType = static_cast<Camera::EProjectionType>(in["ProjectionType"].as<uint8>(static_cast<uint8>(0)));
    }
}
