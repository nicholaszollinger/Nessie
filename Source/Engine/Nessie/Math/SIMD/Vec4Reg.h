// Vec4Reg.h
#pragma once
#include "Math/MathTypes.h"
#include "Math/Scalar3.h"
#include "Math/Scalar4.h"
#include "Math/Detail/Swizzle.h"

namespace nes
{
    //----------------------------------------------------------------------------------------------------
    /// @brief : Vector Register class that stores 4 floats. 
    //----------------------------------------------------------------------------------------------------
    struct Vec4Reg
    {
    #if defined (NES_USE_SSE)
        using Type = __m128;
    #else
        using Type = struct { float m_data[4]; };
    #endif
        
        union
        {
            Type m_value;
            float m_f32[4];
        };

        Vec4Reg() = default; /// Uninitialized for performance.
        Vec4Reg(const Vec4Reg& reg) = default;
        Vec4Reg& operator=(const Vec4Reg& reg) = default;

        /// Conversion Constructors
        constexpr NES_INLINE        Vec4Reg(const Type value) : m_value(value) {}

        /// Constructs the register with the Z and W components both set to the Vector's Z value.
        explicit NES_INLINE         Vec4Reg(const Vec3 vec);
        NES_INLINE                  Vec4Reg(const Vec3 vec, const float w);
        NES_INLINE                  Vec4Reg(const Vec4 vec);
        NES_INLINE                  Vec4Reg(const float uniformValue); 
        NES_INLINE                  Vec4Reg(const float x, const float y, const float z); 
        NES_INLINE                  Vec4Reg(const float x, const float y, const float z, const float w); 
        explicit NES_INLINE         Vec4Reg(const Float3& value) : Vec4Reg(value.x, value.y, value.z, value.z) {}
        explicit NES_INLINE         Vec4Reg(const Float4& value);

        /// Operators
        NES_INLINE float            operator[](const size_t index) const                    { NES_ASSERT(index < 4); return m_f32[index]; }
        NES_INLINE float&           operator[](const size_t index)                          { NES_ASSERT(index < 4); return m_f32[index]; }
        NES_INLINE bool             operator==(const Vec4Reg& other) const;
        NES_INLINE bool             operator!=(const Vec4Reg& other) const                  { return !(*this == other); }
        NES_INLINE bool             operator<(const Vec4Reg& other) const                   { return GetX() < other.GetX() && GetY() < other.GetY() && GetZ() < other.GetZ() && GetW() < other.GetW(); }
        NES_INLINE bool             operator>(const Vec4Reg& other) const                   { return GetX() > other.GetX() && GetY() > other.GetY() && GetZ() > other.GetZ() && GetW() > other.GetW(); }
        NES_INLINE bool             operator<=(const Vec4Reg& other) const                  { return GetX() <= other.GetX() && GetY() <= other.GetY() && GetZ() <= other.GetZ() && GetW() <= other.GetW(); }
        NES_INLINE bool             operator>=(const Vec4Reg& other) const                  { return GetX() >= other.GetX() && GetY() >= other.GetY() && GetZ() >= other.GetZ() && GetW() >= other.GetW(); }
        NES_INLINE Vec4Reg          operator*(const Vec4Reg& other) const;
        NES_INLINE Vec4Reg&         operator*=(const Vec4Reg& other);
        NES_INLINE Vec4Reg          operator*(const float value) const;
        NES_INLINE Vec4Reg&         operator*=(const float value);
        friend NES_INLINE Vec4Reg   operator*(const float value, const Vec4Reg& vec);
        NES_INLINE Vec4Reg          operator/(const Vec4Reg& other) const;
        NES_INLINE Vec4Reg&         operator/=(const Vec4Reg& other);
        NES_INLINE Vec4Reg          operator/(const float value) const;
        NES_INLINE Vec4Reg&         operator/=(const float value);
        NES_INLINE Vec4Reg          operator+(const Vec4Reg& other) const;
        NES_INLINE Vec4Reg&         operator+=(const Vec4Reg& other);
        NES_INLINE Vec4Reg          operator-(const Vec4Reg& other) const;
        NES_INLINE Vec4Reg&         operator-=(const Vec4Reg& other);
        NES_INLINE Vec4Reg          operator-() const;

        /// Get individual components
        NES_INLINE float            GetX() const;
        NES_INLINE float            GetY() const;
        NES_INLINE float            GetZ() const;
        NES_INLINE float            GetW() const;

        /// Set individual components
        NES_INLINE void             SetX(const float x)                                                { m_f32[0] = x; }    
        NES_INLINE void             SetY(const float y)                                                { m_f32[1] = y; }    
        NES_INLINE void             SetZ(const float z)                                                { m_f32[2] = z; }    
        NES_INLINE void             SetW(const float w)                                                { m_f32[3] = w; }
        NES_INLINE void             SetComponent(const size_t index, const float value)                { NES_ASSERT(index < 4); m_f32[index] = value; }

        //----------------------------------------------------------------------------------------------------
        /// @brief : Set all the components.
        //----------------------------------------------------------------------------------------------------
        NES_INLINE void             Set(const float x, const float y, const float z, const float w) { *this = Vec4Reg(x, y, z, w); }

        //----------------------------------------------------------------------------------------------------
        /// @brief : Convert to a Vec3. 
        //----------------------------------------------------------------------------------------------------
        NES_INLINE Vec3             ToVec3() const;

        //----------------------------------------------------------------------------------------------------
        /// @brief : Convert to a Vec4. 
        //----------------------------------------------------------------------------------------------------
        NES_INLINE Vec4             ToVec4() const;
        
        //----------------------------------------------------------------------------------------------------
        /// @brief : Check if two vectors are close
        //----------------------------------------------------------------------------------------------------
        NES_INLINE bool             IsClose(const Vec4Reg& other, const float maxDistSqr = 1.0e-12f) const;

        //----------------------------------------------------------------------------------------------------
        /// @brief : Check if the vector is normalized (Length = 1.0).
        //----------------------------------------------------------------------------------------------------
        NES_INLINE bool             IsNormalized(const float tolerance = 1.0e-6f) const;

        //----------------------------------------------------------------------------------------------------
        /// @brief : Check if *any* components are NaN (not a number). 
        //----------------------------------------------------------------------------------------------------
        NES_INLINE bool             IsNaN() const;

        //----------------------------------------------------------------------------------------------------
        /// @brief : Create a vector with all components equal to this vector's X component. 
        //----------------------------------------------------------------------------------------------------
        NES_INLINE Vec4Reg          SplatX() const;

        //----------------------------------------------------------------------------------------------------
        /// @brief : Create a vector with all components equal to this vector's Y component. 
        //----------------------------------------------------------------------------------------------------
        NES_INLINE Vec4Reg          SplatY() const;
        
        //----------------------------------------------------------------------------------------------------
        /// @brief : Create a vector with all components equal to this vector's Z component. 
        //----------------------------------------------------------------------------------------------------
        NES_INLINE Vec4Reg          SplatZ() const;

        //----------------------------------------------------------------------------------------------------
        /// @brief : Create a vector with all components equal to this vector's W component. 
        //----------------------------------------------------------------------------------------------------
        NES_INLINE Vec4Reg          SplatW() const;

        //----------------------------------------------------------------------------------------------------
        /// @brief : To "Swizzle" a vector means to set the components equal to the specified component value of the passed
        ///     in swizzle argument. For example, Swizzle<0, 0, 1, 1>()
        ///     will set the XY components equal to the current X value, and the ZW components equal to the current Y value.
        //----------------------------------------------------------------------------------------------------
        template <uint32 SwizzleX, uint32 SwizzleY, uint32 SwizzleZ, uint32 SwizzleW>
        NES_INLINE Vec4Reg          Swizzle() const;

        //----------------------------------------------------------------------------------------------------
        /// @brief : Return the absolute value of each component.
        //----------------------------------------------------------------------------------------------------
        NES_INLINE Vec4Reg          Abs() const;
            
        //----------------------------------------------------------------------------------------------------
        /// @brief : Return the reciprocal (1 / value) of each component. 
        //----------------------------------------------------------------------------------------------------
        NES_INLINE Vec4Reg          Reciprocal() const                              { return One() / m_value; }

        //----------------------------------------------------------------------------------------------------
        /// @brief : Return the dot product stored across each component of the result vector.   
        //----------------------------------------------------------------------------------------------------
        NES_INLINE Vec4Reg          DotV(const Vec4Reg& other) const;

        //----------------------------------------------------------------------------------------------------
        /// @brief : Return the dot product between this and another vector.
        //----------------------------------------------------------------------------------------------------
        NES_INLINE float            Dot(const Vec4Reg& other) const;

        //----------------------------------------------------------------------------------------------------
        /// @brief : Returns the squared length (magnitude) of the vector. 
        //----------------------------------------------------------------------------------------------------
        NES_INLINE float            LengthSqr() const;

        //----------------------------------------------------------------------------------------------------
        /// @brief : Returns the length (magnitude) of the vector. 
        //----------------------------------------------------------------------------------------------------
        NES_INLINE float            Length() const;

        //----------------------------------------------------------------------------------------------------
        /// @brief : Normalize this vector. 
        //----------------------------------------------------------------------------------------------------
        NES_INLINE Vec4Reg&         Normalize();

        //----------------------------------------------------------------------------------------------------
        /// @brief : Get a normalized version of this vector. 
        //----------------------------------------------------------------------------------------------------
        NES_INLINE Vec4Reg          Normalized() const;

        //----------------------------------------------------------------------------------------------------
        /// @brief : Store the component values into pOutFloats.
        //----------------------------------------------------------------------------------------------------
        NES_INLINE void             StoreFloat4(Float4* pOutFloats) const;

        //----------------------------------------------------------------------------------------------------
        /// @brief : Store the component values into a 4-component vector.
        //----------------------------------------------------------------------------------------------------
        NES_INLINE void             StoreVec4(Vec4* pOutVec) const;

        //----------------------------------------------------------------------------------------------------
        /// @brief : Convert each component to an int.
        //----------------------------------------------------------------------------------------------------
        NES_INLINE UVec4Reg         ToInt() const;

        //----------------------------------------------------------------------------------------------------
        /// @brief : Reinterpret Vec4Reg as a UVec4Reg (doesn't change the bits). 
        //----------------------------------------------------------------------------------------------------
        NES_INLINE UVec4Reg         ReinterpretAsInt() const;

        //----------------------------------------------------------------------------------------------------
        /// @brief : Store if X is negative in bit 0, Y in bit 1, Z in bit 2, W in bit 3. 
        //----------------------------------------------------------------------------------------------------
        NES_INLINE int              GetSignBits() const;

        //----------------------------------------------------------------------------------------------------
        /// @brief : Return the minimum value of X, Y, Z, W.  
        //----------------------------------------------------------------------------------------------------
        NES_INLINE float            MinComponent() const;

        //----------------------------------------------------------------------------------------------------
        /// @brief : Return the maximum value of X, Y, Z, W.  
        //----------------------------------------------------------------------------------------------------
        NES_INLINE float            MaxComponent() const;

        //----------------------------------------------------------------------------------------------------
        /// @brief : Component-wise square root.
        //----------------------------------------------------------------------------------------------------
        NES_INLINE Vec4Reg          Sqrt() const;

        //----------------------------------------------------------------------------------------------------
        /// @brief : Returns a vector that contains the sign of each component (1.0 for positive, -1.0 for negative). 
        //----------------------------------------------------------------------------------------------------
        NES_INLINE Vec4Reg          GetSign() const;

        //----------------------------------------------------------------------------------------------------
        /// @brief : Calculate the sine and cosine of each component (input as radians). 
        //----------------------------------------------------------------------------------------------------
        inline void                 SinCos(Vec4Reg& outSin, Vec4Reg& outCos) const;

        //----------------------------------------------------------------------------------------------------
        /// @brief : Calculate the tangent for each component (input as radians).
        //----------------------------------------------------------------------------------------------------
        inline Vec4Reg              Tan() const;

        //----------------------------------------------------------------------------------------------------
        /// @brief : Calculate the arc sine for each element of this vector (returns value in the range [-PI / 2, PI / 2])
        /// @note : All input values will be clamped to the range [-1, 1], and this function will not return NaNs like std::asin
        //----------------------------------------------------------------------------------------------------
        inline Vec4Reg              ASin() const;

        //----------------------------------------------------------------------------------------------------
        /// @brief : Calculate the arc cosine for each element of this vector (returns value in the range [0, PI])
        /// @note : All input values will be clamped to the range [-1, 1] and this function will not return NaNs like std::acos
        //----------------------------------------------------------------------------------------------------
        inline Vec4Reg              ACos() const;

        //----------------------------------------------------------------------------------------------------
        /// @brief : Calculate the arc tangent for each element of this vector (returns value in the range [-PI / 2, PI / 2])
        //----------------------------------------------------------------------------------------------------
        inline Vec4Reg              ATan() const;

        //----------------------------------------------------------------------------------------------------
        /// @brief : Calculate the arc tangent of y / x using the signs of the arguments to determine the correct quadrant (returns value in the range [-PI, PI]) 
        //----------------------------------------------------------------------------------------------------
        inline static Vec4Reg       ATan2(const Vec4Reg& y, const Vec4Reg& x);

        //----------------------------------------------------------------------------------------------------
        /// @brief : Vector with all components set to zero. 
        //----------------------------------------------------------------------------------------------------
        static NES_INLINE Vec4Reg   Zero();

        //----------------------------------------------------------------------------------------------------
        /// @brief : Vector with all components set to one.
        //----------------------------------------------------------------------------------------------------
        static NES_INLINE Vec4Reg   One()                                       { return Replicate(1.f); }

        //----------------------------------------------------------------------------------------------------
        /// @brief : Vector with all components set to NaN (Not a Number). 
        //----------------------------------------------------------------------------------------------------
        static NES_INLINE Vec4Reg   NaN()                                       { return Replicate(std::numeric_limits<float>::quiet_NaN()); }

        //----------------------------------------------------------------------------------------------------
        /// @brief : Create a vector with all components to the specified value.
        //----------------------------------------------------------------------------------------------------
        static NES_INLINE Vec4Reg   Replicate(const float value);

        //----------------------------------------------------------------------------------------------------
        /// @brief : Calculate the length (magnitude) of a 3-component vector.
        //----------------------------------------------------------------------------------------------------
        static NES_INLINE float     Length3(const Vec3 vec);

        //----------------------------------------------------------------------------------------------------
        /// @brief : Calculate the squared length (magnitude) of a 3-component vector.
        //----------------------------------------------------------------------------------------------------
        static NES_INLINE float     LengthSqr3(const Vec3 vec);

        //----------------------------------------------------------------------------------------------------
        /// @brief : Returns the normalized vector, or the zeroValue if the length of this vector is zero.
        //----------------------------------------------------------------------------------------------------
        static NES_INLINE Vec3      NormalizedOr3(const Vec3 vec, const Vec3 zeroValue);

        //----------------------------------------------------------------------------------------------------
        ///	@brief : Calculate the cross-product between two 3-component vectors.
        ///     The cross-product yields a vector that is perpendicular to both vectors.
        //----------------------------------------------------------------------------------------------------
        static NES_INLINE Vec3      Cross3(const Vec3 a, const Vec3 b);

        //----------------------------------------------------------------------------------------------------
        /// @brief : Create a vector register from a 4-component vector.
        //----------------------------------------------------------------------------------------------------
        static NES_INLINE Vec4Reg   LoadVec4(const Vec4* pVec);

        //----------------------------------------------------------------------------------------------------
        /// @brief : Load a vector from a 4-element array.
        //----------------------------------------------------------------------------------------------------
        static NES_INLINE Vec4Reg   LoadFloat4(const Float4* pFloats);

        //----------------------------------------------------------------------------------------------------
        /// @brief : Load a vector from a 4-element array, 16 byte aligned.
        //----------------------------------------------------------------------------------------------------
        static NES_INLINE Vec4Reg   LoadFloat4Aligned(const Float4* pFloats);

        //----------------------------------------------------------------------------------------------------
        /// @brief : Load a vector from a 3-element array (reads an extra 32 bits which it doesn't use).
        //----------------------------------------------------------------------------------------------------
        static NES_INLINE Vec4Reg   LoadFloat3Unsafe(const Float3& value);

        //----------------------------------------------------------------------------------------------------
        /// @brief : Load a vector from a 3-element array (reads an extra 32 bits which it doesn't use).
        //----------------------------------------------------------------------------------------------------
        static NES_INLINE Vec4Reg   LoadVec3Unsafe(const Vec3& value);

        //----------------------------------------------------------------------------------------------------
        /// @brief : Gather 4 floats from memory at pBase + offsets[i] * Scale.
        //----------------------------------------------------------------------------------------------------
        template <const int Scale>
        static NES_INLINE Vec4Reg   GatherFloat4(const float* pBase, const UVec4Reg& offsets);

        //----------------------------------------------------------------------------------------------------
        /// @brief : Creates a vector with the minimum value of each component.
        //----------------------------------------------------------------------------------------------------
        static NES_INLINE Vec4Reg   Min(const Vec4Reg& left, const Vec4Reg& right);

        //----------------------------------------------------------------------------------------------------
        /// @brief : Creates a vector with the maximum value of each component.
        //----------------------------------------------------------------------------------------------------
        static NES_INLINE Vec4Reg   Max(const Vec4Reg& left, const Vec4Reg& right);

        //----------------------------------------------------------------------------------------------------
        /// @brief : Component-wise equality. Returns a vector with 0 for each component that is not equal.
        //----------------------------------------------------------------------------------------------------
        static NES_INLINE UVec4Reg  Equals(const Vec4Reg& left, const Vec4Reg& right);

        //----------------------------------------------------------------------------------------------------
        /// @brief : Component-wise less than. Returns a vector with 0 for each component that is not less than.
        //----------------------------------------------------------------------------------------------------
        static NES_INLINE UVec4Reg  Less(const Vec4Reg& left, const Vec4Reg& right);

        //----------------------------------------------------------------------------------------------------
        /// @brief : Component-wise less than or equal. Returns a vector with 0 for each component that is not less than or equal.
        //----------------------------------------------------------------------------------------------------
        static NES_INLINE UVec4Reg  LessOrEqual(const Vec4Reg& left, const Vec4Reg& right);

        //----------------------------------------------------------------------------------------------------
        /// @brief : Component-wise greater than. Returns a vector with 0 for each component that is not greater than.
        //----------------------------------------------------------------------------------------------------
        static NES_INLINE UVec4Reg  Greater(const Vec4Reg& left, const Vec4Reg& right);

        //----------------------------------------------------------------------------------------------------
        /// @brief : Component-wise greater than or equal. Returns a vector with 0 for each component that is not greater than or equal. 
        //----------------------------------------------------------------------------------------------------
        static NES_INLINE UVec4Reg  GreaterOrEqual(const Vec4Reg& left, const Vec4Reg& right);

        //----------------------------------------------------------------------------------------------------
        /// @brief : Calculates mul1 * mul2 + add.
        //----------------------------------------------------------------------------------------------------
        static NES_INLINE Vec4Reg   FusedMultiplyAdd(const Vec4Reg& mul1, const Vec4Reg& mul2, const Vec4Reg& add);

        //----------------------------------------------------------------------------------------------------
        /// @brief : Component-wise select: returns notSet when the highest bit of mask is 0, otherwise returns set.
        //----------------------------------------------------------------------------------------------------
        static NES_INLINE Vec4Reg   Select(const Vec4Reg notSet, const Vec4Reg set, const UVec4Reg mask);

        //----------------------------------------------------------------------------------------------------
        /// @brief : Component-wise logical Or.
        //----------------------------------------------------------------------------------------------------
        static NES_INLINE Vec4Reg   Or(const Vec4Reg left, const Vec4Reg right);

        //----------------------------------------------------------------------------------------------------
        /// @brief : Component-wise logical Xor.
        //----------------------------------------------------------------------------------------------------
        static NES_INLINE Vec4Reg   Xor(const Vec4Reg left, const Vec4Reg right);

        //----------------------------------------------------------------------------------------------------
        /// @brief : Component-wise logical And. 
        //----------------------------------------------------------------------------------------------------
        static NES_INLINE Vec4Reg   And(const Vec4Reg left, const Vec4Reg right);

        //----------------------------------------------------------------------------------------------------
        /// @brief : Sort the four elements of the value and sort the index at the same time.
        ///     Based on a sorting network: http://en.wikipedia.org/wiki/Sorting_network
        //----------------------------------------------------------------------------------------------------
        static NES_INLINE void      Sort4(Vec4Reg& value, UVec4Reg& index);

        //----------------------------------------------------------------------------------------------------
        /// @brief : Reverse sort the four elements of the value (highest first) and sort the index at the same time.
        ///     Based on a sorting network: http://en.wikipedia.org/wiki/Sorting_network
        //----------------------------------------------------------------------------------------------------
        static NES_INLINE void      Sort4Reverse(Vec4Reg& value, UVec4Reg& index);

        //----------------------------------------------------------------------------------------------------
        /// @brief : Helper that asserts that W is equal to Z, so dividing by it should not generate dividing by 0
        ///     when using the register as a 3 component vector.
        //----------------------------------------------------------------------------------------------------
        NES_INLINE void             CheckW() const;

        //----------------------------------------------------------------------------------------------------
        /// @brief : Helper function that ensures that the Z component is replicated to the W component to
        ///     prevent dividing by zero when using the register as a 3 component vector.
        //----------------------------------------------------------------------------------------------------
        static NES_INLINE Type      FixW(Type value);
    };
}

#include "Vec4Reg.inl"