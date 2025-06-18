// Quat.h
#pragma once
#include "Vec3.h"
#include "Vec4.h"

namespace nes
{
    //----------------------------------------------------------------------------------------------------
    /// @brief : Quaternion class. Quaternions are 4-dimensional vectors which describe rotations in
    ///     3-dimensional space if their length is 1.
    ///
    /// They are written as:
    ///     q = w + xi + yj + zk
    ///
    /// or in vector notation:
    ///     q = [w, v] = [w, x, y, z]
    ///
    /// Where:
    /// - w = the real part
    /// - v = the imaginary part (x, y, z)
    ///
    /// @note : We store the quaternion in a Vec4 as [x, y, z, w] because that makes it easy to extract
    ///     the rotation axis of the quaternion: q = [cos(angle / 2), sin(angle / 2) * rotationAxis].
    //----------------------------------------------------------------------------------------------------
    struct alignas (NES_VECTOR_ALIGNMENT) Quat
    {
        Vec4 m_value;

        Quat() = default;
        Quat(const Quat& other) = default;
        Quat& operator=(const Quat& other) = default;

        /// Conversion Constructors
        inline                  Quat(const float x, const float y, const float z, const float w) : m_value(x, y, z, w) {}
        inline explicit         Quat(const Vec4& vec) : m_value(vec) {}

        /// Operators
        inline bool             operator==(const Quat& other) const                 { return m_value == other.m_value; }
        inline bool             operator!=(const Quat& other) const                 { return m_value != other.m_value; }
        NES_INLINE Quat&        operator+=(const Quat& other)                       { m_value += other.m_value; return *this; }
        NES_INLINE Quat&        operator-=(const Quat& other)                       { m_value -= other.m_value; return *this; }
        NES_INLINE Quat&        operator*=(const float scalar)                      { m_value *= scalar; return *this; }
        NES_INLINE Quat&        operator/=(const float scalar)                      { m_value /= scalar; return *this; }
        NES_INLINE Quat         operator+(const Quat& other) const                  { return Quat(m_value + other.m_value); }
        NES_INLINE Quat         operator-(const Quat& other) const                  { return Quat(m_value - other.m_value); }
        NES_INLINE Quat         operator/(const float scalar) const                 { return Quat(m_value / scalar); }
        NES_INLINE Quat         operator*(const Quat& other) const;                 
        NES_INLINE Quat         operator*(const float scalar) const                 { return Quat(m_value * scalar); }
        inline friend Quat      operator*(const float scalar, const Quat& other)    { return Quat(other.m_value * scalar); }
        NES_INLINE Quat         operator-() const                                   { return Quat(-m_value); }

        //----------------------------------------------------------------------------------------------------
        /// @brief : Rotate a vector by this quaternion.
        //----------------------------------------------------------------------------------------------------
        NES_INLINE Vec3         operator*(const Vec3& vec) const;

        //----------------------------------------------------------------------------------------------------
        /// @brief : Check if this quaternion is close to the other. Not that -q and q represent the same rotation,
        ///     and that is *not* checked here.
        //----------------------------------------------------------------------------------------------------
        inline bool             IsClose(const Quat& other, const float maxDistSqr = 1.0e-12f) const {  return m_value.IsClose(other.m_value, maxDistSqr); }

        //----------------------------------------------------------------------------------------------------
        /// @brief : Check if this quaternion's length is 1 +/- the tolerance.
        //----------------------------------------------------------------------------------------------------
        inline bool             IsNormalized(const float tolerance = 1.0e-5f) const                 { return m_value.IsNormalized(tolerance); }

        //----------------------------------------------------------------------------------------------------
        /// @brief : Check if any component of the quaternion is NaN (not a number). 
        //----------------------------------------------------------------------------------------------------
        inline bool             IsNaN() const                                                       { return m_value.IsNaN(); }

        //----------------------------------------------------------------------------------------------------
        /// @brief : Get the x component (imaginary part i). 
        //----------------------------------------------------------------------------------------------------
        NES_INLINE float        GetX() const                                                   { return m_value.x; }

        //----------------------------------------------------------------------------------------------------
        /// @brief : Get the y component (imaginary part j). 
        //----------------------------------------------------------------------------------------------------
        NES_INLINE float        GetY() const                                                   { return m_value.y; }

        //----------------------------------------------------------------------------------------------------
        /// @brief : Get the z component (imaginary part k). 
        //----------------------------------------------------------------------------------------------------
        NES_INLINE float        GetZ() const                                                   { return m_value.z; }

        //----------------------------------------------------------------------------------------------------
        /// @brief : Get the w component (real part). 
        //----------------------------------------------------------------------------------------------------
        NES_INLINE float        GetW() const                                                   { return m_value.w; }

        //----------------------------------------------------------------------------------------------------
        /// @brief : Get the imaginary part of the quaternion. 
        //----------------------------------------------------------------------------------------------------
        NES_INLINE Vec3         GetXYZ() const                                                  { return Vec3(m_value); }

        //----------------------------------------------------------------------------------------------------
        /// @brief : Get the quaternion as a Vec4. 
        //----------------------------------------------------------------------------------------------------
        NES_INLINE Vec4         GetXYZW() const                                                 { return m_value; }

        NES_INLINE void         SetX(const float value)                                         { m_value.x = value; }        
        NES_INLINE void         SetY(const float value)                                         { m_value.y = value; }        
        NES_INLINE void         SetZ(const float value)                                         { m_value.z = value; }        
        NES_INLINE void         SetW(const float value)                                         { m_value.w = value; }
        
        //----------------------------------------------------------------------------------------------------
        /// @brief : Get the axis and angle that this quaternion represents.
        ///     The angle will be in the range [0, PI].
        //----------------------------------------------------------------------------------------------------
        NES_INLINE void         ToAxisAngle(Vec3& outAxis, float& outAngle) const;

        //----------------------------------------------------------------------------------------------------
        /// @brief : Creates a quaternion that rotates a vector from the direction of 'from' to the direction
        ///     of 'to' along the shortest path.
        /// @see : https://www.euclideanspace.com/maths/algebra/vectors/angleBetween/index.htm
        //----------------------------------------------------------------------------------------------------
        static NES_INLINE Quat  FromTo(const Vec3& from, const Vec3& to);

        //----------------------------------------------------------------------------------------------------
        /// @brief : Conversion from euler angles. Rotation order is X then Y then Z  (RotZ * RotY * RotX).
        ///     Angles must be in radians.
        //----------------------------------------------------------------------------------------------------
        static inline Quat      EulerAngles(const Vec3& eulerAngles);

        //----------------------------------------------------------------------------------------------------
        /// @brief : Convert to euler angles. Rotation order is X then Y then Z  (RotZ * RotY * RotX).
        ///     Angles returned will be in radians.
        //----------------------------------------------------------------------------------------------------
        inline Vec3             ToEulerAngles() const;

        //----------------------------------------------------------------------------------------------------
        /// @brief : Get the squared length of this quaternion. 
        //----------------------------------------------------------------------------------------------------
        NES_INLINE float        LengthSqr() const          { return m_value.LengthSqr(); }

        //----------------------------------------------------------------------------------------------------
        /// @brief : Get the length of this quaternion.
        //----------------------------------------------------------------------------------------------------
        NES_INLINE float        Length() const             { return m_value.Length(); }

        //----------------------------------------------------------------------------------------------------
        /// @brief : Normalize the quaternion (make it length of 1).
        //----------------------------------------------------------------------------------------------------
        NES_INLINE Quat&        Normalize();

        //----------------------------------------------------------------------------------------------------
        /// @brief : Get the normalized version of this quaternion (length of 1). 
        //----------------------------------------------------------------------------------------------------
        NES_INLINE Quat         Normalized() const          { return Quat(m_value.Normalized()); }

        //----------------------------------------------------------------------------------------------------
        /// @brief : Rotate a vector by this quaternion.
        //----------------------------------------------------------------------------------------------------
        NES_INLINE Vec3         Rotate(const Vec3& vec) const     { return *this * vec; }

        //----------------------------------------------------------------------------------------------------
        /// @brief : Rotate a vector by the inverse of this quaternion.
        //----------------------------------------------------------------------------------------------------
        NES_INLINE Vec3         InverseRotate(const Vec3& vec) const;

        //----------------------------------------------------------------------------------------------------
        /// @brief : Rotate the vector (1, 0, 0) with this quaternion. 
        //----------------------------------------------------------------------------------------------------
        NES_INLINE Vec3         RotateAxisX() const;

        //----------------------------------------------------------------------------------------------------
        /// @brief : Rotate the vector (0, 1, 0) with this quaternion. 
        //----------------------------------------------------------------------------------------------------
        NES_INLINE Vec3         RotateAxisY() const;

        //----------------------------------------------------------------------------------------------------
        /// @brief : Rotate the vector (0, 0, 1) with this quaternion. 
        //----------------------------------------------------------------------------------------------------
        NES_INLINE Vec3         RotateAxisZ() const;

        //----------------------------------------------------------------------------------------------------
        /// @brief : Get the Forward vector after begin rotated by this quaternion. 
        //----------------------------------------------------------------------------------------------------
        NES_INLINE Vec3         ForwardVector() const { return RotateAxisZ(); }

        //----------------------------------------------------------------------------------------------------
        /// @brief : Get the right vector after being rotated by this quaternion. 
        //----------------------------------------------------------------------------------------------------
        NES_INLINE Vec3         RightVector() const   { return RotateAxisX(); }

        //----------------------------------------------------------------------------------------------------
        /// @brief : Get the up vector after being rotated by this quaternion. 
        //----------------------------------------------------------------------------------------------------
        NES_INLINE Vec3         UpVector() const      { return RotateAxisY(); }

        //----------------------------------------------------------------------------------------------------
        /// @brief : Get the rotation angle (in radians) about the x-axis. 
        //----------------------------------------------------------------------------------------------------
        NES_INLINE float        Pitch() const;

        //----------------------------------------------------------------------------------------------------
        /// @brief : Get the rotation angle (in radians) about the y-axis. 
        //----------------------------------------------------------------------------------------------------
        NES_INLINE float        Yaw() const;

        //----------------------------------------------------------------------------------------------------
        /// @brief : Get the rotation angle (in radians) about the z-axis. 
        //----------------------------------------------------------------------------------------------------
        NES_INLINE float        Roll() const;

        //----------------------------------------------------------------------------------------------------
        /// @brief : Get the Dot Product between two Quaternions.
        ///     The dot product geometrically represents the cosine of the angle between the two
        ///     quaternions and can be used to determine if two quaternions are pointing in roughly
        ///     the same direction. For unit quaternions, the dot product is equal to 1 if they are
        ///     the same; -1 if they are opposite.
        //----------------------------------------------------------------------------------------------------
        NES_INLINE float        Dot(const Quat& other) const { return m_value.Dot(other.m_value); }

        //----------------------------------------------------------------------------------------------------
        /// @brief : The Conjugate of a quaternion is a quaternion with the axis of rotation inverted. Preserves
        ///     the "w" component, meaning that this represents the opposite angular displacement of the
        ///     quaternion. If this is a normalized quaternion, this is equal to the inverse. 
        //----------------------------------------------------------------------------------------------------
        NES_INLINE Quat         Conjugate() const;

        //----------------------------------------------------------------------------------------------------
        /// @brief : Get the inverse of this quaternion. The inverse represents the opposite angular displacement
        ///     of this quaternion. It is achieved by negating the axis of rotation. 
        //----------------------------------------------------------------------------------------------------
        NES_INLINE Quat         Inverse() const;

        //----------------------------------------------------------------------------------------------------
        /// @brief : Invert this quaternion. The inverse represents the opposite angular displacement
        ///     of this quaternion. It is achieved by negating the axis of rotation. 
        //----------------------------------------------------------------------------------------------------
        NES_INLINE Quat&        Invert();

        //----------------------------------------------------------------------------------------------------
        /// @brief : Ensures that the w component is positive by negating the entire quaternion if it is not. 
        ///     This is useful when you want to store a quaternion as a 3 vector by discarding W and reconstructing
        ///     it as sqrt(1 - x^2 - y^2 - z^2).
        //----------------------------------------------------------------------------------------------------
        NES_INLINE Quat         EnsureWPositive() const;

        //----------------------------------------------------------------------------------------------------
        /// @brief : Get a quaternion that is perpendicular to this one. 
        //----------------------------------------------------------------------------------------------------
        NES_INLINE Quat         GetPerpendicular() const;

        //----------------------------------------------------------------------------------------------------
        /// @brief : Get the rotation angle around 'axis'. This uses the Swing Twist Decomposition to get the
        ///     twist quaternion and uses q(axis, angle) = [cos(angle / 2), axis * sin(angle / 2)])
        //----------------------------------------------------------------------------------------------------
        NES_INLINE float        GetRotationAngle(const Vec3& axis) const;

        //----------------------------------------------------------------------------------------------------
        /// @brief : Swing Twist Decomposition: any quaternion can be split up as:
        ///             q = q_swing * q_twist
        ///     where q_twist rotates only around axis 'v'.
        ///
        ///     Twist can be calculated as:
        ///             q_twist = [q_w, q_ijk * v * v] / abs([q_w, q_ijk * v * v])
        ///     where 'q_w' is the real part of the quaternion and 'q_ijk' is the imaginary part (Vec3). 
        ///
        ///     Swing can be calculated as:
        ///             q_swing = q * q*_twist
        ///     where 'q*_twist' is the complex conjugate of q_twist.
        //----------------------------------------------------------------------------------------------------
        NES_INLINE Quat         GetTwist(const Vec3& axis) const;

        //----------------------------------------------------------------------------------------------------
        /// @brief : Decomposes the quaternion into its swing and twist components:
        ///             q = q_swing * q_twist
        ///
        ///     - q_twist only rotates around the x-axis.
        ///     - q_swing only rotates around the y and z axes.
        /// @see Gino van den Bergen - Rotational Joint Limits in Quaternion Space - GDC 2016
        //----------------------------------------------------------------------------------------------------
        NES_INLINE void         GetSwingTwist(Quat& outSwing, Quat& outTwist) const;

        //----------------------------------------------------------------------------------------------------
        /// @brief : Linearly interpolate between two quaternions (for small steps).
        ///	@param destination : The destination quaternion (this quaternion is the starting point).
        ///	@param fraction : Must be in the range [0, 1].
        ///	@returns : (1 - inFraction) * this + fraction * inDestination
        //----------------------------------------------------------------------------------------------------
        NES_INLINE Quat         Lerp(const Quat& destination, const float fraction) const;

        //----------------------------------------------------------------------------------------------------
        /// @brief : Linearly interpolate between two quaternions (for small steps).
        ///	@param start : The starting quaternion.
        ///	@param end : The destination quaternion.
        ///	@param fraction : Must be in the range [0, 1].
        ///	@returns : (1 - inFraction) * start + fraction * end
        //----------------------------------------------------------------------------------------------------
        static NES_INLINE Quat  Lerp(const Quat& start, const Quat& end, const float fraction);

        //----------------------------------------------------------------------------------------------------
        /// @brief : Spherically interpolate between two quaternions.
        ///	@param destination : The destination quaternion (this quaternion is the starting point).
        ///	@param fraction : Must be in the range [0, 1].
        ///	@returns : When the fraction = 0, this quaternion is returned; when the fraction = 1, the destination
        ///     quaternion is returned. When the fraction is between 0 and 1, an interpolation along the
        ///     shortest path is returned.
        //----------------------------------------------------------------------------------------------------
        NES_INLINE Quat         Slerp(const Quat& destination, const float fraction) const;

        //----------------------------------------------------------------------------------------------------
        /// @brief : Spherically interpolate between two quaternions.
        ///	@param start : The starting quaternion.
        ///	@param end : The end quaternion.
        ///	@param fraction : Must be in the range [0, 1].
        ///	@returns : When the fraction = 0, the start quaternion is returned; when the fraction = 1, the end
        ///     quaternion is returned. When the fraction is between 0 and 1, an interpolation along the
        ///     shortest path between the start and end is returned.
        //----------------------------------------------------------------------------------------------------
        static NES_INLINE Quat  Slerp(const Quat& start, const Quat& end, const float fraction);

        //----------------------------------------------------------------------------------------------------
        /// @brief : Loads 3 floats form memory (x, y, and z component and then calculates w); reads 32 bits
        ///     extra which it doesn't use.
        //----------------------------------------------------------------------------------------------------
        static NES_INLINE Quat  LoadFloat3Unsafe(const Float3& value);

        //----------------------------------------------------------------------------------------------------
        /// @brief : Store 3 floats to memory (x, y and z components). 
        //----------------------------------------------------------------------------------------------------
        NES_INLINE void         StoreFloat3(Float3* outValue) const;

        //----------------------------------------------------------------------------------------------------
        /// @brief : Returns a quaternion with components = [0, 0, 0, 0 ]
        //----------------------------------------------------------------------------------------------------
        static NES_INLINE Quat  Zero()                                                       { return Quat(Vec4::Zero()); }

        //----------------------------------------------------------------------------------------------------
        /// @brief : Returns a quaternion with components = [0, 0, 0, 1]. Represents no rotation.
        //----------------------------------------------------------------------------------------------------
        static NES_INLINE Quat  Identity()                                                   { return Quat(0.f, 0.f, 0.f, 1.f); }

        //----------------------------------------------------------------------------------------------------
        /// @brief : Create a rotation from an axis and an angle.
        //----------------------------------------------------------------------------------------------------
        static NES_INLINE Quat  FromAxisAngle(const Vec3& axis, const float angle);
    };
}

#include "Quat.inl"
