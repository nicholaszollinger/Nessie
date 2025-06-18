// Camera.h
#pragma once
#include "Math/Math.h"

namespace nes
{
    //----------------------------------------------------------------------------------------------------
    //	NOTES:
    //  - The Camera's world position can be calculated as the last column of the inverse view matrix. 
    //
    /// @brief : Camera class that contains a Projection and View Matrix for rendering.
    //----------------------------------------------------------------------------------------------------
    class Camera
    {
    public:
        enum EProjectionType : uint8_t
        {
            Perspective = 0,
            Orthographic,
        };
        
    public:
        Camera() = default;

        //----------------------------------------------------------------------------------------------------
        /// @brief : Set the Camera's View Matrix to look at a target position.
        /// @param eyePosition : The position of the viewer (the camera itself).
        /// @param targetPosition : The target position to look at.
        /// @param up : The basis up direction. Which direction is up when looking at the target?
        //----------------------------------------------------------------------------------------------------
        void            LookAt(const nes::Vec3& eyePosition, const nes::Vec3& targetPosition, const nes::Vec3& up);

        //----------------------------------------------------------------------------------------------------
        ///	@brief : Update the Camera's Projection Matrix based on the new viewport. 
        //----------------------------------------------------------------------------------------------------
        void            UpdateViewport(const uint32_t width, const uint32_t height, const bool flipYAxis = true);

        //----------------------------------------------------------------------------------------------------
        /// @brief : Set whether this camera is perspective or orthographic.
        /// @note : This should be followed with a call to UpdateViewPort() to recalculate the view and
        ///     projection matrices.
        //----------------------------------------------------------------------------------------------------
        void            SetProjectionType(const EProjectionType type)       { m_projectionType = type; }

        //----------------------------------------------------------------------------------------------------
        /// @brief : Set the view matrix directly.
        //----------------------------------------------------------------------------------------------------
        void            SetViewMatrix(const Mat44& viewMatrix)               { m_viewMatrix = viewMatrix; }

        //----------------------------------------------------------------------------------------------------
        ///	@brief : Calculates the Camera's view location in world space based on the current view matrix.
        ///     This is an expensive call because we have to calculate the inverse of the view matrix,
        ///     so this should be done sparingly.
        //----------------------------------------------------------------------------------------------------
        Vec3            CameraViewLocation()  const;

        //----------------------------------------------------------------------------------------------------
        ///	@brief : Create a camera that uses single-point perspective. This is the standard viewing for most
        ///     3D applications.
        ///	@param fovRadians : The Field of View, expressed in radians. Should be ~[45-120] degrees.
        ///	@param viewWidth : The width of the view port.
        ///	@param viewHeight : The height of the view port.
        ///	@param nearPlane : The near plane distance from the view position. 0.1f is a reasonable default.
        ///	@param farPlane : The far plane distance from the view position. 1000.f is a reasonable default.
        ///	@param flipYAxis : If you need to flip the Y-axis.
        //----------------------------------------------------------------------------------------------------
        void            SetPerspective(const float fovRadians, const uint32_t viewWidth, const uint32_t viewHeight, const float nearPlane, const float farPlane, const bool flipYAxis = true);

        //----------------------------------------------------------------------------------------------------
        ///	@brief : Create a Camera that uses single-point perspective. This is the standard viewing for most
        ///     3D applications.
        ///	@param fovRadians : The Field of View, expressed in radians. Should be ~[45-120] degrees.
        ///	@param aspectRatio : The aspect ratio of the view port (width/height).
        ///	@param nearPlane : The near plane distance from the view position. 0.1f is a reasonable default.
        ///	@param farPlane : The far plane distance from the view position. 1000.f is a reasonable default.
        ///	@param flipYAxis : If you need to flip the Y-axis.
        //----------------------------------------------------------------------------------------------------
        void            SetPerspective(const float fovRadians, const float aspectRatio, const float nearPlane, const float farPlane, const bool flipYAxis = true);

        //----------------------------------------------------------------------------------------------------
        /// @brief : Set the perspective field of view, expressed in radians. Should be ~[45-120] degrees.
        /// @note : This should be followed with a call to UpdateViewPort() to recalculate the view and
        ///     projection matrices.
        //----------------------------------------------------------------------------------------------------
        void            SetPerspectiveFOV(const float fovRadians)           { m_perspectiveFOV = fovRadians; }

        //----------------------------------------------------------------------------------------------------
        /// @brief :  The near plane distance from the view position. 0.1f is a reasonable default.
        /// @note : This should be followed with a call to UpdateViewPort() to recalculate the view and
        ///     projection matrices.
        //----------------------------------------------------------------------------------------------------
        void            SetPerspectiveNearPlane(const float nearPlane)      { m_perspectiveNear = nearPlane; }

        //----------------------------------------------------------------------------------------------------
        /// @brief : The far plane distance from the view position. 1000.f is a reasonable default.
        /// @note : This should be followed with a call to UpdateViewPort() to recalculate the view and
        ///     projection matrices.
        //----------------------------------------------------------------------------------------------------
        void            SetPerspectiveFarPlane(const float farPlane)        { m_perspectiveFar = farPlane; }

        //----------------------------------------------------------------------------------------------------
        /// @brief : Create a camera that preserves parallel lines. This is primarily used in 2D rendering,
        ///     (HUDs, Menus, etc.) and isometric views.
        /// @param viewWidth : The width of the view port.
        /// @param viewHeight : The height of the view port.
        /// @param orthographicSize : The size of the orthographic projection.
        /// @param near : The near plane distance from the view position. 0.1f is a reasonable default.
        /// @param far : The far plane distance from the view position. 1000.f is a reasonable default.
        /// @param flipYAxis : If you need to flip the Y-axis.
        //----------------------------------------------------------------------------------------------------
        void            SetOrthographic(const uint32_t viewWidth, const uint32_t viewHeight, const float orthographicSize, const float near = -1.0, const float far = 1.0, const bool flipYAxis = true);
        
        //----------------------------------------------------------------------------------------------------
        /// @brief : Set the size of the orthographic projection.
        /// @note : This should be followed with a call to UpdateViewPort() to recalculate the view and
        ///     projection matrices.
        //----------------------------------------------------------------------------------------------------
        inline void     SetOrthographicSize(const float size)               { m_orthographicSize = size; }

        //----------------------------------------------------------------------------------------------------
        /// @brief : The near plane distance from the view position. 0.1f is a reasonable default.
        /// @note : This should be followed with a call to UpdateViewPort() to recalculate the view and
        ///     projection matrices. 
        //----------------------------------------------------------------------------------------------------
        inline void     SetOrthographicNearPlane(const float nearPlane)     { m_orthographicNear = nearPlane; }

        //----------------------------------------------------------------------------------------------------
        /// @brief : The far plane distance from the view position. 1000.f is a reasonable default.
        /// @note : This should be followed with a call to UpdateViewPort() to recalculate the view and
        ///     projection matrices. 
        //----------------------------------------------------------------------------------------------------
        inline void     SetOrthographicFarPlane(const float farPlane)       { m_orthographicFar = farPlane; }
        
        const Mat44&    GetProjectionMatrix() const                         { return m_projectionMatrix; }
        Mat44&          GetProjectionMatrix()                               { return m_projectionMatrix; }
        const Mat44&    GetViewMatrix() const                               { return m_viewMatrix; }
        Mat44&          GetViewMatrix()                                     { return m_viewMatrix; }
        Mat44           ViewProjectionMatrix() const                        { return m_projectionMatrix * m_viewMatrix; }
        EProjectionType GetProjectionType() const                           { return m_projectionType; }
        
        float           GetPerspectiveFOV() const                           { return m_perspectiveFOV; }
        float           GetPerspectiveNear() const                          { return m_perspectiveNear; }
        float           GetPerspectiveFar() const                           { return m_perspectiveFar; }
        float           GetOrthographicSize() const                         { return m_orthographicSize; }
        float           GetOrthographicNear() const                         { return m_orthographicNear; }
        float           GetOrthographicFar() const                          { return m_orthographicFar; }

    private:
        Mat44           m_projectionMatrix = Mat44::Identity();
        Mat44           m_viewMatrix = Mat44::Identity();
        float           m_perspectiveFOV = 60.f;
        float           m_perspectiveNear = 0.01f;
        float           m_perspectiveFar = 1000.f;
        float           m_orthographicSize = 10.f;
        float           m_orthographicNear = -1.f;
        float           m_orthographicFar = 1.f;
        EProjectionType m_projectionType = EProjectionType::Perspective;
    };
}