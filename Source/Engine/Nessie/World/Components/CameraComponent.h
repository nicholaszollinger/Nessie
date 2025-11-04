// CameraComponent.h
#pragma once
#include "Nessie/Graphics/Camera.h"
#include "Nessie/World/Component.h"
#include "Nessie/Math/Math.h"

namespace nes
{
    struct CameraComponent
    {
        Camera          m_camera{};

        //----------------------------------------------------------------------------------------------------
        /// @brief : Calculates the projection matrix, which will be either Perspective or Orthographic depending
        ///     on the current projection type.
        //----------------------------------------------------------------------------------------------------
        Mat44           CalculateProjectionMatrix(const uint32 width, const uint32 height, const bool flipAxis = true) const { return m_camera.CalculateProjectionMatrix(width, height, flipAxis); };    
        
        //----------------------------------------------------------------------------------------------------
        /// @brief : The Exposure Factor is the combined EV value and the ISO adjustment that is applied to
        ///     HDR lighting in the scene.
        //----------------------------------------------------------------------------------------------------
        float           CalculateExposureFactor() const { return m_camera.CalculateExposureFactor(); }
        
        static void     Serialize(YAML::Emitter& out, const CameraComponent& component);
        static void     Deserialize(const YAML::Node& in, CameraComponent& component);
    };
}