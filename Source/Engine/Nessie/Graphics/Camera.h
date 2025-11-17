// Camera.h
#pragma once
#include "Nessie/Math/Math.h"

namespace nes
{
    //----------------------------------------------------------------------------------------------------
    /// @brief : The Camera contains properties for calculating a projection matrix, as wells as the Aperture,
    ///     Shutter Speed, ISO, etc.
    //----------------------------------------------------------------------------------------------------
    struct Camera
    {
        enum class EProjectionType : uint8
        {
            // Used to create a Camera that uses single-point perspective. This is the standard viewing for most
            // 3D applications.
            Perspective,

            // Used to create a camera that preserves parallel lines. This is primarily used in 2D rendering,
            // (HUDs, Menus, etc.) and isometric views.
            Orthographic,
        };
        
        // The perspective field of view, expressed in degrees. Should be ~[45-120].
        float           m_perspectiveFOV = 60.f;
        
        // Determines the height of the orthographic frustum. The width of the frustum is equal to 'half the size' * 'the aspect ratio of the viewport'. 
        float           m_orthographicSize = 10.f;

        // The near plane distance from the view position. Anything closer than this position will be invisible. 0.1f is a reasonable default.
        float           m_nearPlane = 0.1f;

        // The far plane distance from the view position. 1000.f is a reasonable default.
        float           m_farPlane = 1000.f;
        
        // AKA the "f-stop". Determines the size of the opening in the lens that allows light through.
        // - Lower numbers (1.4, 2.8) = wider aperture = more light.
        // - Higher numbers (8, 16) = narrower aperture = less light.
        float           m_aperture = 8.f;

        // How long the camera sensor is exposed to light, in seconds. When calling CalculateExposureFactor, this value will be
        // set over 1. So if you want a shutter speed of 1/500s, set this to 500.
        //     - Fast Speed (1/500s, 1/1000s) = less light.
        //     - Slow Speed (1/30s, 1) = more light.
        float           m_shutterSpeed = 125.f;

        // Sensor sensitivity. Acts as a linear brightness multiplier.
        float           m_iso = 100.f;

        // Determines how the projection matrix will be calculated. See EProjectionType for more details.
        EProjectionType m_projectionType = EProjectionType::Perspective;

        //----------------------------------------------------------------------------------------------------
        /// @brief : Calculates the projection matrix, which will be either Perspective or Orthographic depending
        ///     on the current projection type.
        //----------------------------------------------------------------------------------------------------
        Mat44           CalculateProjectionMatrix(const uint32 width, const uint32 height, const bool flipAxis = true) const;
        
        //----------------------------------------------------------------------------------------------------
        /// @brief : The Exposure Factor is the combined EV value and the ISO adjustment that is applied to
        ///     HDR lighting in the scene.
        //----------------------------------------------------------------------------------------------------
        float           CalculateExposureFactor() const;
    };
}