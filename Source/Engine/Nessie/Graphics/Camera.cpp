// Camera.cpp

#include "Camera.h"

#include "Math/VectorConversions.h"

namespace nes
{
    //----------------------------------------------------------------------------------------------------
    ///		@brief : Set the Camera's View Matrix to look at a target position.
    ///		@param eyePosition : The position of the viewer (the camera itself).
    ///		@param targetPosition : The target position to look at.
    ///		@param up : The basis up direction. Which direction is up when looking at the target?
    //----------------------------------------------------------------------------------------------------
    void Camera::LookAt(const nes::Vector3& eyePosition, const nes::Vector3& targetPosition, const nes::Vector3& up)
    {
        m_viewMatrix = math::LookAt(eyePosition, targetPosition, up);
    }

    //----------------------------------------------------------------------------------------------------
    ///		@brief : Update the Camera's Projection Matrix based on the new viewport. 
    //----------------------------------------------------------------------------------------------------
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

    //----------------------------------------------------------------------------------------------------
    ///		@brief : Create a Camera that uses single-point perspective. This is the standard viewing for most
    ///             3D applications.
    ///		@param fovRadians : The Field of View, expressed in radians. Should be ~[45-120] degrees.
    ///		@param aspectRatio : The aspect ratio of the view port (width/height).
    ///		@param near : The near plane distance from the view position. 0.1f is a reasonable default.
    ///		@param far : The far plane distance from the view position. 1000.f is a reasonable default.
    ///		@param flipYAxis : If you need to flip the Y-axis.
    //----------------------------------------------------------------------------------------------------
    void Camera::SetPerspective(const float fovRadians, const float aspectRatio, const float near,
                                const float far, const bool flipYAxis)
    {
        m_projectionType = ProjectionType::Perspective;
        m_perspectiveFOV = fovRadians;
        m_perspectiveNear = near;
        m_perspectiveFar = far;
        m_projectionMatrix = math::PerspectiveFOV(fovRadians, aspectRatio, near, far);
        if (flipYAxis)
            m_projectionMatrix[1][1] *= -1.f;
    }

    void Camera::SetPerspectiveFOV(const float fovRadians)
    {
        m_perspectiveFOV = fovRadians;
    }

    void Camera::SetPerspectiveNearPlane(const float nearPlane)
    {
        m_perspectiveNear = nearPlane;
    }

    void Camera::SetPerspectiveFarPlane(const float farPlane)
    {
        m_perspectiveFar = farPlane;
    }

    //----------------------------------------------------------------------------------------------------
    ///		@brief : Calculates the Camera's view location in world space based on the current view matrix.
    ///             This is an expensive call because we have to calculate the inverse of the view matrix,
    ///             so this should be done sparingly.
    //----------------------------------------------------------------------------------------------------
    Vector3 Camera::CameraViewLocation() const
    {
        Mat4 inverse;
        [[maybe_unused]] const bool result = m_viewMatrix.TryGetInverse(inverse);
        NES_ASSERT(result);

        return math::XYZ(inverse.GetColumn(3));
    }

    //----------------------------------------------------------------------------------------------------
    ///		@brief : Create a Camera that uses single-point perspective. This is the standard viewing for most
    ///             3D applications.
    ///		@param fovRadians : The Field of View, expressed in radians. Should be ~[45-120] degrees.
    ///		@param viewWidth : The width of the view port.
    ///		@param viewHeight : The height of the view port.
    ///		@param near : The near plane distance from the view position. 0.1f is a reasonable default.
    ///		@param far : The far plane distance from the view position. 1000.f is a reasonable default.
    ///		@param flipYAxis : If you need to flip the Y-axis.
    //----------------------------------------------------------------------------------------------------
    void Camera::SetPerspective(const float fovRadians, const uint32_t viewWidth, const uint32_t viewHeight, const float near,
        const float far, const bool flipYAxis)
    {
        SetPerspective(fovRadians, static_cast<float>(viewWidth) / static_cast<float>(viewHeight), near, far, flipYAxis);
    }

    //----------------------------------------------------------------------------------------------------
    //		NOTES:
    //		
    ///		@brief : Create a camera that preserves parallel lines. This is primarily used in 2D rendering,
    ///         (HUDs, Menus, etc.) and isometric views.
    ///		@param viewWidth : The width of the view port.
    ///		@param viewHeight : The height of the view port.
    ///		@param orthographicSize : The size of orthographic projection.
    ///		@param near : The near plane distance from the view position. 0.1f is a reasonable default.
    ///		@param far : The far plane distance from the view position. 1000.f is a reasonable default.
    ///		@param flipYAxis : If you need to flip the Y-axis.
    //----------------------------------------------------------------------------------------------------
    void Camera::SetOrthographic(const uint32_t viewWidth, const uint32_t viewHeight, const float orthographicSize,
        const float near, const float far, const bool flipYAxis)
    {
        m_projectionType = ProjectionType::Orthographic;
        m_orthographicSize = orthographicSize;
        m_orthographicNear = near;
        m_orthographicFar = far;
        UpdateViewport(viewWidth, viewHeight, flipYAxis);
    }

    void Camera::SetOrthographicSize(const float size)
    {
        m_orthographicSize = size;
    }

    void Camera::SetOrthographicNearPlane(const float near)
    {
        m_orthographicNear = near;
    }

    void Camera::SetOrthographicFarPlane(const float far)
    {
        m_orthographicFar = far;
    }
}
