// Camera.h
#pragma once
#include "Nessie/Math/Math.h"

struct alignas (64) CameraUBO
{
    nes::Mat44                  m_view              = nes::Mat44::Identity();
    nes::Mat44                  m_projection        = nes::Mat44::Identity();
    nes::Mat44                  m_viewProjection    = nes::Mat44::Identity();   // Cached proj * view.
    nes::Float3                 m_position        = nes::Float3::Zero();
    float                       m_exposureFactor    = 0.000125f;
};

namespace helpers
{
    //----------------------------------------------------------------------------------------------------
    /// @brief : The Exposure Factor is the combined EV value and the ISO adjustment that is applied to
    ///     HDR lighting in the scene.
    ///	@param aperture : AKA the "f-stop". Determines the size of the opening in the lens that allows light through.
    ///     - Lower numbers (1.4, 2.8) = wider aperture = more light.
    ///     - Higher numbers (8, 16) = narrower aperture = less light.
    ///	@param shutterSpeed : How long the camera sensor is exposed to light.
    ///     - Fast Speed (1/500s, 1/1000s) = less light.
    ///     - Slow Speed (1/30s, 1) = more light.
    ///	@param iso : Sensor sensitivity. Acts as a linear brightness multiplier.
    //----------------------------------------------------------------------------------------------------
    static inline float CalculateExposureFactor(const float aperture, const float shutterSpeed, const float iso)
    {
        // Calculate exposure value (EV) - standard photographic formula
        const float ev = std::log2((aperture * aperture) / shutterSpeed);

        // Convert to exposure adjustment factor
        // ISO 100 is baseline, higher ISO = more sensitive (brighter)
        const float isoAdjustment = iso / 100.f;

        // Exposure factor combines EV and ISO
        return isoAdjustment * std::pow(2.f, -ev);
    }
}
