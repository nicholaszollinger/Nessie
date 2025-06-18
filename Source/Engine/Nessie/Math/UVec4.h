// UVec4.h
#pragma once
// #include "Vec4.h"
//
// namespace nes
// {
//     //----------------------------------------------------------------------------------------------------
//     /// @brief : 4-component unsigned integer vector. 
//     //----------------------------------------------------------------------------------------------------
//     class alignas (NES_VECTOR_ALIGNMENT) UVec4
//     {
//     public:
//         static constexpr size_t N = 4;
//
//         uint32 x;
//         uint32 y;
//         uint32 z;
//         uint32 w;
//         
//         UVec4() = default;
//         UVec4(const UVec4&) = default;
//         UVec4& operator=(const UVec4&) = default;
//         
//         /// Construct from 4 integers.
//         NES_INLINE UVec4(const uint32 x, const uint32 y, const uint32 z, const uint32 w);
//
//         /// Operators
//         NES_INLINE bool         operator==(const UVec4& other) const;
//         NES_INLINE bool         operator!=(const UVec4& other) const { return !(*this == other); }
//         NES_INLINE uint32&      operator[](const size_t index);
//         NES_INLINE uint32       operator[](const size_t index) const;
//         NES_INLINE UVec4        operator*(const UVec4& other) const;
//         NES_INLINE UVec4&       operator*=(const UVec4& other);
//         NES_INLINE UVec4        operator+(const UVec4& other) const;
//         NES_INLINE UVec4&       operator+=(const UVec4& other);
//
//         //----------------------------------------------------------------------------------------------------
//         /// @brief : To "Swizzle" a vector means to set the components equal the specified component value of the passed
//         ///     in swizzle argument. Ex: Swizzle<ESwizzle::X, ESwizzle::X, ESwizzle::Y, ESwizzle::Y>() will set the XY components equal to
//         ///     the current X value, and the ZW components equal to the current Y value.
//         //----------------------------------------------------------------------------------------------------
//         template <uint32 SwizzleX, uint32 SwizzleY, uint32 SwizzleZ, uint32 SwizzleW>
//         NES_INLINE UVec4        Swizzle() const;
//
//         // //----------------------------------------------------------------------------------------------------
//         // /// @brief : Count the number of components that are true. True is when the highest bit of a component
//         // ///     is set.
//         // //----------------------------------------------------------------------------------------------------
//         // NES_INLINE int          CountTrues() const;
//         //
//         // //----------------------------------------------------------------------------------------------------
//         // /// @brief : Store if X is true in bit 0, Y in bit 1, Z in bit 2, W in bit 3. True is when the highest
//         // ///     bit of a component is set.
//         // //----------------------------------------------------------------------------------------------------
//         // NES_INLINE int          GetTrues() const;
//         //
//         // //----------------------------------------------------------------------------------------------------
//         // /// @brief : Check if all components are true. True is when the hightest bit of a component is set. 
//         // //----------------------------------------------------------------------------------------------------
//         // NES_INLINE bool         TestAllTrue() const;
//         //
//         // //----------------------------------------------------------------------------------------------------
//         // /// @brief : Check if any of the X, Y, or Z components are true. True is when the hightest bit of a component is set. 
//         // //----------------------------------------------------------------------------------------------------
//         // NES_INLINE bool         TestAnyXYZTrue() const;
//         //
//         // //----------------------------------------------------------------------------------------------------
//         // /// @brief : Check if each of the X, Y, or Z components are true. True is when the hightest bit of a component is set. 
//         // //----------------------------------------------------------------------------------------------------
//         // NES_INLINE bool         TestAllXYZTrue() const;
//         
//         // //----------------------------------------------------------------------------------------------------
//         // /// @brief : Convert each int component to a float. 
//         // //----------------------------------------------------------------------------------------------------
//         // NES_INLINE Vec4         ToFloat() const;
//         //
//         // //----------------------------------------------------------------------------------------------------
//         // /// @brief : Reinterpret int vector register as a float register. Doesn't change the bits.
//         // //----------------------------------------------------------------------------------------------------
//         // NES_INLINE Vec4         ReinterpretAsFloat() const;
//
//         //----------------------------------------------------------------------------------------------------
//         /// @brief : Returns a register with all components equal to this register's X Component. 
//         //----------------------------------------------------------------------------------------------------
//         NES_INLINE UVec4Reg        SplatX() const;
//
//         //----------------------------------------------------------------------------------------------------
//         /// @brief : Returns a register with all components equal to this register's Y Component. 
//         //----------------------------------------------------------------------------------------------------
//         NES_INLINE UVec4Reg        SplatY() const;
//
//         //----------------------------------------------------------------------------------------------------
//         /// @brief : Returns a register with all components equal to this register's Z Component. 
//         //----------------------------------------------------------------------------------------------------
//         NES_INLINE UVec4Reg        SplatZ() const;
//
//         //----------------------------------------------------------------------------------------------------
//         /// @brief : Returns a register with all components equal to this register's W Component. 
//         //----------------------------------------------------------------------------------------------------
//         NES_INLINE UVec4Reg        SplatW() const;
//
//         //----------------------------------------------------------------------------------------------------
//         /// @brief : Shift all components by Count bits to the left (filling with zeros from the left). 
//         //----------------------------------------------------------------------------------------------------
//         template <const uint Count>
//         NES_INLINE UVec4        LogicalShiftLeft() const;
//
//         //----------------------------------------------------------------------------------------------------
//         /// @brief : Shift all components by Count bits to the right (filling with zeros from the right). 
//         //----------------------------------------------------------------------------------------------------
//         template <const uint Count>
//         NES_INLINE UVec4        LogicalShiftRight() const;
//
//         //----------------------------------------------------------------------------------------------------
//         /// @brief : Shift all components by Count bits to the right (shifting in the value of the highest bit).
//         //----------------------------------------------------------------------------------------------------
//         template <const uint Count>
//         NES_INLINE UVec4        ArithmeticShiftRight() const;
//
//         //----------------------------------------------------------------------------------------------------
//         /// @brief : Returns a register with all components equal to zero. 
//         //----------------------------------------------------------------------------------------------------
//         static NES_INLINE UVec4 Zero();
//
//         //----------------------------------------------------------------------------------------------------
//         /// @brief : Creates a register with each component equal to the value.
//         //----------------------------------------------------------------------------------------------------
//         static NES_INLINE UVec4 Replicate(const uint32 value);
//
//         //----------------------------------------------------------------------------------------------------
//         /// @brief : Load 4 uint32_t values into a Register.
//         //----------------------------------------------------------------------------------------------------
//         static NES_INLINE UVec4 Load(const uint32* pValues);
//
//         //----------------------------------------------------------------------------------------------------
//         /// @brief : Store 4 uint32_t values from the Register into pOutValues.
//         //----------------------------------------------------------------------------------------------------
//         static NES_INLINE void Store(const UVec4& vec, uint32* pOutValues);
//
//         //----------------------------------------------------------------------------------------------------
//         /// @brief : Returns the minimum value of each component.
//         //----------------------------------------------------------------------------------------------------
//         static NES_INLINE UVec4 Min(const UVec4& a, const UVec4& b);
//
//         //----------------------------------------------------------------------------------------------------
//         /// @brief : Returns the maximum value of each component.
//         //----------------------------------------------------------------------------------------------------
//         static NES_INLINE UVec4 Max(const UVec4& a, const UVec4& b);
//
//         //----------------------------------------------------------------------------------------------------
//         /// @brief : Component-wise equal operation.
//         //----------------------------------------------------------------------------------------------------
//         static NES_INLINE UVec4Reg Equals(const UVec4& a, const UVec4& b);
//
//         //----------------------------------------------------------------------------------------------------
//         /// @brief : Component-wise Or operation.
//         //----------------------------------------------------------------------------------------------------
//         static NES_INLINE UVec4 Or(const UVec4& a, const UVec4& b);
//
//         //----------------------------------------------------------------------------------------------------
//         /// @brief : Component-wise And operation.
//         //----------------------------------------------------------------------------------------------------
//         static NES_INLINE UVec4 And(const UVec4& a, const UVec4& b);
//
//         //----------------------------------------------------------------------------------------------------
//         /// @brief : Component-wise Not operation.
//         //----------------------------------------------------------------------------------------------------
//         static NES_INLINE UVec4 Not(const UVec4& value);
//
//         //----------------------------------------------------------------------------------------------------
//         /// @brief : Component-wise Xor operation.
//         //----------------------------------------------------------------------------------------------------
//         static NES_INLINE UVec4 Xor(const UVec4& a, const UVec4& b);
//
//         //----------------------------------------------------------------------------------------------------
//         /// @brief : Component-wise select. Returns "notSet" component value when highest bit of "mask" == 0 and
//         ///     "set" component value when the highest bit of "mask" == 1.
//         //----------------------------------------------------------------------------------------------------
//         static NES_INLINE UVec4 Select(const UVec4& notSet, const UVec4& set, const UVec4& mask);
//         
//         //----------------------------------------------------------------------------------------------------
//         /// @brief : Sort the elements in "index" so that the values that correspond to trues in "vec" are the
//         ///     first elements. The remaining elements are set to "vec.w".
//         ///     Ex: if vec = (true, false, true, false) and index = (1, 2, 3, 4), the function returns
//         ///     (1, 3, 4, 4).
//         ///	@param value : Vector value to check for true/false.
//         ///	@param index : The indices that you want to check and sort.
//         //----------------------------------------------------------------------------------------------------
//         static NES_INLINE UVec4 SortTrue(const UVec4& value, const UVec4& index);
//
//         //----------------------------------------------------------------------------------------------------
//         /// @brief : Shift the elements so that the identifiers that correspond with the trues in "value" come first.
//         ///	@param value : Values to test for true/false.
//         ///	@param identifiers : The identifiers that are shifted, on return they are shifted.
//         ///	@returns : The number of trues.
//         //----------------------------------------------------------------------------------------------------
//         static NES_INLINE int   CountAndSortTrues(const UVec4& value, UVec4& identifiers);
//     };
// }
//
// #include "UVec4.inl"
