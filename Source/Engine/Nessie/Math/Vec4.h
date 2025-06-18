// Vec4.h
#pragma once
#include "Scalar4.h"
#include "MathTypes.h"
#include "Detail/Swizzle.h"

namespace nes
{
    //----------------------------------------------------------------------------------------------------
    /// @brief : 4-component Vector class. 
    //----------------------------------------------------------------------------------------------------
    class alignas(NES_VECTOR_ALIGNMENT) Vec4
    {
    public:
        static constexpr size_t N = 4;
        
        float x;
        float y;
        float z;
        float w;
            
        Vec4() = default;
        Vec4(const Vec4& other) = default;
        Vec4& operator=(const Vec4& other) = default;

        /// Conversion Constructors
        explicit NES_INLINE     Vec4(const Vec3 vec);
        NES_INLINE              Vec4(const Vec3 vec, const float w);
        explicit NES_INLINE     Vec4(const float uniformValue);
        NES_INLINE              Vec4(const float x, const float y, const float z, const float w);
        explicit NES_INLINE     Vec4(const Float4& value);

        /// Operators
        NES_INLINE float        operator[](const size_t index) const                { NES_ASSERT(index < N); return *(&x + index); }
        NES_INLINE float&       operator[](const size_t index)                      { NES_ASSERT(index < N); return *(&x + index); }
        NES_INLINE bool         operator==(const Vec4& other) const;
        NES_INLINE bool         operator!=(const Vec4& other) const                 { return !(*this == other); }
        NES_INLINE bool         operator<(const Vec4& other) const                  { return x < other.x && y < other.y && z < other.z && w < other.w; }
        NES_INLINE bool         operator>(const Vec4& other) const                  { return x > other.x && y > other.y && z > other.z && w > other.w; }
        NES_INLINE bool         operator<=(const Vec4& other) const                 { return x <= other.x && y <= other.y && z <= other.z && w <= other.w; }
        NES_INLINE bool         operator>=(const Vec4& other) const                 { return x >= other.x && y >= other.y && z >= other.z && w >= other.w; }
        NES_INLINE Vec4         operator*(const Vec4& other) const;
        NES_INLINE Vec4&        operator*=(const Vec4& other);
        NES_INLINE Vec4         operator*(const float value) const;
        NES_INLINE Vec4&        operator*=(const float value);
        friend NES_INLINE Vec4  operator*(const float value, const Vec4& vec);
        NES_INLINE Vec4         operator/(const Vec4& other) const;
        NES_INLINE Vec4&        operator/=(const Vec4& other);
        NES_INLINE Vec4         operator/(const float value) const;
        NES_INLINE Vec4&        operator/=(const float value);
        NES_INLINE Vec4         operator+(const Vec4& other) const;
        NES_INLINE Vec4&        operator+=(const Vec4& other);
        NES_INLINE Vec4         operator-(const Vec4& other) const;
        NES_INLINE Vec4&        operator-=(const Vec4& other);
        NES_INLINE Vec4         operator-() const;
            
        //----------------------------------------------------------------------------------------------------
        /// @brief : Check if two vectors are close
        //----------------------------------------------------------------------------------------------------
        NES_INLINE bool         IsClose(const Vec4& other, const float maxDistSqr = 1.0e-12f) const;

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
        /// @brief : Create a vector with all components equal to this vector's W component. 
        //----------------------------------------------------------------------------------------------------
        NES_INLINE Vec4Reg      SplatW() const;

        //----------------------------------------------------------------------------------------------------
        /// @brief : To "Swizzle" a vector means to set the components equal to the specified component value of the passed
        ///     in swizzle argument. For example, Swizzle<0, 0, 1, 1>()
        ///     will set the XY components equal to the current X value, and the ZW components equal to the current Y value.
        //----------------------------------------------------------------------------------------------------
        template <uint32 SwizzleX, uint32 SwizzleY, uint32 SwizzleZ, uint32 SwizzleW>
        NES_INLINE Vec4         Swizzle() const;

        //----------------------------------------------------------------------------------------------------
        /// @brief : Return the absolute value of each component.
        //----------------------------------------------------------------------------------------------------
        NES_INLINE Vec4         Abs() const;
            
        //----------------------------------------------------------------------------------------------------
        /// @brief : Return the reciprocal (1 / value) of each component. 
        //----------------------------------------------------------------------------------------------------
        NES_INLINE Vec4         Reciprocal() const;

        //----------------------------------------------------------------------------------------------------
        /// @brief : Return the dot product stored across each component of the result vector.   
        //----------------------------------------------------------------------------------------------------
        NES_INLINE Vec4Reg      DotV(const Vec4& other) const;

        //----------------------------------------------------------------------------------------------------
        /// @brief : Return the dot product between this and another vector.
        //----------------------------------------------------------------------------------------------------
        NES_INLINE float        Dot(const Vec4& other) const;

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
        NES_INLINE Vec4&        Normalize();

        //----------------------------------------------------------------------------------------------------
        /// @brief : Get a normalized version of this vector. 
        //----------------------------------------------------------------------------------------------------
        NES_INLINE Vec4         Normalized() const;

        //----------------------------------------------------------------------------------------------------
        /// @brief : Store if X is negative in bit 0, Y in bit 1, Z in bit 2, W in bit 3. 
        //----------------------------------------------------------------------------------------------------
        NES_INLINE int          GetSignBits() const;

        //----------------------------------------------------------------------------------------------------
        /// @brief : Return the minimum value of X, Y, Z, W.  
        //----------------------------------------------------------------------------------------------------
        NES_INLINE float        MinComponent() const;

        //----------------------------------------------------------------------------------------------------
        /// @brief : Return the maximum value of X, Y, Z, W.  
        //----------------------------------------------------------------------------------------------------
        NES_INLINE float        MaxComponent() const;

        //----------------------------------------------------------------------------------------------------
        /// @brief : Component-wise square root.
        //----------------------------------------------------------------------------------------------------
        NES_INLINE Vec4         Sqrt() const;

        //----------------------------------------------------------------------------------------------------
        /// @brief : Returns a vector that contains the sign of each component (1.0 for positive, -1.0 for negative). 
        //----------------------------------------------------------------------------------------------------
        NES_INLINE Vec4         GetSign() const;

        //----------------------------------------------------------------------------------------------------
        /// @brief : Vector with all components set to zero. 
        //----------------------------------------------------------------------------------------------------
        static NES_INLINE Vec4  Zero();

        //----------------------------------------------------------------------------------------------------
        /// @brief : Vector with all components set to one.
        //----------------------------------------------------------------------------------------------------
        static NES_INLINE Vec4  One();

        //----------------------------------------------------------------------------------------------------
        /// @brief : Vector with all components set to NaN (Not a Number). 
        //----------------------------------------------------------------------------------------------------
        static NES_INLINE Vec4  NaN();

        //----------------------------------------------------------------------------------------------------
        /// @brief : Create a vector with all components to the specified value.
        //----------------------------------------------------------------------------------------------------
        static NES_INLINE Vec4  Replicate(const float value);

        //----------------------------------------------------------------------------------------------------
        /// @brief : Load a vector from a 4-element array.
        //----------------------------------------------------------------------------------------------------
        static NES_INLINE Vec4  LoadFloat4(const Float4* pFloats);

        //----------------------------------------------------------------------------------------------------
        /// @brief : Load a vector from a 4-element array, 16 byte aligned.
        //----------------------------------------------------------------------------------------------------
        static NES_INLINE Vec4  LoadFloat4Aligned(const Float4* pFloats);

        //----------------------------------------------------------------------------------------------------
        /// @brief : Gather 4 floats from memory at pBase + offsets[i] * Scale.
        //----------------------------------------------------------------------------------------------------
        template <const int Scale>
        static NES_INLINE Vec4  GatherFloat4(const float* pBase, const UVec4Reg& offsets);

        //----------------------------------------------------------------------------------------------------
        /// @brief : Creates a vector with the minimum value of each component.
        //----------------------------------------------------------------------------------------------------
        static NES_INLINE Vec4  Min(const Vec4& left, const Vec4& right);

        //----------------------------------------------------------------------------------------------------
        /// @brief : Creates a vector with the maximum value of each component.
        //----------------------------------------------------------------------------------------------------
        static NES_INLINE Vec4  Max(const Vec4& left, const Vec4& right);

        //----------------------------------------------------------------------------------------------------
        /// @brief : Component-wise equality. Returns a vector with 0 for each component that is not equal.
        //----------------------------------------------------------------------------------------------------
        static NES_INLINE UVec4Reg Equals(const Vec4& left, const Vec4& right);

        //----------------------------------------------------------------------------------------------------
        /// @brief : Component-wise less than. Returns a vector with 0 for each component that is not less than.
        //----------------------------------------------------------------------------------------------------
        static NES_INLINE UVec4Reg Less(const Vec4& left, const Vec4& right);

        //----------------------------------------------------------------------------------------------------
        /// @brief : Component-wise less than or equal. Returns a vector with 0 for each component that is not less than or equal.
        //----------------------------------------------------------------------------------------------------
        static NES_INLINE UVec4Reg LessOrEqual(const Vec4& left, const Vec4& right);

        //----------------------------------------------------------------------------------------------------
        /// @brief : Component-wise greater than. Returns a vector with 0 for each component that is not greater than.
        //----------------------------------------------------------------------------------------------------
        static NES_INLINE UVec4Reg Greater(const Vec4& left, const Vec4& right);

        //----------------------------------------------------------------------------------------------------
        /// @brief : Component-wise greater than or equal. Returns a vector with 0 for each component that is not greater than or equal. 
        //----------------------------------------------------------------------------------------------------
        static NES_INLINE UVec4Reg GreaterOrEqual(const Vec4& left, const Vec4& right);

        //----------------------------------------------------------------------------------------------------
        /// @brief : Calculates mul1 * mul2 + add.
        //----------------------------------------------------------------------------------------------------
        static NES_INLINE Vec4  FusedMultiplyAdd(const Vec4& mul1, const Vec4& mul2, const Vec4& add);

        //----------------------------------------------------------------------------------------------------
        /// @brief : Component-wise select: returns notSet when the highest bit of mask is 0, otherwise returns set.
        //----------------------------------------------------------------------------------------------------
        static NES_INLINE Vec4  Select(const Vec4 notSet, const Vec4 set, const UVec4Reg mask);

        //----------------------------------------------------------------------------------------------------
        /// @brief : Component-wise logical Or.
        //----------------------------------------------------------------------------------------------------
        static NES_INLINE Vec4  Or(const Vec4 left, const Vec4 right);

        //----------------------------------------------------------------------------------------------------
        /// @brief : Component-wise logical Xor.
        //----------------------------------------------------------------------------------------------------
        static NES_INLINE Vec4  Xor(const Vec4 left, const Vec4 right);

        //----------------------------------------------------------------------------------------------------
        /// @brief : Component-wise logical And. 
        //----------------------------------------------------------------------------------------------------
        static NES_INLINE Vec4  And(const Vec4 left, const Vec4 right);

        //----------------------------------------------------------------------------------------------------
        /// @brief : Sort the four elements of the value and sort the index at the same time.
        ///     Based on a sorting network: http://en.wikipedia.org/wiki/Sorting_network
        //----------------------------------------------------------------------------------------------------
        static NES_INLINE void  Sort4(Vec4& value, UVec4Reg& index);

        //----------------------------------------------------------------------------------------------------
        /// @brief : Reverse sort the four elements of the value (highest first) and sort the index at the same time.
        ///     Based on a sorting network: http://en.wikipedia.org/wiki/Sorting_network
        //----------------------------------------------------------------------------------------------------
        static NES_INLINE void  Sort4Reverse(Vec4& value, UVec4Reg& index);
    };
}

#include "Vec4.inl"