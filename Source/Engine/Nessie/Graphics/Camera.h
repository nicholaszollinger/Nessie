// Camera.h
#pragma once
#include "Math/Matrix.h"

namespace nes
{
    //----------------------------------------------------------------------------------------------------
    //		NOTES:
    //      - The Camera's world position can be calculated as the last column of the inverse view matrix. 
    //		
    ///		@brief : Camera class that contains a Projection and View Matrix for rendering.
    //----------------------------------------------------------------------------------------------------
    class Camera
    {
    public:
        enum ProjectionType : uint8_t
        {
            Perspective = 0,
            Orthographic,
        };

    private:
        Mat4 m_projectionMatrix = Mat4::Identity();
        Mat4 m_viewMatrix = Mat4::Identity();
        float m_perspectiveFOV = 60.f;
        float m_perspectiveNear = 0.01f;
        float m_perspectiveFar = 1000.f;
        float m_orthographicSize = 10.f;
        float m_orthographicNear = -1.f;
        float m_orthographicFar = 1.f;
        ProjectionType m_projectionType = ProjectionType::Perspective;
        
    public:
        Camera() = default;
        
        void LookAt(const nes::Vector3& eyePosition, const nes::Vector3& targetPosition, const nes::Vector3& up);
        void UpdateViewport(const uint32_t width, const uint32_t height, const bool flipYAxis = true);
        void SetProjectionType(const ProjectionType type)       { m_projectionType = type; }
        void SetViewMatrix(const Mat4& viewMatrix)              { m_viewMatrix = viewMatrix; }
        [[nodiscard]] Vector3 CameraViewLocation()  const;

        // Perspective
        void SetPerspective(const float fovRadians, const uint32_t viewWidth, const uint32_t viewHeight, const float near, const float far, const bool flipYAxis = true);
        void SetPerspective(const float fovRadians, const float aspectRatio, const float near, const float far, const bool flipYAxis = true);
        void SetPerspectiveFOV(const float fovRadians);
        void SetPerspectiveNearPlane(const float nearPlane);
        void SetPerspectiveFarPlane(const float farPlane);

        // Orthographic:
        void SetOrthographic(const uint32_t viewWidth, const uint32_t viewHeight, const float orthographicSize, const float near = -1.0, const float far = 1.0, const bool flipYAxis = true);
        void SetOrthographicSize(const float size);
        void SetOrthographicNearPlane(const float near);
        void SetOrthographicFarPlane(const float far);
        
        [[nodiscard]] const Mat4& GetProjectionMatrix()   const { return m_projectionMatrix; }
        [[nodiscard]]       Mat4& GetProjectionMatrix()         { return m_projectionMatrix; }
        [[nodiscard]] const Mat4& GetViewMatrix()         const { return m_viewMatrix; }
        [[nodiscard]]       Mat4& GetViewMatrix()               { return m_viewMatrix; }
        [[nodiscard]] Mat4  GetViewProjectionMatrix()     const { return m_projectionMatrix * m_viewMatrix; }
        [[nodiscard]] ProjectionType GetProjectionType()  const { return m_projectionType; }

        [[nodiscard]] float GetPerspectiveFOV()     const { return m_perspectiveFOV; }
        [[nodiscard]] float GetPerspectiveNear()    const { return m_perspectiveNear; }
        [[nodiscard]] float GetPerspectiveFar()     const { return m_perspectiveFar; }
        [[nodiscard]] float GetOrthographicSize()   const { return m_orthographicSize; }
        [[nodiscard]] float GetOrthographicNear()   const { return m_orthographicNear; }
        [[nodiscard]] float GetOrthographicFar()    const { return m_orthographicFar; }
    };
}