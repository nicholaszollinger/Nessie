// VectorRegisterUint.h
#pragma once
#include "Math/UVec4.h"

namespace nes
{
    using VectorRegisterUint = UVec4;   
}

//
// namespace nes
// {
//     struct VectorRegisterUint
//     {
// #if defined(NES_USE_SSE)
//         using Type = __m128i;
// #else
//         using Type = struct { uint32_t m_data[4]; };
// #endif
//
//         union
//         {
//             Type        m_value;
//             uint32_t    m_u32[4];
//         };
//
//         VectorRegisterUint() = default;
//         VectorRegisterUint(const VectorRegisterUint&) = default;
//         VectorRegisterUint& operator=(const VectorRegisterUint&) = default;
//
//         /// Construct from a Register Value.
//         NES_INLINE VectorRegisterUint(const Type value) : m_value(value) {}
//
//         /// Construct from 4 integers.
//         NES_INLINE VectorRegisterUint(const uint32_t x, const uint32_t y, const uint32_t z, const uint32_t w);
//         
//         NES_INLINE bool         operator==(const VectorRegisterUint& other) const;
//         NES_INLINE bool         operator!=(const VectorRegisterUint& other) const { return !(*this == other); }
//         NES_INLINE uint32_t&    operator[](const uint32_t index);
//         NES_INLINE uint32_t     operator[](const uint32_t index) const;
//         
//         NES_INLINE uint32_t GetX() const; 
//         NES_INLINE uint32_t GetY() const; 
//         NES_INLINE uint32_t GetZ() const;
//         NES_INLINE uint32_t GetW() const;
//
//         NES_INLINE void Set(const uint32_t x, const uint32_t y, const uint32_t z, const uint32_t w) { *this = VectorRegisterUint(x, y, z, w); } 
//         NES_INLINE void SetX(const uint32_t value)                                                  { m_u32[0] = value; } 
//         NES_INLINE void SetY(const uint32_t value)                                                  { m_u32[0] = value; } 
//         NES_INLINE void SetZ(const uint32_t value)                                                  { m_u32[0] = value; }
//         NES_INLINE void SetW(const uint32_t value)                                                  { m_u32[0] = value; }
//
//         //----------------------------------------------------------------------------------------------------
//         /// @brief : To "Swizzle" a vector means to set the components equal the specified component value of the passed
//         ///     in swizzle argument. For example: Swizzle<0, 0, 1, 1>() will set the XY components equal to
//         ///     the current X value, and the ZW components equal to the current Y value.
//         /// @note : All Swizzle arguments must be in the range [0, 3].
//         //----------------------------------------------------------------------------------------------------
//         template <uint32_t SwizzleX, uint32_t SwizzleY, uint32_t SwizzleZ, uint32_t SwizzleW>
//         NES_INLINE VectorRegisterUint Swizzle() const;
//
//         //----------------------------------------------------------------------------------------------------
//         /// @brief : Count the number of components that are true. True is when the highest bit of a component
//         ///     is set.
//         //----------------------------------------------------------------------------------------------------
//         NES_INLINE int CountTrues() const;
//
//         //----------------------------------------------------------------------------------------------------
//         /// @brief : Store if X is true in bit 0, Y in bit 1, Z in bit 2, W in bit 3. True is when the highest
//         ///     bit of a component is set.
//         //----------------------------------------------------------------------------------------------------
//         NES_INLINE int GetTrues() const;
//         
//         //----------------------------------------------------------------------------------------------------
//         /// @brief : Check if all components are true. True is when the hightest bit of a component is set. 
//         //----------------------------------------------------------------------------------------------------
//         NES_INLINE bool TestAllTrue() const;
//
//         //----------------------------------------------------------------------------------------------------
//         /// @brief : Check if any of the X, Y, or Z components are true. True is when the hightest bit of a component is set. 
//         //----------------------------------------------------------------------------------------------------
//         NES_INLINE bool TestAnyXYZTrue() const;
//
//         //----------------------------------------------------------------------------------------------------
//         /// @brief : Check if each of the X, Y, or Z components are true. True is when the hightest bit of a component is set. 
//         //----------------------------------------------------------------------------------------------------
//         NES_INLINE bool TestAllXYZTrue() const;
//         
//         //----------------------------------------------------------------------------------------------------
//         /// @brief : Convert each int component to a float. 
//         //----------------------------------------------------------------------------------------------------
//         NES_INLINE VectorRegisterF ConvertToFloat() const;
//
//         //----------------------------------------------------------------------------------------------------
//         /// @brief : Reinterpret int vector register as a float register. Doesn't change the bits.
//         //----------------------------------------------------------------------------------------------------
//         NES_INLINE VectorRegisterF ReinterpretAsFloat() const;
//
//         //----------------------------------------------------------------------------------------------------
//         /// @brief : Returns a register with all components equal to this register's X Component. 
//         //----------------------------------------------------------------------------------------------------
//         NES_INLINE VectorRegisterUint SplatX() const;
//
//         //----------------------------------------------------------------------------------------------------
//         /// @brief : Returns a register with all components equal to this register's Y Component. 
//         //----------------------------------------------------------------------------------------------------
//         NES_INLINE VectorRegisterUint SplatY() const;
//
//         //----------------------------------------------------------------------------------------------------
//         /// @brief : Returns a register with all components equal to this register's Z Component. 
//         //----------------------------------------------------------------------------------------------------
//         NES_INLINE VectorRegisterUint SplatZ() const;
//
//         //----------------------------------------------------------------------------------------------------
//         /// @brief : Returns a register with all components equal to this register's W Component. 
//         //----------------------------------------------------------------------------------------------------
//         NES_INLINE VectorRegisterUint SplatW() const;
//
//         //----------------------------------------------------------------------------------------------------
//         /// @brief : Returns a register with all components equal to zero. 
//         //----------------------------------------------------------------------------------------------------
//         static NES_INLINE VectorRegisterUint Zero();
//
//         //----------------------------------------------------------------------------------------------------
//         /// @brief : Creates a register with each component equal to the value.
//         //----------------------------------------------------------------------------------------------------
//         static NES_INLINE VectorRegisterUint Replicate(const uint32_t value);
//
//         //----------------------------------------------------------------------------------------------------
//         /// @brief : Load 4 uint32_t values into a Register.
//         //----------------------------------------------------------------------------------------------------
//         static NES_INLINE VectorRegisterUint Load(const uint32_t* pValues);
//
//         //----------------------------------------------------------------------------------------------------
//         /// @brief : Store 4 uint32_t values from the Register into pOutValues.
//         //----------------------------------------------------------------------------------------------------
//         static NES_INLINE void              Store(const VectorRegisterUint& vec, uint32_t* pOutValues);
//
//         //----------------------------------------------------------------------------------------------------
//         /// @brief : Returns the minimum value of each of the components.
//         //----------------------------------------------------------------------------------------------------
//         static NES_INLINE VectorRegisterUint Min(const VectorRegisterUint& a, const VectorRegisterUint& b);
//
//         //----------------------------------------------------------------------------------------------------
//         /// @brief : Returns the maximum value of each of the components.
//         //----------------------------------------------------------------------------------------------------
//         static NES_INLINE VectorRegisterUint Max(const VectorRegisterUint& a, const VectorRegisterUint& b);
//
//         //----------------------------------------------------------------------------------------------------
//         /// @brief : Component-wise equal operation.
//         //----------------------------------------------------------------------------------------------------
//         static NES_INLINE VectorRegisterUint Equals(const VectorRegisterUint& a, const VectorRegisterUint& b);
//
//         //----------------------------------------------------------------------------------------------------
//         /// @brief : Component-wise Or operation.
//         //----------------------------------------------------------------------------------------------------
//         static NES_INLINE VectorRegisterUint Or(const VectorRegisterUint& a, const VectorRegisterUint& b);
//
//         //----------------------------------------------------------------------------------------------------
//         /// @brief : Component-wise And operation.
//         //----------------------------------------------------------------------------------------------------
//         static NES_INLINE VectorRegisterUint And(const VectorRegisterUint& a, const VectorRegisterUint& b);
//
//         //----------------------------------------------------------------------------------------------------
//         /// @brief : Component-wise Not operation.
//         //----------------------------------------------------------------------------------------------------
//         static NES_INLINE VectorRegisterUint Not(const VectorRegisterUint& value);
//
//         //----------------------------------------------------------------------------------------------------
//         /// @brief : Component-wise Xor operation.
//         //----------------------------------------------------------------------------------------------------
//         static NES_INLINE VectorRegisterUint Xor(const VectorRegisterUint& a, const VectorRegisterUint& b);
//
//         //----------------------------------------------------------------------------------------------------
//         //	NOTES:
//         //		
//         /// @brief : Component-wise select. Returns "notSet" component value when hightest bit of "control" == 0 and
//         ///     "set" component value when the highest bit of "control" == 1.
//         ///	@returns : 
//         //----------------------------------------------------------------------------------------------------
//         static NES_INLINE VectorRegisterUint Select(const VectorRegisterUint& notSet, const VectorRegisterUint& set, const VectorRegisterUint& control);
//         
//         //----------------------------------------------------------------------------------------------------
//         /// @brief : Sorts elements in "index" so that the values that correspond to trues in "vec" are the
//         ///     first elements. The remaining elements are set to "vec.w".
//         ///     i.e. if vec = (true, false, true, false) and index = (1, 2, 3, 4) the function returns
//         ///     (1, 3, 4, 4).
//         ///	@param value : Vector value to check for true/false.
//         ///	@param index : The indices that you want to check & sort.
//         //----------------------------------------------------------------------------------------------------
//         static NES_INLINE VectorRegisterUint SortTrue(const VectorRegisterUint& value, const VectorRegisterUint& index);
//
//         //----------------------------------------------------------------------------------------------------
//         /// @brief : Shift the elements so that the identifiers that correspond with the trues in "value" come first.
//         ///	@param value : Values to test for true/false.
//         ///	@param identifiers : The identifiers that are shifted, on return they are shifted.
//         ///	@returns : The number of trues.
//         //----------------------------------------------------------------------------------------------------
//         static NES_INLINE int                CountAndSortTrues(const VectorRegisterUint& value, VectorRegisterUint& identifiers);
//     };
// }
//
// #include "VectorRegisterUint.inl"
