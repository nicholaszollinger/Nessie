// Rotation.h
#pragma once
#include "Quat.h"

namespace nes
{
    //----------------------------------------------------------------------------------------------------
    //	NOTES: This is meant to be the human-readable rotation class.
    ///	@brief : Describes a 3D Rotation in pitch, yaw, and roll. All values are stored as degrees.
    //----------------------------------------------------------------------------------------------------
    struct Rotation
    {
        float m_pitch = 0.f;    /// Rotation about the right axis (X-Axis). Looking up and down. (0 = straight, +Down, -Up).
        float m_yaw = 0.f;      /// Rotation about the up axis (Y-Axis). Looking left and right. (0 = straight, +Right, -Left).
        float m_roll = 0.f;     /// Rotation about the forward axis (Z-Axis). Tilting your head. (0 = straight, +CounterClockwise, -Clockwise)
        
        /// Constructors
        constexpr                   Rotation() = default;

        //----------------------------------------------------------------------------------------------------
        /// @brief : Create a rotation, with passed in values expected to be in degrees.
        //----------------------------------------------------------------------------------------------------
        constexpr                   Rotation(const float pitch, const float yaw, const float roll) : m_pitch(pitch), m_yaw(yaw), m_roll(roll) {}

        //----------------------------------------------------------------------------------------------------
        ///	@brief : Create a Rotation from a set of euler angles. The angles are expected to be stored in
        ///     degrees, with x = pitch, y = yaw, z = roll.
        //----------------------------------------------------------------------------------------------------
                                    Rotation(const Vec3& eulerAngles) : m_pitch(eulerAngles.x) , m_yaw(eulerAngles.y), m_roll(eulerAngles.z) {}

        /// Operators
        NES_INLINE Rotation         operator+(const Rotation other) const;
        NES_INLINE Rotation         operator-(const Rotation other) const;
        NES_INLINE Rotation&        operator+=(const Rotation& other);
        NES_INLINE Rotation&        operator-=(const Rotation& other);

        //----------------------------------------------------------------------------------------------------
        ///	@brief : Clamps each axis value to the range [0, 360). 
        //----------------------------------------------------------------------------------------------------
        NES_INLINE void             Clamp();

        //----------------------------------------------------------------------------------------------------
        ///	@brief : Return the clamped version of this rotation, with each axis value
        ///     set to the range [0, 360).
        //----------------------------------------------------------------------------------------------------
        NES_INLINE Rotation         Clamped() const;

        //----------------------------------------------------------------------------------------------------
        ///	@brief : Clamps each axis value to the range (-180, 180]. 
        //----------------------------------------------------------------------------------------------------
        NES_INLINE void             Normalize();

        //----------------------------------------------------------------------------------------------------
        ///	@brief : Return the normalized version of this rotation, with each axis value
        ///     set to the range (-180, 180].
        //----------------------------------------------------------------------------------------------------
        NES_INLINE Rotation         Normalized() const;

        //----------------------------------------------------------------------------------------------------
        ///	@brief : Apply this rotation to the Vector. 
        //----------------------------------------------------------------------------------------------------
        NES_INLINE void             RotateVector(Vec3& vector) const;

        //----------------------------------------------------------------------------------------------------
        ///	@brief : Get the resulting vector with the rotation applied.
        //----------------------------------------------------------------------------------------------------
        NES_INLINE Vec3             RotatedVector(const Vec3& vector) const;

        //----------------------------------------------------------------------------------------------------
        ///	@brief : Returns the vector representation of the rotation. Axis values will be normalized.
        //----------------------------------------------------------------------------------------------------
        NES_INLINE Vec3             ToEuler() const;

        //----------------------------------------------------------------------------------------------------
        ///	@brief : Returns this Rotation represented as a Quaternion. 
        //----------------------------------------------------------------------------------------------------
        NES_INLINE Quat             ToQuat() const      { return Quat::EulerAngles(ToEuler() * math::DegreesToRadians<float>()); }

        //----------------------------------------------------------------------------------------------------
        /// @brief : Clamps the angle (in degrees) between [0, 360). 
        //----------------------------------------------------------------------------------------------------
        NES_INLINE static void      ClampAxis(float& angle);

        //----------------------------------------------------------------------------------------------------
        /// @brief : Clamps the angle (in degrees) between (-180, 180]. 
        //----------------------------------------------------------------------------------------------------
        NES_INLINE static void      NormalizeAxis(float& angle);

        //----------------------------------------------------------------------------------------------------
        ///	@returns : Returns a Rotation object with each of pitch, yaw and roll set to zero.
        //----------------------------------------------------------------------------------------------------
        static constexpr Rotation   Zero()        { return Rotation(0, 0, 0); }
    };
}

#include "Rotation.inl"