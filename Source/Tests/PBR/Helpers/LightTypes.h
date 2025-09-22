// PointLight.h
#pragma once

struct alignas(16) PointLight
{
    nes::Float3 m_position      = nes::Float3(0.f);      // World Position of the Light.
    float       m_intensity     = 1.0f;                             // The amount of energy emitted by a light, in lumens.
    nes::Float3 m_color         = nes::Float3(1.0f);     // Light color.
    float       m_radius        = 10.f;                             // Radius of the light's effect.
    float       m_falloffExp    = 2.f;                              // Drives how quickly the light dissipates from the light source. 
};

//----------------------------------------------------------------------------------------------------
/// @brief : Defines a light from a direction. Commonly used for the Sun or Moon.
///
/// Example Values for the Sun:
/// <code>
///     // Dawn (6:00 AM) - Soft, warm, low angle
///     DirectionalLight dawn = DirectionalLight
///     (
///          m_direction = nes::Vec3(0.8f, -0.3f, 0.5f).Normalized()),   // Low angle, coming from horizon
///          m_intensity = 400.f                                        // 400 lux - Early sunrise
///          m_color = (1.f, 0.7f, 0.4f),                               // Warm orange-pink
///     );
///
///     // Noon (12:00 PM) - Bright white, overhead
///     DirectionalLight noon = DirectionalLight
///     (
///          m_direction = nes::Vec3(0.1f, -1.f, 0.1f).Normalized()),    // Nearly straight down
///          m_intensity = 120000.f                                     // 120,000 lux - Direct sunlight
///          m_color = (1.f, 1.f, 0.95f),                               // Pure white, slight warm tint
///     );
///
///     // Night/Moonlight (10:00 PM) - Cool, very dim
///     DirectionalLight night = DirectionalLight
///     (
///          m_direction = nes::Vec3(-0.3f, -0.8f, -0.5f).Normalized()), // Nearly straight down
///          m_intensity = 0.25f                                        // 0.25 lux - Full moonlight
///          m_color = (0.7f, 0.8f, 1.f),                               // Cool blue-white
///     );
/// </code>
//----------------------------------------------------------------------------------------------------
struct alignas(16) DirectionalLight
{
    nes::Float3   m_direction     = nes::Float3(0.f, -1.f, 0.f);  // Direction of the light.  
    float         m_intensity     = 100'000.f;                          // The amount of energy emitted by the light, in lux (lumens/m^2).
    nes::Float3   m_color         = nes::Float3(1.f);        // Color of the light.
};