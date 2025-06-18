// VectorRegisterF.h
#pragma once
#include "Math/Vec3.h"
#include "Math/Detail/Swizzle.h"

namespace nes
{
    using VectorRegisterF = Vec3;
}
//     //----------------------------------------------------------------------------------------------------
//     // [TODO]: This class might be put into the Vector3 and Vector4 classes to unify the API. Right now they
//     //      are disconnected and they shouldn't be really.
//     /// @brief : Register containing 4 floats. 
//     //----------------------------------------------------------------------------------------------------
//     struct VectorRegisterF
//     {
// #if defined(NES_USE_SSE)
//         using Type = __m128;
// #else
//         using Type = struct { float m_data[4]; };
// #endif
//
//         union
//         {
//             Type    m_value;
//             float   m_f32[4];
//         };
//
//         VectorRegisterF() = default;
//         VectorRegisterF(const VectorRegisterF&) = default;
//         VectorRegisterF& operator=(const VectorRegisterF&) = default;
//
//         /// Construct from a register value.
//         NES_INLINE VectorRegisterF(const Type& value) : m_value(value) {}
//
//         /// Construct from 4 floats.
//         NES_INLINE VectorRegisterF(const float x, const float y, const float z, const float w);
//
//         NES_INLINE VectorRegisterF(const Vector3& vec);
//
//         NES_INLINE bool             operator==(const VectorRegisterF& other) const;
//         NES_INLINE bool             operator!=(const VectorRegisterF& other) const { return !(*this == other); }
//         NES_INLINE VectorRegisterF  operator+(const VectorRegisterF& other) const;
//         NES_INLINE VectorRegisterF& operator+=(const VectorRegisterF& other);
//         NES_INLINE VectorRegisterF  operator-(const VectorRegisterF& other) const;
//         NES_INLINE VectorRegisterF  operator-() const;
//         NES_INLINE VectorRegisterF& operator-=(const VectorRegisterF& other);
//         NES_INLINE VectorRegisterF  operator*(const float value) const;
//         NES_INLINE VectorRegisterF& operator*=(const float value);
//         NES_INLINE VectorRegisterF  operator*(const VectorRegisterF other) const;
//         NES_INLINE VectorRegisterF& operator*=(const VectorRegisterF other);
//         NES_INLINE VectorRegisterF  operator/(const VectorRegisterF other) const;
//         NES_INLINE VectorRegisterF& operator/=(const VectorRegisterF other);
//         NES_INLINE VectorRegisterF  operator/(const float value) const;
//         NES_INLINE VectorRegisterF& operator/=(const float value);
//         NES_INLINE float&           operator[](const uint32_t index);
//         NES_INLINE float            operator[](const uint32_t index) const;
//         friend NES_INLINE VectorRegisterF operator*(const float value, const VectorRegisterF& vec);
//         
//         NES_INLINE float GetX() const;
//         NES_INLINE float GetY() const;
//         NES_INLINE float GetZ() const;
//         NES_INLINE float GetW() const;
//         
//         NES_INLINE void Set(const float x, const float y, const float z, const float w) { *this = VectorRegisterF(x, y, z, w); }
//         NES_INLINE void SetX(const float value)                                         { m_f32[0] = value; }
//         NES_INLINE void SetY(const float value)                                         { m_f32[1] = value; }
//         NES_INLINE void SetZ(const float value)                                         { m_f32[2] = value; }
//         NES_INLINE void SetW(const float value)                                         { m_f32[3] = value; }
//
//         //----------------------------------------------------------------------------------------------------
//         /// @brief : Returns whether any of the components contain NaN. 
//         //----------------------------------------------------------------------------------------------------
//         NES_INLINE bool                     IsNaN() const;
//
//         //----------------------------------------------------------------------------------------------------
//         /// @brief : Returns the cross product : this x other.
//         //----------------------------------------------------------------------------------------------------
//         NES_INLINE VectorRegisterF          Cross(const VectorRegisterF& other) const;
//
//         //----------------------------------------------------------------------------------------------------
//         /// @brief : Returns the length (or magnitude) of the vector. 
//         //----------------------------------------------------------------------------------------------------
//         NES_INLINE float                    Length() const;
//
//         //----------------------------------------------------------------------------------------------------
//         /// @brief : Returns the squared length (or squared magnitude) of the vector. 
//         //----------------------------------------------------------------------------------------------------
//         NES_INLINE float                    SquaredLength() const;
//
//         //----------------------------------------------------------------------------------------------------
//         /// @brief : Stores if X is negative in bit 0, Y in bit 1, Z in bit 2, and W in bit 3.
//         //----------------------------------------------------------------------------------------------------
//         NES_INLINE int                      GetSignBits() const;
//
//         //----------------------------------------------------------------------------------------------------
//         /// @brief : To "Swizzle" a vector means to set the components equal the specified component value of the passed
//         ///     in swizzle argument. For example: Swizzle<0, 0, 1, 1>() will set the XY components equal to
//         ///     the current X value, and the ZW components equal to the current Y value.
//         /// @note : All Swizzle arguments must be in the range [0, 3].
//         //----------------------------------------------------------------------------------------------------
//         template <uint32 SwizzleX, uint32 SwizzleY, uint32 SwizzleZ, uint32 SwizzleW>
//         NES_INLINE VectorRegisterF          Swizzle() const;
//
//         //----------------------------------------------------------------------------------------------------
//         /// @brief : Convert each float component to an integer. 
//         //----------------------------------------------------------------------------------------------------
//         NES_INLINE VectorRegisterUint       ConvertToInt() const;
//
//         //----------------------------------------------------------------------------------------------------
//         /// @brief : Reinterpret float vector register as an Integer register. Doesn't change the bits.
//         //----------------------------------------------------------------------------------------------------
//         NES_INLINE VectorRegisterUint       ReinterpretAsInt() const;
//
//         //----------------------------------------------------------------------------------------------------
//         /// @brief : Returns a register with all components equal to this register's X Component. 
//         //----------------------------------------------------------------------------------------------------
//         NES_INLINE VectorRegisterF          SplatX() const;
//
//         //----------------------------------------------------------------------------------------------------
//         /// @brief : Returns a register with all components equal to this register's Y Component. 
//         //----------------------------------------------------------------------------------------------------
//         NES_INLINE VectorRegisterF          SplatY() const;
//
//         //----------------------------------------------------------------------------------------------------
//         /// @brief : Returns a register with all components equal to this register's Z Component. 
//         //----------------------------------------------------------------------------------------------------
//         NES_INLINE VectorRegisterF          SplatZ() const;
//
//         //----------------------------------------------------------------------------------------------------
//         /// @brief : Returns a register with all components equal to this register's W Component. 
//         //----------------------------------------------------------------------------------------------------
//         NES_INLINE VectorRegisterF          SplatW() const;
//
//         //----------------------------------------------------------------------------------------------------
//         /// @brief : Dot Product. 
//         //----------------------------------------------------------------------------------------------------
//         NES_INLINE float                    Dot(const VectorRegisterF& other) const;
//
//         //----------------------------------------------------------------------------------------------------
//         /// @brief : Dot Product using the X, Y, Z components.
//         //----------------------------------------------------------------------------------------------------
//         NES_INLINE VectorRegisterF          DotV(const VectorRegisterF& other) const;
//
//         //----------------------------------------------------------------------------------------------------
//         /// @brief : Dot Product using the X, Y, Z and W components.
//         //----------------------------------------------------------------------------------------------------
//         NES_INLINE VectorRegisterF          DotV4(const VectorRegisterF& other) const;
//         
//         //----------------------------------------------------------------------------------------------------
//         /// @brief : Returns a register with all components equal to zero. 
//         //----------------------------------------------------------------------------------------------------
//         static NES_INLINE VectorRegisterF   Zero();
//
//         //----------------------------------------------------------------------------------------------------
//         /// @brief : Returns a register with all components equal to one. 
//         //----------------------------------------------------------------------------------------------------
//         static NES_INLINE VectorRegisterF   Unit();
//
//         //----------------------------------------------------------------------------------------------------
//         /// @brief : Returns a register with all components equal to Nan. 
//         //----------------------------------------------------------------------------------------------------
//         static NES_INLINE VectorRegisterF   Nan();
//
//         //----------------------------------------------------------------------------------------------------
//         /// @brief : Stores the single value into each component.
//         //----------------------------------------------------------------------------------------------------
//         static NES_INLINE VectorRegisterF   Replicate(const float value);
//
//         //----------------------------------------------------------------------------------------------------
//         /// @brief : Load 4 floats into a Register.
//         //----------------------------------------------------------------------------------------------------
//         static NES_INLINE VectorRegisterF   Load(const float* pValues);
//
//         //----------------------------------------------------------------------------------------------------
//         /// @brief : Store the register's value into 4 floats.
//         //----------------------------------------------------------------------------------------------------
//         static NES_INLINE void              Store(const VectorRegisterF& vec, float* pOutValues);
//
//         //----------------------------------------------------------------------------------------------------
//         /// @brief : Returns the minimum value of each of the components.
//         //----------------------------------------------------------------------------------------------------
//         static NES_INLINE VectorRegisterF   Min(const VectorRegisterF& a, const VectorRegisterF& b);
//
//         //----------------------------------------------------------------------------------------------------
//         /// @brief : Returns the maximum value of each of the components.
//         //----------------------------------------------------------------------------------------------------
//         static NES_INLINE VectorRegisterF   Max(const VectorRegisterF& a, const VectorRegisterF& b);
//
//         //----------------------------------------------------------------------------------------------------
//         /// @brief : Component-wise logical Or.
//         //----------------------------------------------------------------------------------------------------        
//         static NES_INLINE VectorRegisterF   Or(const VectorRegisterF& a, const VectorRegisterF& b);
//         
//         //----------------------------------------------------------------------------------------------------
//         /// @brief : Component-wise logical And.
//         //----------------------------------------------------------------------------------------------------        
//         static NES_INLINE VectorRegisterF   And(const VectorRegisterF& a, const VectorRegisterF& b);
//
//         //----------------------------------------------------------------------------------------------------
//         /// @brief : Component-wise logical Xor.
//         //----------------------------------------------------------------------------------------------------        
//         static NES_INLINE VectorRegisterF   Xor(const VectorRegisterF& a, const VectorRegisterF& b);
//
//         //----------------------------------------------------------------------------------------------------
//         /// @brief : Component-wise equal operation. 
//         //----------------------------------------------------------------------------------------------------
//         static NES_INLINE VectorRegisterUint Equals(const VectorRegisterF& a, const VectorRegisterF& b);
//
//         //----------------------------------------------------------------------------------------------------
//         /// @brief : Component-wise Greater-than operation. 
//         //----------------------------------------------------------------------------------------------------
//         static NES_INLINE VectorRegisterUint Greater(const VectorRegisterF& a, const VectorRegisterF& b);
//
//         //----------------------------------------------------------------------------------------------------
//         /// @brief : Component-wise Greater-than or equal operation. 
//         //----------------------------------------------------------------------------------------------------
//         static NES_INLINE VectorRegisterUint GreaterOrEqual(const VectorRegisterF& a, const VectorRegisterF& b);
//
//         //----------------------------------------------------------------------------------------------------
//         /// @brief : Component-wise Less-than operation. 
//         //----------------------------------------------------------------------------------------------------
//         static NES_INLINE VectorRegisterUint Less(const VectorRegisterF& a, const VectorRegisterF& b);
//
//         //----------------------------------------------------------------------------------------------------
//         /// @brief : Component-wise Less-than or equal operation. 
//         //----------------------------------------------------------------------------------------------------
//         static NES_INLINE VectorRegisterUint LesserOrEqual(const VectorRegisterF& a, const VectorRegisterF& b);
//
//         //----------------------------------------------------------------------------------------------------
//         /// @brief : Component-wise select. Returns "notSet" value when highest bit in "control" = 0, a "set"
//         ///     value when the highest bit in "control" = 1. 
//         //----------------------------------------------------------------------------------------------------
//         static NES_INLINE VectorRegisterF    Select(const VectorRegisterF& notSet, const VectorRegisterF& set, const VectorRegisterUint& control);
//     };
// }
//
#include "VectorRegisterF.inl"