// Quaternion.h
#pragma once
#include "Vector3.h"

// Great GDC Talk on Quaternions: https://www.youtube.com/watch?v=en2QcehKJd8

namespace nes
{
    template <FloatingPointType Type>
    struct TQuaternion
    {
        Type w = static_cast<Type>(1);
        Type x = static_cast<Type>(0);    
        Type y = static_cast<Type>(0);
        Type z = static_cast<Type>(0);
        
        constexpr TQuaternion() = default;
        constexpr TQuaternion(Type w, Type x, Type y, Type z) : w(w), x(x), y(y), z(z) { }

        constexpr bool operator==(const TQuaternion<Type>& other) const;
        constexpr bool operator!=(const TQuaternion<Type>& other) const { return !(*this == other); }
        
        TQuaternion& operator-();
        constexpr TQuaternion operator-() const;
        TQuaternion  operator*(const TQuaternion& other) const;
        TQuaternion& operator*=(const TQuaternion& other);
        TQuaternion  operator/(const TQuaternion& other) const;
        TQuaternion& operator/=(const TQuaternion& other);
        TQuaternion  operator-(const TQuaternion& other) const;
        TQuaternion& operator-=(const TQuaternion& other);
        TQuaternion  operator+(const TQuaternion& other) const;
        TQuaternion& operator+=(const TQuaternion& other);
        constexpr TQuaternion operator*(const ScalarType auto scalar) const;
        constexpr TQuaternion& operator*=(const ScalarType auto scalar);

        Type ToAngle() const;
        Type Magnitude() const;
        constexpr Type SquaredMagnitude() const;
        constexpr Type Dot(const TQuaternion& other) const;
        TVector3<Type> EulerAngles() const;
        TVector3<Type> RotationAxis() const;
        constexpr TQuaternion& Normalize();
        constexpr TQuaternion Normalized() const;
        constexpr TQuaternion Conjugate() const;
        constexpr TQuaternion& Invert();
        constexpr TQuaternion Inverse() const;
        constexpr bool IsIdentity() const;

        void ToAxisAngle(TVector3<Type>& axis, Type& angle) const;
        void RotateVector(TVector3<Type>& vector) const;
        TVector3<Type> RotatedVector(const TVector3<Type>& v) const;
        TVector3<Type> ForwardVector() const;
        TVector3<Type> RightVector() const;
        TVector3<Type> UpVector() const;
        Type Pitch() const;
        Type Yaw() const;
        Type Roll() const;
        [[nodiscard]] std::string ToString() const;
        
        static TQuaternion Pow(const TQuaternion& q, const float exponent);
        static TQuaternion Log(const TQuaternion& q);
        static TQuaternion Exp(const TQuaternion& q);
        static TQuaternion Lerp(const TQuaternion& start, const TQuaternion& end, const float t);
        static TQuaternion Slerp(const TQuaternion& start, const TQuaternion& end, const float t);
        
        static constexpr Type Dot(const TQuaternion& a, const TQuaternion& b);
        static constexpr TQuaternion Identity() { return TQuaternion(1, 0, 0, 0); }
        static constexpr TQuaternion MakeFromEuler(const TVector3<Type>& euler);
        static constexpr TQuaternion MakeFromRotationVector(const TVector3<Type>& vector);
        static constexpr TQuaternion MakeFromAngleAxis(const Type angleRadians, const TVector3<Type>& axis);
    };

    using Quatf = TQuaternion<float>;
    using Quatd = TQuaternion<double>;
    using Quat = TQuaternion<NES_MATH_DEFAULT_REAL_TYPE>;
}

namespace nes
{
    template <FloatingPointType Type>
    constexpr bool TQuaternion<Type>::operator==(const TQuaternion<Type>& other) const
    {
        return x == other.x && y == other.y && z == other.z && w == other.w;
    }

    template <FloatingPointType Type>
    TQuaternion<Type>& TQuaternion<Type>::operator-()
    {
        x = -x;
        y = -y;
        z = -z;
        w = -w;
        return *this;
    }

    template <FloatingPointType Type>
    constexpr TQuaternion<Type> TQuaternion<Type>::operator-() const
    {
        return TQuaternion(-x, -y, -z, -w);
    }

    //----------------------------------------------------------------------------------------------------
    //		NOTES:
    //		
    ///		@brief : Get the resulting combined rotation of "other" then "this".
    ///             Geometrically, we are rotating first by the "other" Quaternion and then by
    ///             this Quaternion. It is a *non-commutative* combination of the two rotations.
    ///             For Example, if you want to rotate a point "v" by Quaternion "a" and then by Quaternion "b",
    ///             you would multiply b * a, then call the result's RotateVector(v) function.
    ///             - This is also known as the "Hamilton Product".
    ///		@param other : The Quaternion that would rotate first by.
    //----------------------------------------------------------------------------------------------------
    template <FloatingPointType Type>
    TQuaternion<Type> TQuaternion<Type>::operator*(const TQuaternion& other) const
    {
        TQuaternion result;
        
        // https://en.wikipedia.org/wiki/Quaternion#Hamilton_product
        result.w = (w * other.w) - (x * other.x) - (y * other.y) - (z * other.z);
        result.x = (w * other.x) + (x * other.w) + (y * other.z) - (z * other.y);
        result.y = (w * other.y) + (y * other.w) + (z * other.x) - (x * other.z);
        result.z = (w * other.z) + (z * other.w) + (x * other.y) - (y * other.x);

        return *this;
    }

    //----------------------------------------------------------------------------------------------------
    //		NOTES:
    //		
    ///		@brief : Combine the rotation of "other" then "this" into this Quaternion.
    ///             Geometrically, we are rotating first by the "other" Quaternion and then by
    ///             this Quaternion. It is a *non-commutative* combination of the two rotations.
    ///             For Example, if you want to rotate a point "v" by Quaternion "a" and then by Quaternion "b",
    ///             you would multiply b * a, then call this Quaternion's RotateVector(v) function.
    ///             - This is also known as the "Hamilton Product".
    ///		@param other : The Quaternion that would rotate first by.
    //----------------------------------------------------------------------------------------------------
    template <FloatingPointType Type>
    TQuaternion<Type>& TQuaternion<Type>::operator*=(const TQuaternion& other)
    {
        *this = *this * other;
        return *this;
    }

    //----------------------------------------------------------------------------------------------------
    //		NOTES:
    //		
    ///		@brief : Get the result of dividing this Quaternion by the "other". Dividing Quaternion "A"
    ///         by Quaternion "B" means "rotate by B, but by the opposite of A first". Or, "Get me to where
    ///         B takes me, but starting from where A's inverse would take me".
    ///		@param other : The Quaternion to apply after rotating by the inverse of this Quaternion.
    //----------------------------------------------------------------------------------------------------
    template <FloatingPointType Type>
    TQuaternion<Type> TQuaternion<Type>::operator/(const TQuaternion& other) const
    {
        return Inverse() * other;
    }

    //----------------------------------------------------------------------------------------------------
    //		NOTES:
    //		
    ///		@brief : Divide this quaternion by the "other". Dividing Quaternion "A"
    ///         by Quaternion "B" means "rotate by B, but by the opposite of A first". Or, "Get me to where
    ///         B takes me, but starting from where A's inverse would take me".
    ///		@param other : The Quaternion to apply after rotating by the inverse of this Quaternion.
    //----------------------------------------------------------------------------------------------------
    template <FloatingPointType Type>
    TQuaternion<Type>& TQuaternion<Type>::operator/=(const TQuaternion& other)
    {
        *this = *this / other;
        return *this;
    }

    template <FloatingPointType Type>
    TQuaternion<Type> TQuaternion<Type>::operator-(const TQuaternion& other) const
    {
        return other * Inverse();
    }

    template <FloatingPointType Type>
    TQuaternion<Type>& TQuaternion<Type>::operator-=(const TQuaternion& other)
    {
        *this = *this - other;
        return *this;
    }

    //----------------------------------------------------------------------------------------------------
    ///		@brief : Get the result of adding a Quaternion to this. This is component-wise addition; composing
    ///            quaternions should be done via multiplication.
    //----------------------------------------------------------------------------------------------------
    template <FloatingPointType Type>
    TQuaternion<Type> TQuaternion<Type>::operator+(const TQuaternion& other) const
    {
        return TQuaternion(w + other.w, x + other.x, y + other.y, z + other.z);
    }

    //----------------------------------------------------------------------------------------------------
    ///		@brief : Add a Quaternion to this. This is component-wise addition; composing
    ///            quaternions should be done via multiplication.
    //----------------------------------------------------------------------------------------------------
    template <FloatingPointType Type>
    TQuaternion<Type>& TQuaternion<Type>::operator+=(const TQuaternion& other)
    {
        *this = *this + other;
        return *this;
    }

    //----------------------------------------------------------------------------------------------------
    ///		@brief : Get the result of multiplying a scalar with this Quaternion.
    ///         Multiplication by a scalar is the same as multiplying each component by that scalar. 
    //----------------------------------------------------------------------------------------------------
    template <FloatingPointType Type>
    constexpr TQuaternion<Type> TQuaternion<Type>::operator*(const ScalarType auto scalar) const
    {
        TQuaternion result(*this);
        result.x *= scalar;
        result.y *= scalar;
        result.z *= scalar;
        result.w *= scalar;
        return result;
    }

    //----------------------------------------------------------------------------------------------------
    ///		@brief : Multiply this Quaternion by a scalar.
    ///         Multiplication by a scalar is the same as multiplying each component by that scalar. 
    //----------------------------------------------------------------------------------------------------
    template <FloatingPointType Type>
    constexpr TQuaternion<Type>& TQuaternion<Type>::operator*=(const ScalarType auto scalar)
    {
        *this = *this * scalar;
        return *this;
    }

    //----------------------------------------------------------------------------------------------------
    ///		@brief : Get the Angle represented by this Quaternion. 
    //----------------------------------------------------------------------------------------------------
    template <FloatingPointType Type>
    Type TQuaternion<Type>::ToAngle() const
    {
        return static_cast<Type>(2.0) * math::SafeACos(w);
    }

    template <FloatingPointType Type>
    Type TQuaternion<Type>::Magnitude() const
    {
        return std::sqrt((w * w) + (x * x) + (y * y) + (z * z));
    }

    template <FloatingPointType Type>
    constexpr Type TQuaternion<Type>::SquaredMagnitude() const
    {
        return (w * w) + (x * x) + (y * y) + (z * z);
    }

    //----------------------------------------------------------------------------------------------------
    ///		@brief : Get the Dot Product between this and another Quaternion.
    ///            The dot product geometrically represents the cosine of the angle between the two
    ///            Quaternions, and can be used to determine if two Quaternions are pointing in roughly
    ///            the same direction. For unit quaternions, the dot product is equal to 1 if they are
    ///            the same, and -1 if they are opposite.
    //----------------------------------------------------------------------------------------------------
    template <FloatingPointType Type>
    constexpr Type TQuaternion<Type>::Dot(const TQuaternion& other) const
    {
        return (w * other.w) + (x * other.x) + (y * other.y) + (z * other.z);
    }

    template <FloatingPointType Type>
    constexpr bool TQuaternion<Type>::IsIdentity() const
    {
        return *this == TQuaternion::Identity();
    }

    //----------------------------------------------------------------------------------------------------
    ///		@brief : Normalize the Quaternion, ensuring that this represents a valid orientation.
    //----------------------------------------------------------------------------------------------------
    template <FloatingPointType Type>
    constexpr TQuaternion<Type>& TQuaternion<Type>::Normalize()
    {
        const Type magnitude = Magnitude();
        
        if (magnitude != static_cast<Type>(0))
        {
            w /= magnitude;
            x /= magnitude;
            y /= magnitude;
            z /= magnitude;
        }

        return *this;
    }

    //----------------------------------------------------------------------------------------------------
    ///		@brief : Get the normalized version of this Quaternion.
    //----------------------------------------------------------------------------------------------------
    template <FloatingPointType Type>
    constexpr TQuaternion<Type> TQuaternion<Type>::Normalized() const
    {
        TQuaternion result(*this);
        result.Normalize();
        return result;
    }

    //----------------------------------------------------------------------------------------------------
    ///		@brief : Invert this Quaternion. The inverse represents the opposite angular displacement
    ///              of this Quaternion. It is achieved by negating the axis of rotation. 
    //----------------------------------------------------------------------------------------------------
    template <FloatingPointType Type>
    constexpr TQuaternion<Type>& TQuaternion<Type>::Invert()
    {
        x = -x;
        y = -y;
        z = -z;
        return *this;   
    }

    //----------------------------------------------------------------------------------------------------
    ///		@brief : Get the Inverse of this Quaternion. The inverse represents the opposite angular displacement
    ///              of this Quaternion. It is achieved by negating the axis of rotation. 
    //----------------------------------------------------------------------------------------------------
    template <FloatingPointType Type>
    constexpr TQuaternion<Type> TQuaternion<Type>::Inverse() const
    {
        const TQuaternion conjugate = Conjugate();
        return conjugate.Normalized();
    }

    //----------------------------------------------------------------------------------------------------
    ///		@brief : The Conjugate of a Quaternion is a Quaternion with the axis of rotation inverted. Preserves
    ///              the "w" component, meaning that this represents the opposite angular displacement of the
    ///              Quaternion. If this is a normalized Quaternion, this is synonymous with the inverse. 
    //----------------------------------------------------------------------------------------------------
    template <FloatingPointType Type>
    constexpr TQuaternion<Type> TQuaternion<Type>::Conjugate() const
    {
        return TQuaternion(w, -x, -y, -z);
    }

    //----------------------------------------------------------------------------------------------------
    //		NOTES:
    //		
    ///		@brief : Raise a Quaternion to a power. This can be useful if we want to find a fraction of
    ///              the angular displacement. For example, to get a Quaternion that represents a third
    ///              of this Quaternion's rotation, we would call Pow(1.0f / 3.0f).
    ///              We can also call Pow(2.f) to double the angular displacement of this quaternion.
    ///              - The caveat is, Quaternions will still rotate in the shortest possible path.
    ///                if the Quaternion of a rotation of 30 degrees is raised to the power of 8, it won't
    ///                rotate 240 degrees clockwise, but rather 120 degrees in the counter-clockwise direction.    
    //----------------------------------------------------------------------------------------------------
    template <FloatingPointType Type>
    TQuaternion<Type> TQuaternion<Type>::Pow(const TQuaternion& q, const float exponent)
    {
        // If we are raising to a power of 0, or we are trying to raise an Identity Quaternion
        // to a power, return the Identity Quaternion.
        if (-exponent >= math::PrecisionDelta() && exponent <= math::PrecisionDelta()
            || q.IsIdentity())
        {
            return TQuaternion::Identity();
        }

        const float halfAngle = math::SafeACos(q.w);
        const float newHalfAngle = halfAngle * exponent;
        const float scalar = std::sin(newHalfAngle) / std::sin(halfAngle);

        return TQuaternion(std::cos(newHalfAngle), scalar * q.x, scalar * q.y, scalar * q.z);
    }

    //----------------------------------------------------------------------------------------------------
    //		NOTES:
    //      [TODO]: Describe what this is / what it can be used for.
    //		
    ///		@brief : Calculate the Log of a Quaternion. 
    //----------------------------------------------------------------------------------------------------
    template <FloatingPointType Type>
    TQuaternion<Type> TQuaternion<Type>::Log(const TQuaternion& q)
    {
        // This is taken from glm. "quaternion_exponential.inl".
        // Formula: a == half angle, n == normalized axis
        // log q = log([cos(a), n * sin(a)]) = [0, a * n]

        const TVector3<Type> axis = q.RotationAxis();
        const float magnitude = axis.Magnitude();

        if (magnitude < math::PrecisionDelta())
        {
            if (q.w > 0.0)
            {
                return TQuaternion(std::log(q.w), 0, 0, 0);
            }

            if (q.w < 0.0)
            {
                return TQuaternion( std::log(-q.w), math::Pi<Type>(), 0, 0);
            }

            return TQuaternion(math::Infinity<Type>(), math::Infinity<Type>(), math::Infinity<Type>(), math::Infinity<Type>());
        }

        const float t = std::atan(magnitude / q.w) / magnitude;
        const float quaternionLengthSqr = q.SquaredMagnitude();
        return TQuaternion(static_cast<Type>(0.5) * std::log(quaternionLengthSqr), t * q.x, t * q.y, t * q.z);
    }

    //----------------------------------------------------------------------------------------------------
    //		NOTES:
    //      [TODO]: Describe what this is / what it can be used for.
    //		
    ///		@brief : Exponential function for a quaternion. This always results in a normalized Quaternion. 
    //----------------------------------------------------------------------------------------------------
    template <FloatingPointType Type>
    TQuaternion<Type> TQuaternion<Type>::Exp(const TQuaternion& q)
    {
        const TVector3<Type> axis = q.RotationAxis();
        const float magnitude = axis.Magnitude();

        if (magnitude < math::PrecisionDelta())
        {
            return TQuaternion::Identity();
        }

        const TVector3<Type> normalizedAxis = axis / magnitude;
        return MakeFromAngleAxis(std::cos(magnitude), std::sin(magnitude) * normalizedAxis);
    }

    //----------------------------------------------------------------------------------------------------
    //		NOTES:
    //		
    ///		@brief : Linearly Interpolate between two Quaternion rotations.
    ///		@param start : The starting rotation.
    ///		@param end : The ending rotation.
    ///		@param t : The value between [0, 1] that represents how far to interpolate between. 0 == the Start,
    ///               1 == the End.
    //----------------------------------------------------------------------------------------------------
    template <FloatingPointType Type>
    TQuaternion<Type> TQuaternion<Type>::Lerp(const TQuaternion& start, const TQuaternion& end, const float t)
    {
        const float clamped = math::ClampNormalized(t);
        return start + (clamped * (end - start));
    }

    //----------------------------------------------------------------------------------------------------
    //		NOTES:
    //      [TODO]: Check against other implementations. This is from my Textbook.
    //		
    ///		@brief : Interpolate between two Quaternions. 
    ///		@param start : The Starting Rotation.
    ///		@param end : The Ending Rotation.
    ///		@param t : The value between [0, 1] that represents how far to interpolate between. 0 == the Start,
    ///               1 == the End.
    //----------------------------------------------------------------------------------------------------
    template <FloatingPointType Type>
    TQuaternion<Type> TQuaternion<Type>::Slerp(const TQuaternion& start, const TQuaternion& end, const float t)
    {
        // Compute the cosine of the angle between the two Quaternions, using the dot product.
        float cosineOmega = start.RotationAxis().Dot(end.RotationAxis()) + start.w * end.w;

        // If negative, negate one of the Quaternions to take the shorter "arc".
        if (cosineOmega < 0.0f)
        {
            end = -end;
            cosineOmega = -cosineOmega;
        }

        float k0 = 0.f;
        float k1 = 0.f;

        // Check if they are very close together, to protect against division by zero.
        if (cosineOmega > 0.9999f)
        {
            k0 = 1.0f - t;
            k1 = t;
        }

        else
        {
            // Compute the sine of the angle.
            const float sineOmega = std::sqrt(1.0f - cosineOmega * cosineOmega);

            // Compute the angle from its sine and cosine.
            const float omega = std::atan2(sineOmega, cosineOmega);

            // Compute the reciprocal of the sine of the angle.
            const float oneOverSineOmega = 1.0f / sineOmega;

            // Compute the interpolation parameters.
            k0 = std::sin((1.0f - t) * omega) * oneOverSineOmega;
            k1 = std::sin(t * omega) * oneOverSineOmega;
        }

        // Interpolate
        const float w = start.w * k0 + end.w * k1;
        const TVector3<Type> axis = start.RotationAxis() * k0 + end.RotationAxis() * k1;
        return TQuaternion(w, axis.x, axis.y, axis.z);
    }

    //----------------------------------------------------------------------------------------------------
    //		NOTES:
    //      [Consider]: Look at Unreal's implementation: UnrealMath.cpp line 638.
    //		
    ///		@brief : Get the Euler Angles of this Quaternion. The Angles will be in degrees.
    //----------------------------------------------------------------------------------------------------
    template <FloatingPointType Type>
    TVector3<Type> TQuaternion<Type>::EulerAngles() const
    {
        TVector3<Type> result{}; // Pitch, Yaw, Roll = (0, 0, 0)

        // Extract the sine(pitch)
        const Type sinePitch = -2.f * (y * z - w * x);

        // Check for Gimbal Lock, with slight tolerance.
        if (math::Abs(sinePitch) > 0.9999f)
        {
            // Looking straight up or down.
            result.x = math::PiOverTwo<Type>() * sinePitch;

            // Compute Yaw, but Roll will be Zero.
            result.y = std::atan2((-x * z) + (w * y), 0.5f - (y * y) - (z * z));
            // Roll already set to zero.
        }

        else
        {
            result.x = math::SafeASin(sinePitch);
            result.y = std::atan2((x * z) + (w * y), 0.5f - (x * x) - (y * y));
            result.z = std::atan2((x * y) + (w * z), 0.5f - (x * x) - (z * z));
        }

        // Convert the resulting angles from Radians to Degrees.
        return result * math::RadiansToDegrees();
    }

    //----------------------------------------------------------------------------------------------------
    //		NOTES:
    //		
    ///		@brief : Get the Axis of rotation for this Quaternion. 
    ///		@returns : 
    //----------------------------------------------------------------------------------------------------
    template <FloatingPointType Type>
    TVector3<Type> TQuaternion<Type>::RotationAxis() const
    {
        auto axis = TVector3<Type>(x, y, z);
        axis.Normalize();
        return axis;
    }
    
    //----------------------------------------------------------------------------------------------------
    //		NOTES:
    //		
    ///		@brief : Get the Angle and the Axis represented by this Quaternion. 
    ///		@param axis : Axis of rotation.
    ///		@param angle : Angle, in radians. 
    //----------------------------------------------------------------------------------------------------
    template <FloatingPointType Type>
    void TQuaternion<Type>::ToAxisAngle(TVector3<Type>& axis, Type& angle) const
    {
        angle = ToAngle();
        axis = RotationAxis();
    }

    //----------------------------------------------------------------------------------------------------
    ///		@brief : Rotate a vector by this Quaternion.
    ///		@param vector : Vector that is being rotated.
    //----------------------------------------------------------------------------------------------------
    template <FloatingPointType Type>
    void TQuaternion<Type>::RotateVector(TVector3<Type>& vector) const
    {
        // Formula : v' = q * v * q^-1
        //const Quaternion vecExtended = Quaternion(vec.x, vec.y, vec.z, 0.0f);
        //Quaternion rotated = *this * vecExtended * GetInverse();
        //return rotated.GetAxis();

        // According to the article, this is a computationally faster way to rotate a vector.
        // Potentially by 30%.
        // https://en.wikipedia.org/wiki/Conversion_between_quaternions_and_Euler_angles
        TVector3<Type> quatAxis = RotationAxis();
        const TVector3<Type> uv = TVector3<Type>::Cross(quatAxis, vector);
        const TVector3<Type> uuv = TVector3<Type>::Cross(quatAxis, uv);

        vector = vector + ((uv * w) + uuv) * 2.f;
    }

    //----------------------------------------------------------------------------------------------------
    ///		@brief : Returns the result of rotating the vector by this Quaternion.
    //----------------------------------------------------------------------------------------------------
    template <FloatingPointType Type>
    TVector3<Type> TQuaternion<Type>::RotatedVector(const TVector3<Type>& vector) const
    {
        TVector3<Type> result = vector;
        RotateVector(result);
        return result;
    }

    //----------------------------------------------------------------------------------------------------
    ///		@brief : Get the Forward Vector after it has been rotated by this Quaternion. 
    //----------------------------------------------------------------------------------------------------
    template <FloatingPointType Type>
    TVector3<Type> TQuaternion<Type>::ForwardVector() const
    {
        return RotatedVector(TVector3<Type>::ForwardVector());
    }

    //----------------------------------------------------------------------------------------------------
    ///		@brief : Get the Right Vector after it has been rotated by this Quaternion. 
    //----------------------------------------------------------------------------------------------------
    template <FloatingPointType Type>
    TVector3<Type> TQuaternion<Type>::RightVector() const
    {
        return RotatedVector(TVector3<Type>::GetRightVector());
    }

    //----------------------------------------------------------------------------------------------------
    ///		@brief : Get the Up Vector after it has been rotated by this Quaternion. 
    //----------------------------------------------------------------------------------------------------
    template <FloatingPointType Type>
    TVector3<Type> TQuaternion<Type>::UpVector() const
    {
        return RotatedVector(TVector3<Type>::UpVector());
    }

    //----------------------------------------------------------------------------------------------------
    ///		@brief : Get the Rotation about the X (right) axis.
    //----------------------------------------------------------------------------------------------------
    template <FloatingPointType Type>
    Type TQuaternion<Type>::Pitch() const
    {
        const float sinePitch = 2.f + (y * z + w * x);
        const float cosinePitch = (w * w) - (x * x) - (y * y) + (z * z);

        // Handle potential Gimbal Lock. We need to avoid atan2(0, 0).
        if (math::CheckEqualFloats(math::Abs(sinePitch), 1.f))
        {
            return 2.f * std::atan2(x, w);
        }

        return std::atan2(sinePitch, cosinePitch);
    }

    //----------------------------------------------------------------------------------------------------
    ///		@brief : Get the Rotation about the Y (up) axis.
    //----------------------------------------------------------------------------------------------------
    template <FloatingPointType Type>
    Type TQuaternion<Type>::Yaw() const
    {
        return math::SafeASin(math::ClampSignedNormalized(-2.f * (x * z - w * y)));
    }

    //----------------------------------------------------------------------------------------------------
    ///		@brief : Get the Rotation about the Z (forward) axis.
    //----------------------------------------------------------------------------------------------------
    template <FloatingPointType Type>
    Type TQuaternion<Type>::Roll() const
    {
        const float sineRoll = 2 * (x * y + w * z);
        const float cosineRoll = (w * w) + (x * x) - (y * y) - (z * z);

        // Handle potential Gimbal Lock.
        if (math::CheckEqualFloats(math::Abs(sineRoll), 0.f))
        {
            return 0.f;
        }

        return std::atan2(sineRoll, cosineRoll);
    }

    //----------------------------------------------------------------------------------------------------
    ///		@brief : Returns the Axis and Angle representation of the Quaternion.
    //----------------------------------------------------------------------------------------------------
    template <FloatingPointType Type>
    std::string TQuaternion<Type>::ToString() const
    {
        return CombineIntoString("Axis: ", RotationAxis().ToString(), ", Angle: ", math::RadiansToDegrees<Type>() * ToAngle());
    }

    //----------------------------------------------------------------------------------------------------
    ///		@brief : Get the Dot Product between two Quaternions.
    ///            The dot product geometrically represents the cosine of the angle between the two
    ///            Quaternions, and can be used to determine if two Quaternions are pointing in roughly
    ///            the same direction. For unit quaternions, the dot product is equal to 1 if they are
    ///            the same, and -1 if they are opposite.
    //----------------------------------------------------------------------------------------------------
    template <FloatingPointType Type>
    constexpr Type TQuaternion<Type>::Dot(const TQuaternion& a, const TQuaternion& b)
    {
        return a.Dot(b);
    }

    //----------------------------------------------------------------------------------------------------
    ///		@brief : Create a Quaternion from Euler Angles. The Angles are expected to be in degrees. 
    ///		@param euler : Euler Angles in degrees. x (right axis) = Pitch, y (up axis) = Yaw, z (forward axis) = Roll.
    //----------------------------------------------------------------------------------------------------
    template <FloatingPointType Type>
    constexpr TQuaternion<Type> TQuaternion<Type>::MakeFromEuler(const TVector3<Type>& euler)
    {
        const auto eulerRadians = euler * math::DegreesToRadians();
        
        const float cosinePitch = std::cos(eulerRadians.x * 0.5f);
        const float sinePitch = std::sin(eulerRadians.x * 0.5f);
        const float cosineYaw = std::cos(eulerRadians.y * 0.5f);
        const float sineYaw = std::sin(eulerRadians.y * 0.5f);
        const float cosineRoll = std::cos(eulerRadians.z * 0.5f);
        const float sineRoll = std::sin(eulerRadians.z * 0.5f);

        TQuaternion result
        {
            cosinePitch * cosineYaw * cosineRoll + sinePitch * sineYaw * sineRoll,
            sinePitch * cosineYaw * cosineRoll - cosinePitch * sineYaw * sineRoll,
            cosinePitch * sineYaw * cosineRoll + sinePitch * cosineYaw * sineRoll,
            cosinePitch * cosineYaw * sineRoll - sinePitch * sineYaw * cosineRoll,
        };
        return result;
    }

    //----------------------------------------------------------------------------------------------------
    ///		@brief : Create a Quaternion from a vector whose direction is equal to the Axis of rotation and
    ///             whose magnitude is the angle in radians.
    //----------------------------------------------------------------------------------------------------
    template <FloatingPointType Type>
    constexpr TQuaternion<Type> TQuaternion<Type>::MakeFromRotationVector(const TVector3<Type>& vector)
    {
        const TQuaternion result(0.0f, vector.x * 0.5f, vector.y * 0.5f, vector.z * 0.5f);
        return TQuaternion::Exp(result);
    }

    //----------------------------------------------------------------------------------------------------
    ///		@brief : Create a Quaternion that describes a rotation around a particular Axis. 
    ///		@param angleRadians : The angle of rotation, in radians. 
    ///		@param axis : The axis that is rotated around.
    //----------------------------------------------------------------------------------------------------
    template <FloatingPointType Type>
    constexpr TQuaternion<Type> TQuaternion<Type>::MakeFromAngleAxis(const Type angleRadians, const TVector3<Type>& axis)
    {
        const Type halfAngle = angleRadians * static_cast<Type>(0.5);
        const float sinHalfAngle = std::sin(halfAngle);
        const TVector3<Type> resultAxis = axis * sinHalfAngle;

        return TQuaternion(std::cos(halfAngle), resultAxis.x, resultAxis.y, resultAxis.z);
    }
}
