// CameraComponent.cpp
#include "CameraComponent.h"

namespace nes
{
    Mat44 CameraComponent::CalculateProjectionMatrix(const uint32 width, const uint32 height, const bool flipAxis) const
    {
        Mat44 projection;
        
        if (m_projectionType == EProjectionType::Perspective)
        {
            projection = Mat44::Perspective(m_perspectiveFOV, static_cast<float>(width), static_cast<float>(height), m_nearPlane, m_farPlane);
        }
        else
        {
            const float aspect = static_cast<float>(width) / static_cast<float>(height);
            const float orthoHalfHeight = m_orthographicSize * 0.5f;
            const float orthoHalfWidth = orthoHalfHeight * aspect;
            projection = Mat44::Orthographic(-orthoHalfWidth, orthoHalfWidth, -orthoHalfHeight, orthoHalfHeight, m_nearPlane, m_farPlane);
        }
        
        if (flipAxis)
            projection[1][1] *= -1.f;

        return projection;
    }

    float CameraComponent::CalculateExposureFactor() const
    {
        // Calculate exposure value (EV) - standard photographic formula
        const float ev = std::log2((m_aperture * m_aperture) / m_shutterSpeed);

        // Convert to exposure adjustment factor
        // ISO 100 is baseline, higher ISO = more sensitive (brighter)
        const float isoAdjustment = m_iso / 100.f;

        // Exposure factor combines EV and ISO
        return isoAdjustment * std::pow(2.f, -ev);
    }

    void CameraComponent::Serialize(YAML::Emitter&, const CameraComponent&)
    {
        // [TODO]: 
    }

    void CameraComponent::Deserialize(const YAML::Node& in, CameraComponent& component)
    {
        component.m_nearPlane = in["NearPlane"].as<float>(0.1f);
        component.m_farPlane = in["FarPlane"].as<float>(1000.f);
        component.m_perspectiveFOV = in["PerspectiveFOV"].as<float>(60.f) * math::DegreesToRadians();
        component.m_orthographicSize = in["OrthographicSize"].as<float>(10.f);
        component.m_aperture = in["Aperture"].as<float>(8.f);
        component.m_shutterSpeed = 1.f / in["ShutterSpeed"].as<float>(125.f);
        component.m_iso = in["ISO"].as<float>(100.f);
        component.m_projectionType = static_cast<EProjectionType>(in["ProjectionType"].as<uint8>(static_cast<uint8>(0)));
    }
}
