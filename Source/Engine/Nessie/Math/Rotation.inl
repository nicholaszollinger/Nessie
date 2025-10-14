// Rotation.inl
#pragma once

namespace nes
{
    Rotation Rotation::operator+(const Rotation other) const
    {
        return Rotation(
            m_pitch + other.m_pitch
            , m_yaw + other.m_yaw
            , m_roll + other.m_roll);
    }
    
    Rotation Rotation::operator-(const Rotation other) const
    {
        return Rotation(
            m_pitch - other.m_pitch
            , m_yaw - other.m_yaw
            , m_roll - other.m_roll);
    }
    
    Rotation& Rotation::operator+=(const Rotation& other)
    {
        *this = *this + other;
        return *this;
    }
    
    Rotation& Rotation::operator-=(const Rotation& other)
    {
        *this = *this - other;
        return *this;
    }
    
    void Rotation::Clamp()
    {
        ClampAxis(m_pitch);
        ClampAxis(m_roll);
        ClampAxis(m_yaw);
    }
    
    void Rotation::Normalize()
    {
        NormalizeAxis(m_pitch);
        NormalizeAxis(m_roll);
        NormalizeAxis(m_yaw);
    }
    
    void Rotation::RotateVector(Vec3& vector) const
    {
        vector = ToQuat().Rotate(vector);
    }
    
    Rotation Rotation::Clamped() const
    {
        Rotation result(*this);
        result.Clamp();
        return result;
    }
    
    Rotation Rotation::Normalized() const
    {
        Rotation result(*this);
        result.Normalize();
        return result;
    }
    
    Vec3 Rotation::ToEuler() const
    {
        const Rotation normalized = Normalized();
        return Vec3(normalized.m_pitch, normalized.m_yaw, normalized.m_roll);
    }
    
    Vec3 Rotation::RotatedVector(const Vec3& vector) const
    {
        Vec3 result(vector);
        RotateVector(result);
        return result;
    }
    
    void Rotation::ClampAxis(float& angle)
    {
        angle = math::ModF(angle, 360.f);
        
        if (angle < 0.f)
            angle += 360.f;
    }
    
    void Rotation::NormalizeAxis(float& angle)
    {
        // Clamp to [0, 360)
        ClampAxis(angle);

        // If greater than 180, subtract 360 to get within range of (-180, 180].
        if (angle > 180.f)
            angle -= 360.f;
    }
}