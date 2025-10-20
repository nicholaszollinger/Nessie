// PBRLights.h
#pragma once
#include "Nessie/Math/Math.h"

namespace pbr
{
    struct alignas(16) PointLight
    {
        nes::Float3     m_position      = nes::Float3(0.f);      // World Position of the Light.
        float           m_intensity     = 1.0f;                             // The amount of energy emitted by a light, in lumens.
        nes::Float3     m_color         = nes::Float3(1.0f);     // Light color.
        float           m_radius        = 10.f;                             // Radius of the light's effect.
        float           m_falloffExp    = 2.f;                              // Drives how quickly the light dissipates from the light source. 
    };
    
    struct alignas(16) DirectionalLight
    {
        nes::Float3     m_direction     = nes::Float3(0.f, -1.f, 0.f);  // Direction of the light.  
        float           m_intensity     = 100'000.f;                          // The amount of energy emitted by the light, in lux (lumens/m^2).
        nes::Float3     m_color         = nes::Float3(1.f);        // Color of the light.
    };
}