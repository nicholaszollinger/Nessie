// Vec2.h
#pragma once
#include "MathTypes.h"
#include "Scalar2.h"

namespace nes
{
    //----------------------------------------------------------------------------------------------------
    // [TODO]: Make this a templated type, because there are no alignment considerations for this Vector.
    /// @brief : 2-component vector.
    //----------------------------------------------------------------------------------------------------
    class Vec2
    {
    public:
        static constexpr size_t N = 2;
        
        float x;
        float y;

        /// Constructors
        NES_INLINE                          Vec2() = default;
        constexpr NES_INLINE                Vec2(const Vec2& other) = default;
        constexpr NES_INLINE                Vec2(const float x, const float y)  : x(x), y(y) {}
        constexpr NES_INLINE explicit       Vec2(const Float2& value)           : x(value.x), y(value.y) {}
        constexpr NES_INLINE explicit       Vec2(const float uniformValue)      : x(uniformValue), y(uniformValue) {}
        constexpr NES_INLINE Vec2&          operator=(const Vec2& other)        = default;

        /// Operators
        NES_INLINE bool           operator==(const Vec2 other) const      { return x == other.x && y == other.y; }
        NES_INLINE bool           operator!=(const Vec2 other) const      { return !(*this == other); }
        NES_INLINE bool           operator<(const Vec2 other) const       { return x < other.x && y < other.y; }
        NES_INLINE bool           operator>(const Vec2 other) const       { return x > other.x && y > other.y; }
        NES_INLINE bool           operator<=(const Vec2 other) const      { return x <= other.x && y <= other.y; }
        NES_INLINE bool           operator>=(const Vec2 other) const      { return x >= other.x && y >= other.y; }
        NES_INLINE float          operator[](const size_t index) const    { NES_ASSERT(index < 2); return *(&x + index); }
        NES_INLINE float&         operator[](const size_t index)          { NES_ASSERT(index < 2); return *(&x + index); }
        NES_INLINE Vec2           operator-() const                       { return Vec2(-x, -y); }
        NES_INLINE Vec2           operator+(const Vec2 other) const;
        NES_INLINE Vec2           operator*(const Vec2 other) const;
        NES_INLINE Vec2           operator-(const Vec2 other) const;
        friend NES_INLINE Vec2    operator*(const float scalar, const Vec2 vec);
        NES_INLINE Vec2           operator/(const Vec2 other) const;
        NES_INLINE Vec2           operator*(const float scalar) const;
        NES_INLINE Vec2           operator/(const float scalar) const;
        NES_INLINE Vec2&                    operator+=(const Vec2 other);
        NES_INLINE Vec2&                    operator-=(const Vec2 other);
        NES_INLINE Vec2&                    operator*=(const Vec2 other);
        NES_INLINE Vec2&                    operator/=(const Vec2 other);
        NES_INLINE Vec2&                    operator*=(const float scalar);
        NES_INLINE Vec2&                    operator/=(const float scalar);

        //----------------------------------------------------------------------------------------------------
        /// @brief : Set all components.
        //----------------------------------------------------------------------------------------------------
        NES_INLINE void         Set(const float inX, const float inY)                   { x = inX; y = inY; }

        //----------------------------------------------------------------------------------------------------
        /// @brief : Check if two vectors are close
        //----------------------------------------------------------------------------------------------------
        NES_INLINE bool         IsClose(const Vec2& other, const float maxDistSqr = 1.0e-12f) const;

        //----------------------------------------------------------------------------------------------------
        /// @brief : Check if the vector is close to zero.
        //----------------------------------------------------------------------------------------------------
        NES_INLINE bool         IsNearZero(const float maxDistSqr = 1.0e-12f) const;

        //----------------------------------------------------------------------------------------------------
        /// @brief : Check if the vector is normalized (Length = 1.0).
        //----------------------------------------------------------------------------------------------------
        NES_INLINE bool         IsNormalized(const float tolerance = 1.0e-6f) const;

        //----------------------------------------------------------------------------------------------------
        /// @brief : Check if *any* components are NaN (not a number). 
        //----------------------------------------------------------------------------------------------------
        NES_INLINE bool         IsNaN() const;

        //----------------------------------------------------------------------------------------------------
        /// @brief : Create a vector with all components equal to this vector's X component. 
        //----------------------------------------------------------------------------------------------------
        NES_INLINE Vec4Reg         SplatX() const;

        //----------------------------------------------------------------------------------------------------
        /// @brief : Create a vector with all components equal to this vector's Y component. 
        //----------------------------------------------------------------------------------------------------
        NES_INLINE Vec4Reg         SplatY() const;

        //----------------------------------------------------------------------------------------------------
        /// @brief : To "Swizzle" a vector means to set the components equal to the specified component value of the passed
        ///     in swizzle argument. For example, Swizzle<1, 0>() will swap the x and y components.
        //----------------------------------------------------------------------------------------------------
        template <uint32 SwizzleX, uint32 SwizzleY>
        NES_INLINE Vec2         Swizzle() const;

        //----------------------------------------------------------------------------------------------------
        /// @brief : Return the absolute value of each component.
        //----------------------------------------------------------------------------------------------------
        NES_INLINE Vec2         Abs() const;
            
        //----------------------------------------------------------------------------------------------------
        /// @brief : Return the reciprocal (1 / value) of each component. 
        //----------------------------------------------------------------------------------------------------
        NES_INLINE Vec2         Reciprocal() const;

        //----------------------------------------------------------------------------------------------------
        /// @brief : Return the dot product stored across each component of the result vector.   
        //----------------------------------------------------------------------------------------------------
        NES_INLINE Vec4Reg         DotV4(const Vec2& other) const;

        //----------------------------------------------------------------------------------------------------
        /// @brief : Return the dot product between this and another vector.
        //----------------------------------------------------------------------------------------------------
        NES_INLINE float        Dot(const Vec2& other) const;

        //----------------------------------------------------------------------------------------------------
        /// @brief : Returns the squared length (magnitude) of the vector. 
        //----------------------------------------------------------------------------------------------------
        NES_INLINE float        LengthSqr() const;

        //----------------------------------------------------------------------------------------------------
        /// @brief : Returns the length (magnitude) of the vector. 
        //----------------------------------------------------------------------------------------------------
        NES_INLINE float        Length() const;

        //----------------------------------------------------------------------------------------------------
        /// @brief : Normalize this vector. 
        //----------------------------------------------------------------------------------------------------
        NES_INLINE Vec2&        Normalize();

        //----------------------------------------------------------------------------------------------------
        /// @brief : Get a normalized version of this vector. 
        //----------------------------------------------------------------------------------------------------
        NES_INLINE Vec2         Normalized() const;

        //----------------------------------------------------------------------------------------------------
        /// @brief : Returns the normalized vector, or the zeroValue if the length of this vector is zero.
        //----------------------------------------------------------------------------------------------------
        NES_INLINE Vec2         NormalizedOr(const Vec2& zeroValue) const;

        //----------------------------------------------------------------------------------------------------
        /// @brief : Returns a perpendicular vector, equal to rotating this vector 90 degrees counter-clockwise.
        //----------------------------------------------------------------------------------------------------
        NES_INLINE Vec2         Perpendicular() const;  
        
        //----------------------------------------------------------------------------------------------------
        /// @brief : Get a normalized perpendicular vector. 
        //----------------------------------------------------------------------------------------------------
        NES_INLINE Vec2         NormalizedPerpendicular() const;

        //----------------------------------------------------------------------------------------------------
        /// @brief : Rotate this vector by an angle (in radians).
        //----------------------------------------------------------------------------------------------------
        NES_INLINE Vec2&        Rotate(const float angle);

        //----------------------------------------------------------------------------------------------------
        /// @brief : Get this vector rotated by an angle (in radians).
        //----------------------------------------------------------------------------------------------------
        NES_INLINE Vec2         Rotated(const float angle) const;

        //----------------------------------------------------------------------------------------------------
        /// @brief : Store the component values into pOutFloats.
        //----------------------------------------------------------------------------------------------------
        NES_INLINE void         StoreFloat2(Float2* pOutFloats) const;

        //----------------------------------------------------------------------------------------------------
        /// @brief : Return the minimum value of X, Y.  
        //----------------------------------------------------------------------------------------------------
        NES_INLINE float        MinComponent() const;

        //----------------------------------------------------------------------------------------------------
        /// @brief : Return the maximum value of X, Y.  
        //----------------------------------------------------------------------------------------------------
        NES_INLINE float        MaxComponent() const;

        //----------------------------------------------------------------------------------------------------
        /// @brief : Return the index of the minimum value between X, Y.  
        //----------------------------------------------------------------------------------------------------
        NES_INLINE int          MinComponentIndex() const;

        //----------------------------------------------------------------------------------------------------
        /// @brief : Return the index of the maximum value between X, Y.  
        //----------------------------------------------------------------------------------------------------
        NES_INLINE int          MaxComponentIndex() const;

        //----------------------------------------------------------------------------------------------------
        /// @brief : Component-wise square root.
        //----------------------------------------------------------------------------------------------------
        NES_INLINE Vec2         Sqrt() const;

        //----------------------------------------------------------------------------------------------------
        /// @brief : Returns a vector that contains the sign of each component (1.0 for positive, -1.0 for negative). 
        //----------------------------------------------------------------------------------------------------
        NES_INLINE Vec2         GetSign() const;

        //----------------------------------------------------------------------------------------------------
        /// @brief : Vector with all components set to zero. 
        //----------------------------------------------------------------------------------------------------
        static constexpr NES_INLINE Vec2  Zero()                            { return Vec2(0.0f, 0.0f); }

        //----------------------------------------------------------------------------------------------------
        /// @brief : Vector with all components set to one.
        //----------------------------------------------------------------------------------------------------
        static constexpr NES_INLINE Vec2  One()                             { return Vec2(1.0f, 1.0f); }

        //----------------------------------------------------------------------------------------------------
        /// @brief : Vector with all components set to NaN (Not a Number). 
        //----------------------------------------------------------------------------------------------------
        static constexpr NES_INLINE Vec2  NaN()                             { return Vec2(std::numeric_limits<float>::quiet_NaN(), std::numeric_limits<float>::quiet_NaN()); }

        //----------------------------------------------------------------------------------------------------
        /// @brief : Return the X Axis vector. 
        //----------------------------------------------------------------------------------------------------
        static constexpr NES_INLINE Vec2  AxisX()                           { return Vec2(1.0f, 0.0f); }

        //----------------------------------------------------------------------------------------------------
        /// @brief : Return the Y Axis vector. 
        //----------------------------------------------------------------------------------------------------
        static constexpr NES_INLINE Vec2  AxisY()                                     { return Vec2(0.0f, 1.0f); }

        //----------------------------------------------------------------------------------------------------
        /// @brief : Return the Right axis vector (equal to X Axis). 
        //----------------------------------------------------------------------------------------------------
        static constexpr NES_INLINE Vec2  Right()                                     { return Vec2(1.0f, 0.0f); }

        //----------------------------------------------------------------------------------------------------
        /// @brief : Return the Up axis vector (equal to Y Axis). 
        //----------------------------------------------------------------------------------------------------
        static constexpr NES_INLINE Vec2  Up()                                        { return Vec2(0.0f, 1.0f); }

        //----------------------------------------------------------------------------------------------------
        /// @brief : Creates a vector with the minimum value of each component.
        //----------------------------------------------------------------------------------------------------
        static NES_INLINE Vec2  Min(const Vec2 a, const Vec2 b);

        //----------------------------------------------------------------------------------------------------
        /// @brief : Creates a vector with the maximum value of each component.
        //----------------------------------------------------------------------------------------------------
        static NES_INLINE Vec2  Max(const Vec2 a, const Vec2 b);

        //----------------------------------------------------------------------------------------------------
        /// @brief : Create a Vec2 from an angle (in radians). X = cos(angle) and the y = sin(angle).
        //----------------------------------------------------------------------------------------------------
        static NES_INLINE Vec2  FromAngle(const float radians);

        //----------------------------------------------------------------------------------------------------
        /// @brief : Calculate the dot product between two vectors.
        //----------------------------------------------------------------------------------------------------
        static NES_INLINE float Dot(const Vec2 a, const Vec2 b)             { return a.Dot(b); }
        
        //----------------------------------------------------------------------------------------------------
        ///	@brief : Linearly interpolate between two vectors.
        ///	@param from : The Vector we are starting from.
        ///	@param to : The Vector we are going toward.
        ///	@param t : The percentage of the resulting vector will be between the two vectors. Should be a value between [0, 1].
        //----------------------------------------------------------------------------------------------------
        static NES_INLINE Vec2  Lerp(const Vec2 from, const Vec2 to, const float t);

        //----------------------------------------------------------------------------------------------------
        /// @brief : Compute the distance between two points.
        //----------------------------------------------------------------------------------------------------
        static NES_INLINE float Distance(const Vec2& a, const Vec2& b)      { return (a - b).Length(); }

        //----------------------------------------------------------------------------------------------------
        /// @brief : Compute the squared distance between two points.
        //----------------------------------------------------------------------------------------------------
        static NES_INLINE float DistanceSqr(const Vec2& a, const Vec2& b)   { return (a - b).LengthSqr(); }
    };
}

#include "Vec2.inl"