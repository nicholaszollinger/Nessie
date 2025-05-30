// Camera.cpp
#include "Camera.h"
#include "Math/VectorConversions.h"

namespace nes
{
    void Camera::LookAt(const nes::Vector3& eyePosition, const nes::Vector3& targetPosition, const nes::Vector3& up)
    {
        m_viewMatrix = math::LookAt(eyePosition, targetPosition, up);
    }
    
    void Camera::UpdateViewport(const uint32_t width, const uint32_t height, const bool flipYAxis)
    {
        switch (m_projectionType)
        {
            case Perspective:
            {
                m_projectionMatrix = math::PerspectiveFOV(m_perspectiveFOV, static_cast<float>(width), static_cast<float>(height), m_perspectiveNear, m_perspectiveFar);
                break;
            }

            case Orthographic:
            {
                const float aspect = static_cast<float>(width) / static_cast<float>(height);
                const float orthoHalfHeight = m_orthographicSize * 0.5f;
                const float orthoHalfWidth = orthoHalfHeight * aspect;
                m_projectionMatrix = math::Orthographic(-orthoHalfWidth, orthoHalfWidth, -orthoHalfHeight, orthoHalfHeight, m_orthographicNear, m_orthographicFar);
                break;
            }
        }

        if (flipYAxis)
            m_projectionMatrix[1][1] *= -1.f;
    }
    
    void Camera::SetPerspective(const float fovRadians, const float aspectRatio, const float nearPlane,
                                const float farPlane, const bool flipYAxis)
    {
        m_projectionType = EProjectionType::Perspective;
        m_perspectiveFOV = fovRadians;
        m_perspectiveNear = nearPlane;
        m_perspectiveFar = farPlane;
        m_projectionMatrix = math::PerspectiveFOV(fovRadians, aspectRatio, nearPlane, farPlane);
        if (flipYAxis)
            m_projectionMatrix[1][1] *= -1.f;
    }
    
    Vector3 Camera::CameraViewLocation() const
    {
        Mat4 inverse;
        [[maybe_unused]] const bool result = m_viewMatrix.TryGetInverse(inverse);
        NES_ASSERT(result);

        return math::XYZ(inverse.GetColumn(3));
    }
    
    void Camera::SetPerspective(const float fovRadians, const uint32_t viewWidth, const uint32_t viewHeight, const float nearPlane,
        const float farPlane, const bool flipYAxis)
    {
        SetPerspective(fovRadians, static_cast<float>(viewWidth) / static_cast<float>(viewHeight), nearPlane, farPlane, flipYAxis);
    }
    
    void Camera::SetOrthographic(const uint32_t viewWidth, const uint32_t viewHeight, const float orthographicSize,
        const float near, const float far, const bool flipYAxis)
    {
        m_projectionType = EProjectionType::Orthographic;
        m_orthographicSize = orthographicSize;
        m_orthographicNear = near;
        m_orthographicFar = far;
        UpdateViewport(viewWidth, viewHeight, flipYAxis);
    }

    // inline void Camera::SetOrthographicSize(const float size)
    // {
    //     m_orthographicSize = size;
    // }
    //
    // inline void Camera::SetOrthographicNearPlane(const float near)
    // {
    //     m_orthographicNear = near;
    // }
    //
    // inline void Camera::SetOrthographicFarPlane(const float far)
    // {
    //     m_orthographicFar = far;
    // }
}
