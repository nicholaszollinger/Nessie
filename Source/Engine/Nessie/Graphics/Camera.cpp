// Camera.cpp
#include "Camera.h"

namespace nes
{
    Mat44 Camera::CalculateProjectionMatrix(const uint32 width, const uint32 height, const bool flipAxis) const
    {
        Mat44 projection;
        
        if (m_projectionType == EProjectionType::Perspective)
        {
            projection = Mat44::Perspective(math::ToRadians(m_perspectiveFOV), static_cast<float>(width), static_cast<float>(height), m_nearPlane, m_farPlane);
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

    float Camera::CalculateExposureFactor() const
    {
        // Calculate exposure value (EV) - standard photographic formula
        const float ev = std::log2((m_aperture * m_aperture) / (1.f / m_shutterSpeed));

        // Convert to exposure adjustment factor
        // ISO 100 is baseline, higher ISO = more sensitive (brighter)
        const float isoAdjustment = m_iso / 100.f;

        // Exposure factor combines EV and ISO
        return isoAdjustment * std::pow(2.f, -ev);
    }
}
