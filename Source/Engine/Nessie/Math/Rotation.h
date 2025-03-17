// Rotation.h
#pragma once
#include "Quaternion.h"

namespace nes
{
    //----------------------------------------------------------------------------------------------------
    //		NOTES:
    //      This is meant to be the human-readable rotation class.
    //		
    ///		@brief : Describes a 3D Rotation in each of pitch, yaw, and roll. All values are stored as degrees.
    //----------------------------------------------------------------------------------------------------
    template <FloatingPointType Type>
    struct TRotation
    {
    public:
        //----------------------------------------------------------------------------------------------------
        ///		@brief : Rotation about the right axis (X-Axis). Looking up and down. (0 = straight, +Down, -Up). 
        //----------------------------------------------------------------------------------------------------
        Type m_pitch{};

        //----------------------------------------------------------------------------------------------------
        ///		@brief : Rotation about the up axis (Y-Axis). Looking left and right. (0 = straight, +Right, -Left).
        //----------------------------------------------------------------------------------------------------
        Type m_yaw{};

        //----------------------------------------------------------------------------------------------------
        ///		@brief : Rotation about the forward axis (Z-Axis). Tilting your head. (0 = straight,
        ///         +CounterClockwise, -Clockwise) 
        //----------------------------------------------------------------------------------------------------
        Type m_roll{};
        
    public:
        constexpr TRotation() = default;
        constexpr TRotation(const Type pitch, const Type yaw, const Type roll);
        constexpr TRotation(const TVector3<Type> eulerAngles);

        TRotation operator+(const TRotation other) const;
        TRotation operator-(const TRotation other) const;
        TRotation& operator+=(const TRotation& other);
        TRotation& operator-=(const TRotation& other);
        
        void Clamp();
        void Normalize();
        void RotateVector(TVector3<Type>& vector);
        
        TRotation           GetClamped() const;
        TRotation           GetNormalized() const;
        TVector3<Type>      ToEuler() const;
        TVector3<Type>      RotatedVector(const TVector3<Type>& vector);
        TQuaternion<Type>   ToQuaternion() const;

        static void ClampAxis(Type& angle);
        static void NormalizeAxis(Type& angle);
    };

    template <FloatingPointType Type>
    constexpr TRotation<Type>::TRotation(const Type pitch, const Type yaw, const Type roll)
        : m_pitch(pitch)
        , m_yaw(yaw)
        , m_roll(roll)
    {
        //
    }

    //----------------------------------------------------------------------------------------------------
    ///		@brief : Create a Rotation from a set of euler angles. The angles are expected to be stored in
    ///         degrees, with x = pitch, y = yaw, z = roll.
    //----------------------------------------------------------------------------------------------------
    template <FloatingPointType Type>
    constexpr TRotation<Type>::TRotation(const TVector3<Type> eulerAngles)
        : m_pitch(eulerAngles.x)
        , m_yaw(eulerAngles.y)
        , m_roll(eulerAngles.z)
    {
        //
    }

    template <FloatingPointType Type>
    TRotation<Type> TRotation<Type>::operator+(const TRotation other) const
    {
        return TRotation(
            m_pitch + other.m_pitch
            , m_yaw + other.m_yaw
            , m_roll + other.m_roll);
    }

    template <FloatingPointType Type>
    TRotation<Type> TRotation<Type>::operator-(const TRotation other) const
    {
        return TRotation(
            m_pitch - other.m_pitch
            , m_yaw - other.m_yaw
            , m_roll - other.m_roll);
    }

    template <FloatingPointType Type>
    TRotation<Type>& TRotation<Type>::operator+=(const TRotation& other)
    {
        *this = *this + other;
        return *this;
    }

    template <FloatingPointType Type>
    TRotation<Type>& TRotation<Type>::operator-=(const TRotation& other)
    {
        *this = *this - other;
        return *this;
    }

    //----------------------------------------------------------------------------------------------------
    ///		@brief : Clamps each axis value to the range [0, 360). 
    //----------------------------------------------------------------------------------------------------
    template <FloatingPointType Type>
    void TRotation<Type>::Clamp()
    {
        ClampAxis(m_pitch);
        ClampAxis(m_roll);
        ClampAxis(m_yaw);
    }

    //----------------------------------------------------------------------------------------------------
    ///		@brief : Clamps each axis value to the range (-180, 180]. 
    //----------------------------------------------------------------------------------------------------
    template <FloatingPointType Type>
    void TRotation<Type>::Normalize()
    {
        NormalizeAxis(m_pitch);
        NormalizeAxis(m_roll);
        NormalizeAxis(m_yaw);
    }

    //----------------------------------------------------------------------------------------------------
    ///		@brief : Apply this rotation to the Vector. 
    //----------------------------------------------------------------------------------------------------
    template <FloatingPointType Type>
    void TRotation<Type>::RotateVector(TVector3<Type>& vector)
    {
        ToQuaternion().RotateVector(vector);
    }

    //----------------------------------------------------------------------------------------------------
    ///		@brief : Return the clamped version of this rotation, with each axis value
    ///         set to the range [0, 360).
    //----------------------------------------------------------------------------------------------------
    template <FloatingPointType Type>
    TRotation<Type> TRotation<Type>::GetClamped() const
    {
        TRotation result(*this);
        result.Clamp();
        return result;
    }

    //----------------------------------------------------------------------------------------------------
    ///		@brief : Return the normalized version of this rotation, with each axis value
    ///         set to the range (-180, 180].
    //----------------------------------------------------------------------------------------------------
    template <FloatingPointType Type>
    TRotation<Type> TRotation<Type>::GetNormalized() const
    {
        TRotation result(*this);
        result.Normalize();
        return result;
    }

    //----------------------------------------------------------------------------------------------------
    ///		@brief : Returns the vector representation of the rotation. Axis values will be normalized.
    //----------------------------------------------------------------------------------------------------
    template <FloatingPointType Type>
    TVector3<Type> TRotation<Type>::ToEuler() const
    {
        const TRotation normalized = GetNormalized();
        return TVector3<Type>(normalized.m_pitch, normalized.m_yaw, normalized.m_roll);
    }

    //----------------------------------------------------------------------------------------------------
    ///		@brief : Get the resulting vector with the rotation applied.
    //----------------------------------------------------------------------------------------------------
    template <FloatingPointType Type>
    TVector3<Type> TRotation<Type>::RotatedVector(const TVector3<Type>& vector)
    {
        TVector3<Type> result(vector);
        RotateVector(result);
        return result;
    }

    //----------------------------------------------------------------------------------------------------
    ///		@brief : Returns this Rotation represented as a Quaternion. 
    //----------------------------------------------------------------------------------------------------
    template <FloatingPointType Type>
    TQuaternion<Type> TRotation<Type>::ToQuaternion() const
    {
        return TQuaternion<Type>::MakeFromEuler(ToEuler()); 
    }

    template <FloatingPointType Type>
    void TRotation<Type>::ClampAxis(Type& angle)
    {
        angle = math::ModF(angle, static_cast<Type>(360.0));
        
        if (angle < static_cast<Type>(0.0))
            angle += static_cast<Type>(360.0);
    }

    template <FloatingPointType Type>
    void TRotation<Type>::NormalizeAxis(Type& angle)
    {
        // Clamp to [0, 360)
        ClampAxis(angle);

        // If greater than 180, subtract 360 to get within range of (-180, 180].
        if (angle > static_cast<Type>(180.0))
            angle -= static_cast<Type>(360.0);
    }

    using Rotationf = TRotation<float>;
    using Rotationd = TRotation<double>;
    using Rotation = TRotation<NES_MATH_DEFAULT_REAL_TYPE>;
}