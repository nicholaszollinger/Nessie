// Vec3.h
#pragma once
#include "Scalar3.h"
#include "Detail/Swizzle.h"
#include "MathTypes.h"

namespace nes
{
    //----------------------------------------------------------------------------------------------------
    /// @brief : 3-component vector that is 16-byte aligned. Consider using Float3 for storage savings.
    //----------------------------------------------------------------------------------------------------
    class alignas (NES_VECTOR_ALIGNMENT) Vec3
    {
    public:
        static constexpr size_t N = 3;

        float x;
        float y;
        float z;
            
        Vec3() = default;
        Vec3(const Vec3& other) = default;
        Vec3& operator=(const Vec3& other) = default;

        /// Conversion Constructors
        explicit NES_INLINE     Vec3(const Vec2 vec, float z);
        explicit NES_INLINE     Vec3(const Vec4 vec);
        explicit NES_INLINE     Vec3(const float uniformValue);
        NES_INLINE              Vec3(const Float3& value);
        NES_INLINE              Vec3(const float x, const float y, const float z);

        /// Operators
        NES_INLINE float        operator[](const size_t index) const                { NES_ASSERT(index < N); return *(&x + index); }
        NES_INLINE float&       operator[](const size_t index)                      { NES_ASSERT(index < N); return *(&x + index); }
        NES_INLINE bool         operator==(const Vec3& other) const;
        NES_INLINE bool         operator!=(const Vec3& other) const                 { return !(*this == other); }
        NES_INLINE bool         operator<(const Vec3& other) const                  { return x < other.x && y < other.y && z < other.z; }
        NES_INLINE bool         operator>(const Vec3& other) const                  { return x > other.x && y > other.y && z > other.z; }
        NES_INLINE bool         operator<=(const Vec3& other) const                 { return x <= other.x && y <= other.y && z <= other.z; }
        NES_INLINE bool         operator>=(const Vec3& other) const                 { return x >= other.x && y >= other.y && z >= other.z; }
        NES_INLINE Vec3         operator*(const Vec3& other) const;
        NES_INLINE Vec3&        operator*=(const Vec3& other);
        NES_INLINE Vec3         operator*(const float value) const;
        NES_INLINE Vec3&        operator*=(const float value);
        friend NES_INLINE Vec3  operator*(const float value, const Vec3& vec);
        NES_INLINE Vec3         operator/(const Vec3& other) const;
        NES_INLINE Vec3&        operator/=(const Vec3& other);
        NES_INLINE Vec3         operator/(const float value) const;
        NES_INLINE Vec3&        operator/=(const float value);
        NES_INLINE Vec3         operator+(const Vec3& other) const;
        NES_INLINE Vec3&        operator+=(const Vec3& other);
        NES_INLINE Vec3         operator-(const Vec3& other) const;
        NES_INLINE Vec3&        operator-=(const Vec3& other);
        NES_INLINE Vec3         operator-() const;
            
        //----------------------------------------------------------------------------------------------------
        /// @brief : Check if two vectors are close
        //----------------------------------------------------------------------------------------------------
        NES_INLINE bool         IsClose(const Vec3& other, const float maxDistSqr = 1.0e-12f) const;

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
        NES_INLINE Vec4Reg      SplatX() const;

        //----------------------------------------------------------------------------------------------------
        /// @brief : Create a vector with all components equal to this vector's Y component. 
        //----------------------------------------------------------------------------------------------------
        NES_INLINE Vec4Reg      SplatY() const;
        
        //----------------------------------------------------------------------------------------------------
        /// @brief : Create a vector with all components equal to this vector's Z component. 
        //----------------------------------------------------------------------------------------------------
        NES_INLINE Vec4Reg      SplatZ() const;

        //----------------------------------------------------------------------------------------------------
        /// @brief : To "Swizzle" a vector means to set the components equal to the specified component value of the passed
        ///     in swizzle argument. For example, Swizzle<0, 0, 1>()
        ///     will set the XY components equal to the current X value, and the Z component equal to the current Y value.
        //----------------------------------------------------------------------------------------------------
        template <uint32 SwizzleX, uint32 SwizzleY, uint32 SwizzleZ>
        NES_INLINE Vec3         Swizzle() const;

        //----------------------------------------------------------------------------------------------------
        /// @brief : Return the absolute value of each component.
        //----------------------------------------------------------------------------------------------------
        NES_INLINE Vec3         Abs() const;
            
        //----------------------------------------------------------------------------------------------------
        /// @brief : Return the reciprocal (1 / value) of each component. 
        //----------------------------------------------------------------------------------------------------
        NES_INLINE Vec3         Reciprocal() const;

        //----------------------------------------------------------------------------------------------------
        /// @brief : Return the dot product stored across each component of the result vector.  
        //----------------------------------------------------------------------------------------------------
        NES_INLINE Vec3         DotV(const Vec3& other) const;

        //----------------------------------------------------------------------------------------------------
        /// @brief : Return the dot product stored across each component of the result vector.   
        //----------------------------------------------------------------------------------------------------
        NES_INLINE Vec4Reg      DotV4(const Vec3& other) const;

        //----------------------------------------------------------------------------------------------------
        /// @brief : Return the dot product between this and another vector.
        //----------------------------------------------------------------------------------------------------
        NES_INLINE float        Dot(const Vec3& other) const;

        //----------------------------------------------------------------------------------------------------
        ///	@brief : Calculate the cross-product between this and another vector.
        ///     The cross-product yields a vector that is perpendicular to both vectors.
        //----------------------------------------------------------------------------------------------------
        NES_INLINE Vec3         Cross(const Vec3& other) const;

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
        NES_INLINE Vec3&        Normalize();

        //----------------------------------------------------------------------------------------------------
        /// @brief : Get a normalized version of this vector. 
        //----------------------------------------------------------------------------------------------------
        NES_INLINE Vec3         Normalized() const;

        //----------------------------------------------------------------------------------------------------
        /// @brief : Returns the normalized vector, or the zeroValue if the length of this vector is zero.
        //----------------------------------------------------------------------------------------------------
        NES_INLINE Vec3         NormalizedOr(const Vec3& zeroValue) const;

        //----------------------------------------------------------------------------------------------------
        /// @brief : Store the component values into pOutFloats.
        //----------------------------------------------------------------------------------------------------
        NES_INLINE void         StoreFloat3(Float3* pOutFloats) const;

        // //----------------------------------------------------------------------------------------------------
        // /// @brief : Convert each component to an int.
        // //----------------------------------------------------------------------------------------------------
        // NES_INLINE UVec4        ToInt() const;
        //
        // //----------------------------------------------------------------------------------------------------
        // /// @brief : Reinterpret Vec3 as a UVec4 (doesn't change the bits). 
        // //----------------------------------------------------------------------------------------------------
        // NES_INLINE UVec4        ReinterpretAsInt() const;

        //----------------------------------------------------------------------------------------------------
        /// @brief : Return the minimum value of X, Y, Z, W.  
        //----------------------------------------------------------------------------------------------------
        NES_INLINE float        MinComponent() const;

        //----------------------------------------------------------------------------------------------------
        /// @brief : Return the maximum value of X, Y, Z, W.  
        //----------------------------------------------------------------------------------------------------
        NES_INLINE float        MaxComponent() const;

        //----------------------------------------------------------------------------------------------------
        /// @brief : Return the index of the minimum value between X, Y, Z.  
        //----------------------------------------------------------------------------------------------------
        NES_INLINE int          MinComponentIndex() const;

        //----------------------------------------------------------------------------------------------------
        /// @brief : Return the index of the maximum value between X, Y, Z.  
        //----------------------------------------------------------------------------------------------------
        NES_INLINE int          MaxComponentIndex() const;

        //----------------------------------------------------------------------------------------------------
        /// @brief : Component-wise square root.
        //----------------------------------------------------------------------------------------------------
        NES_INLINE Vec3         Sqrt() const;

        //----------------------------------------------------------------------------------------------------
        /// @brief : Returns a vector that contains the sign of each component (1.0 for positive, -1.0 for negative). 
        //----------------------------------------------------------------------------------------------------
        NES_INLINE Vec3         GetSign() const;

        //----------------------------------------------------------------------------------------------------
        /// @brief : Returns a normalized vector that is perpendicular to this vector.
        //----------------------------------------------------------------------------------------------------
        NES_INLINE Vec3         NormalizedPerpendicular() const;

        //----------------------------------------------------------------------------------------------------
        /// @brief : Vector with all components set to zero. 
        //----------------------------------------------------------------------------------------------------
        static NES_INLINE Vec3  Zero()                                  { return Replicate(0.0f); }

        //----------------------------------------------------------------------------------------------------
        /// @brief : Vector with all components set to one.
        //----------------------------------------------------------------------------------------------------
        static NES_INLINE Vec3  One()                                   { return Replicate(1.0f); }

        //----------------------------------------------------------------------------------------------------
        /// @brief : Vector with all components set to NaN (Not a Number). 
        //----------------------------------------------------------------------------------------------------
        static NES_INLINE Vec3  NaN()                                   { return Replicate(std::numeric_limits<float>::quiet_NaN()); }

        //----------------------------------------------------------------------------------------------------
        /// @brief : Return the X Axis vector. 
        //----------------------------------------------------------------------------------------------------
        static NES_INLINE Vec3  AxisX()                                  { return Vec3(1.0f, 0.0f, 0.0f); }

        //----------------------------------------------------------------------------------------------------
        /// @brief : Return the Y Axis vector. 
        //----------------------------------------------------------------------------------------------------
        static NES_INLINE Vec3  AxisY()                                  { return Vec3(0.0f, 1.0f, 0.0f); }

        //----------------------------------------------------------------------------------------------------
        /// @brief : Return the Z Axis vector. 
        //----------------------------------------------------------------------------------------------------
        static NES_INLINE Vec3  AxisZ()                                  { return Vec3(0.0f, 0.0f, 1.0f); }

        //----------------------------------------------------------------------------------------------------
        /// @brief : Return the Right axis vector (equal to X Axis). 
        //----------------------------------------------------------------------------------------------------
        static NES_INLINE Vec3  Right()                                  { return Vec3(1.0f, 0.0f, 0.0f); }

        //----------------------------------------------------------------------------------------------------
        /// @brief : Return the Up axis vector (equal to Y Axis). 
        //----------------------------------------------------------------------------------------------------
        static NES_INLINE Vec3  Up()                                     { return Vec3(0.0f, 1.0f, 0.0f); }

        //----------------------------------------------------------------------------------------------------
        /// @brief : Return the Forward axis vector (equal to Z Axis). 
        //----------------------------------------------------------------------------------------------------
        static NES_INLINE Vec3  Forward()                                { return Vec3(0.0f, 0.0f, 1.0f); }

        //----------------------------------------------------------------------------------------------------
        /// @brief : Return the Pitch axis vector (equal to X Axis). 
        //----------------------------------------------------------------------------------------------------
        static NES_INLINE Vec3  PitchAxis()                               { return Vec3(1.0f, 0.0f, 0.0f); }
        
        //----------------------------------------------------------------------------------------------------
        /// @brief : Return the Yaw axis vector (equal to Y Axis). 
        //----------------------------------------------------------------------------------------------------
        static NES_INLINE Vec3  YawAxis()                                { return Vec3(0.0f, 1.0f, 0.0f); }
        
        //----------------------------------------------------------------------------------------------------
        /// @brief : Return the Roll axis vector (equal to Z Axis). 
        //----------------------------------------------------------------------------------------------------
        static NES_INLINE Vec3  RollAxis()                                { return Vec3(0.0f, 0.0f, 1.0f); }

        //----------------------------------------------------------------------------------------------------
        /// @brief : Create a vector with all components to the specified value.
        //----------------------------------------------------------------------------------------------------
        static NES_INLINE Vec3  Replicate(const float value);

        //----------------------------------------------------------------------------------------------------
        /// @brief : Load a vector from a 3-element array (reads an extra 32 bits which it doesn't use).
        //----------------------------------------------------------------------------------------------------
        static NES_INLINE Vec3  LoadFloat3Unsafe(const Float3& value);

        //----------------------------------------------------------------------------------------------------
        /// @brief : Creates a vector with the minimum value of each component.
        //----------------------------------------------------------------------------------------------------
        static NES_INLINE Vec3  Min(const Vec3& left, const Vec3& right);

        //----------------------------------------------------------------------------------------------------
        /// @brief : Creates a vector with the maximum value of each component.
        //----------------------------------------------------------------------------------------------------
        static NES_INLINE Vec3  Max(const Vec3& left, const Vec3& right);

        //----------------------------------------------------------------------------------------------------
        /// @brief : Clamp each component of the vector between the min and max components.
        //----------------------------------------------------------------------------------------------------
        static NES_INLINE Vec3  Clamp(const Vec3& vec, const Vec3& min, const Vec3& max);

        //----------------------------------------------------------------------------------------------------
        /// @brief : Component-wise equality. Returns a vector with 0 for each component that is not equal.
        //----------------------------------------------------------------------------------------------------
        static NES_INLINE UVec4Reg Equals(const Vec3& left, const Vec3& right);

        //----------------------------------------------------------------------------------------------------
        /// @brief : Component-wise less than. Returns a vector with 0 for each component that is not less than.
        //----------------------------------------------------------------------------------------------------
        static NES_INLINE UVec4Reg Less(const Vec3& left, const Vec3& right);

        //----------------------------------------------------------------------------------------------------
        /// @brief : Component-wise less than or equal. Returns a vector with 0 for each component that is not less than or equal.
        //----------------------------------------------------------------------------------------------------
        static NES_INLINE UVec4Reg LessOrEqual(const Vec3& left, const Vec3& right);

        //----------------------------------------------------------------------------------------------------
        /// @brief : Component-wise greater than. Returns a vector with 0 for each component that is not greater than.
        //----------------------------------------------------------------------------------------------------
        static NES_INLINE UVec4Reg Greater(const Vec3& left, const Vec3& right);

        //----------------------------------------------------------------------------------------------------
        /// @brief : Component-wise greater than or equal. Returns a vector with 0 for each component that is not greater than or equal. 
        //----------------------------------------------------------------------------------------------------
        static NES_INLINE UVec4Reg GreaterOrEqual(const Vec3& left, const Vec3& right);

        //----------------------------------------------------------------------------------------------------
        /// @brief : Calculates mul1 * mul2 + add.
        //----------------------------------------------------------------------------------------------------
        static NES_INLINE Vec3  FusedMultiplyAdd(const Vec3& mul1, const Vec3& mul2, const Vec3& add);

        //----------------------------------------------------------------------------------------------------
        /// @brief : Component-wise select. Returns notSet when the highest bit of mask is 0, otherwise returns set.
        //----------------------------------------------------------------------------------------------------
        static NES_INLINE Vec3  Select(const Vec3& notSet, const Vec3& set, const UVec4Reg& mask);

        //----------------------------------------------------------------------------------------------------
        /// @brief : Component-wise logical Or.
        //----------------------------------------------------------------------------------------------------
        static NES_INLINE Vec3  Or(const Vec3& left, const Vec3& right);

        //----------------------------------------------------------------------------------------------------
        /// @brief : Component-wise logical Xor.
        //----------------------------------------------------------------------------------------------------
        static NES_INLINE Vec3  Xor(const Vec3& left, const Vec3& right);

        //----------------------------------------------------------------------------------------------------
        /// @brief : Component-wise logical And. 
        //----------------------------------------------------------------------------------------------------
        static NES_INLINE Vec3  And(const Vec3& left, const Vec3& right);
        
        //----------------------------------------------------------------------------------------------------
        /// @brief : Return the dot product between two vectors.
        //----------------------------------------------------------------------------------------------------
        static NES_INLINE float Dot(const Vec3 a, const Vec3 b)                         { return a.Dot(b); }

        //----------------------------------------------------------------------------------------------------
        ///	@brief : Calculate the cross-product between this and another vector.
        ///     The cross-product yields a vector that is perpendicular to both vectors.
        //----------------------------------------------------------------------------------------------------
        static NES_INLINE Vec3  Cross(const Vec3 a, const Vec3 b)                        { return a.Cross(b); }
        
        //----------------------------------------------------------------------------------------------------
        ///	@brief : Linearly interpolate between two vectors.
        ///	@param from : The Vector we are starting from.
        ///	@param to : The Vector we are going toward.
        ///	@param t : The percentage of the resulting vector will be between the two vectors. Should be a value between [0, 1].
        //----------------------------------------------------------------------------------------------------
        static NES_INLINE Vec3  Lerp(const Vec3& from, const Vec3& to, const float t);

        //----------------------------------------------------------------------------------------------------
        /// @brief : Get a unit vector based on the spherical coordinates.
        ///	@param theta : [0, PI] - the angle between the vector and the z-axis.
        ///	@param phi : [0, 2PI] - the angle in the xy-plane starting from the x-axis and rotating counter-clockwise
        ///     around the z-axis.
        //----------------------------------------------------------------------------------------------------
        static NES_INLINE Vec3  UnitSpherical(float theta, float phi);

        //----------------------------------------------------------------------------------------------------
        /// @brief : Compute the distance between two points.
        //----------------------------------------------------------------------------------------------------
        static NES_INLINE float Distance(const Vec3& a, const Vec3& b)                   { return (a - b).Length(); }

        //----------------------------------------------------------------------------------------------------
        /// @brief : Compute the squared distance between two points.
        //----------------------------------------------------------------------------------------------------
        static NES_INLINE float DistanceSqr(const Vec3& a, const Vec3& b)                { return (a - b).LengthSqr(); }

        //----------------------------------------------------------------------------------------------------
        /// @brief : Returns the angle (in radians) between the two vectors.
        //----------------------------------------------------------------------------------------------------
        static NES_INLINE float AngleBetween(const Vec3& a, const Vec3& b);

        //----------------------------------------------------------------------------------------------------
        /// @brief : Returns the angle (in degrees) between the two vectors.
        //----------------------------------------------------------------------------------------------------
        static NES_INLINE float AngleBetweenDegrees(const Vec3& a, const Vec3& b);

        //----------------------------------------------------------------------------------------------------
        /// @brief : The Scalar Triple Product is found by taking the cross-product between a and b, and then
        ///     using that result and getting the dot product with c: (a x b) * c. The resulting value represents
        ///     the *signed* volume of the parallelepiped formed by the three vectors.
        ///     - If the result is 0, then the three vectors are all coplanar.
        //----------------------------------------------------------------------------------------------------
        static NES_INLINE float ScalarTripleProduct(const Vec3& a, const Vec3& b, const Vec3& c)    { return Cross(a, b).Dot(c); }

        //----------------------------------------------------------------------------------------------------
        /// @brief : Returns true if the basis vectors x, y, and z are in a left-handed coordinate system.
        //----------------------------------------------------------------------------------------------------
        static NES_INLINE bool  IsLeftHanded(const Vec3& x, const Vec3& y, const Vec3& z)            { return ScalarTripleProduct(x, y, z) < 0.f; }
    };
}

#include "Vec3.inl"