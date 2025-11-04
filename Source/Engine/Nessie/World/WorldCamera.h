// WorldCamera.h
#pragma once
#include "Nessie/Graphics/Camera.h"
#include "Nessie/Math/Math.h"

namespace nes
{
    struct WorldCamera
    {
        Camera          m_camera{};                         // Camera properties. 
        Vec3            m_position = nes::Vec3::Zero();     // Position in world space.
        Vec3            m_forward = nes::Vec3::Forward();   // Forward direction in world space.
        Vec3            m_up = nes::Vec3::Up();             // Up direction in world space.
        
        Mat44           CalculateViewMatrix() const         { return Mat44::LookAt(m_position, m_position + m_forward, m_up); }
    };
}