// UVec4Reg.h
#pragma once
#include "Vec4Reg.h"

namespace nes
{
    //----------------------------------------------------------------------------------------------------
    /// @brief : Vector register class that stores 4 integers. 
    //----------------------------------------------------------------------------------------------------
    struct alignas (NES_VECTOR_ALIGNMENT) UVec4Reg
    {
    #if defined (NES_USE_SSE)
        using Type = __m128i;
    #else
        using Type = struct { uint32 m_data[4]; };
    #endif

        union
        {
            Type   m_value;                 /// SIMD (if enabled) type.
            uint32 m_u32[4];                /// Components in array form.
        };

        UVec4Reg() = default;
        UVec4Reg(const UVec4Reg&) = default;
        UVec4Reg& operator=(const UVec4Reg&) = default;

        /// Construct from a Register Value.
        NES_INLINE UVec4Reg(const Type value) : m_value(value) {}

        /// Construct from 4 integers.
        NES_INLINE UVec4Reg(const uint32 x, const uint32 y, const uint32 z, const uint32 w);

        /// Operators
        NES_INLINE bool             operator==(const UVec4Reg& other) const                 { return Equals(*this, other).TestAllTrue(); }
        NES_INLINE bool             operator!=(const UVec4Reg& other) const                 { return !(*this == other); }
        NES_INLINE uint32&          operator[](const size_t index);
        NES_INLINE uint32           operator[](const size_t index) const;
        NES_INLINE UVec4Reg         operator*(const UVec4Reg& other) const;
        NES_INLINE UVec4Reg&        operator*=(const UVec4Reg& other);
        NES_INLINE UVec4Reg         operator+(const UVec4Reg& other) const;
        NES_INLINE UVec4Reg&        operator+=(const UVec4Reg& other);

        NES_INLINE uint32           GetX() const;    
        NES_INLINE uint32           GetY() const;    
        NES_INLINE uint32           GetZ() const;    
        NES_INLINE uint32           GetW() const;
    
        NES_INLINE void             SetX(const uint32 x)                                    { m_u32[0] = x; }    
        NES_INLINE void             SetY(const uint32 y)                                    { m_u32[1] = y; }    
        NES_INLINE void             SetZ(const uint32 z)                                    { m_u32[2] = z; }    
        NES_INLINE void             SetW(const uint32 w)                                    { m_u32[3] = w; }
        NES_INLINE void             SetComponent(const size_t index, const uint32 value)    { NES_ASSERT(index < 4); m_u32[index] = value; }

        //----------------------------------------------------------------------------------------------------
        /// @brief : Set each component at once.
        //----------------------------------------------------------------------------------------------------
        NES_INLINE void             Set(const uint32_t x, const uint32_t y, const uint32_t z, const uint32_t w) { *this = UVec4Reg(x, y, z, w); } 

        //----------------------------------------------------------------------------------------------------
        /// @brief : To "Swizzle" a vector means to set the components equal the specified component value of the passed
        ///     in swizzle argument. Ex: Swizzle<ESwizzle::X, ESwizzle::X, ESwizzle::Y, ESwizzle::Y>() will set the XY components equal to
        ///     the current X value, and the ZW components equal to the current Y value.
        //----------------------------------------------------------------------------------------------------
        template <uint32 SwizzleX, uint32 SwizzleY, uint32 SwizzleZ, uint32 SwizzleW>
        NES_INLINE UVec4Reg         Swizzle() const;

        //----------------------------------------------------------------------------------------------------
        /// @brief : Count the number of components that are true. True is when the highest bit of a component
        ///     is set.
        //----------------------------------------------------------------------------------------------------
        NES_INLINE int              CountTrues() const;

        //----------------------------------------------------------------------------------------------------
        /// @brief : Store if X is true in bit 0, Y in bit 1, Z in bit 2, W in bit 3. True is when the highest
        ///     bit of a component is set.
        //----------------------------------------------------------------------------------------------------
        NES_INLINE int              GetTrues() const;
        
        //----------------------------------------------------------------------------------------------------
        /// @brief : Check if all components are true. True is when the hightest bit of a component is set. 
        //----------------------------------------------------------------------------------------------------
        NES_INLINE bool             TestAllTrue() const                                     { return GetTrues() == 0b1111; }

        //----------------------------------------------------------------------------------------------------
        /// @brief : Check if any of the X, Y, or Z components are true. True is when the hightest bit of a component is set. 
        //----------------------------------------------------------------------------------------------------
        NES_INLINE bool             TestAnyXYZTrue() const                                  { return (GetTrues() & 0b111) != 0; }

        //----------------------------------------------------------------------------------------------------
        /// @brief : Check if each of the X, Y, or Z components are true. True is when the hightest bit of a component is set. 
        //----------------------------------------------------------------------------------------------------
        NES_INLINE bool             TestAllXYZTrue() const                                  { return (GetTrues() & 0b111) == 0b111; }
        
        //----------------------------------------------------------------------------------------------------
        /// @brief : Convert each int component to a float. 
        //----------------------------------------------------------------------------------------------------
        NES_INLINE Vec4Reg          ToFloat() const;

        //----------------------------------------------------------------------------------------------------
        /// @brief : Reinterpret int vector register as a float register. Doesn't change the bits.
        //----------------------------------------------------------------------------------------------------
        NES_INLINE Vec4Reg          ReinterpretAsFloat() const;

        //----------------------------------------------------------------------------------------------------
        /// @brief : Store 4 ints to memory, aligned to 16 bytes.
        //----------------------------------------------------------------------------------------------------
        NES_INLINE void             StoreInt4Aligned(uint32* pOutValues) const; 

        //----------------------------------------------------------------------------------------------------
        /// @brief : Returns a register with all components equal to this register's X Component. 
        //----------------------------------------------------------------------------------------------------
        NES_INLINE UVec4Reg         SplatX() const;

        //----------------------------------------------------------------------------------------------------
        /// @brief : Returns a register with all components equal to this register's Y Component. 
        //----------------------------------------------------------------------------------------------------
        NES_INLINE UVec4Reg         SplatY() const;

        //----------------------------------------------------------------------------------------------------
        /// @brief : Returns a register with all components equal to this register's Z Component. 
        //----------------------------------------------------------------------------------------------------
        NES_INLINE UVec4Reg         SplatZ() const;

        //----------------------------------------------------------------------------------------------------
        /// @brief : Returns a register with all components equal to this register's W Component. 
        //----------------------------------------------------------------------------------------------------
        NES_INLINE UVec4Reg         SplatW() const;

        //----------------------------------------------------------------------------------------------------
        /// @brief : Shift all components by Count bits to the left (filling with zeros from the left). 
        //----------------------------------------------------------------------------------------------------
        template <const uint Count>
        NES_INLINE UVec4Reg         LogicalShiftLeft() const;

        //----------------------------------------------------------------------------------------------------
        /// @brief : Shift all components by Count bits to the right (filling with zeros from the right). 
        //----------------------------------------------------------------------------------------------------
        template <const uint Count>
        NES_INLINE UVec4Reg         LogicalShiftRight() const;

        //----------------------------------------------------------------------------------------------------
        /// @brief : Shift all components by Count bits to the right (shifting in the value of the highest bit).
        //----------------------------------------------------------------------------------------------------
        template <const uint Count>
        NES_INLINE UVec4Reg         ArithmeticShiftRight() const;

        //----------------------------------------------------------------------------------------------------
        /// @brief : Returns a register with all components equal to zero. 
        //----------------------------------------------------------------------------------------------------
        static NES_INLINE UVec4Reg  Zero();

        //----------------------------------------------------------------------------------------------------
        /// @brief : Creates a register with each component equal to the value.
        //----------------------------------------------------------------------------------------------------
        static NES_INLINE UVec4Reg  Replicate(const uint32 value);

        //----------------------------------------------------------------------------------------------------
        /// @brief : Load 4 uint32_t values into a Register.
        //----------------------------------------------------------------------------------------------------
        static NES_INLINE UVec4Reg  Load(const uint32* pValues);

        //----------------------------------------------------------------------------------------------------
        /// @brief : Store 4 uint32_t values from the Register into pOutValues.
        //----------------------------------------------------------------------------------------------------
        static NES_INLINE void      Store(const UVec4Reg& vec, uint32* pOutValues);

        //----------------------------------------------------------------------------------------------------
        /// @brief : Returns the minimum value of each component.
        //----------------------------------------------------------------------------------------------------
        static NES_INLINE UVec4Reg  Min(const UVec4Reg& a, const UVec4Reg& b);

        //----------------------------------------------------------------------------------------------------
        /// @brief : Returns the maximum value of each component.
        //----------------------------------------------------------------------------------------------------
        static NES_INLINE UVec4Reg  Max(const UVec4Reg& a, const UVec4Reg& b);

        //----------------------------------------------------------------------------------------------------
        /// @brief : Component-wise equal operation.
        //----------------------------------------------------------------------------------------------------
        static NES_INLINE UVec4Reg  Equals(const UVec4Reg& a, const UVec4Reg& b);

        //----------------------------------------------------------------------------------------------------
        /// @brief : Component-wise Or operation.
        //----------------------------------------------------------------------------------------------------
        static NES_INLINE UVec4Reg  Or(const UVec4Reg& a, const UVec4Reg& b);

        //----------------------------------------------------------------------------------------------------
        /// @brief : Component-wise And operation.
        //----------------------------------------------------------------------------------------------------
        static NES_INLINE UVec4Reg  And(const UVec4Reg& a, const UVec4Reg& b);

        //----------------------------------------------------------------------------------------------------
        /// @brief : Component-wise Not operation.
        //----------------------------------------------------------------------------------------------------
        static NES_INLINE UVec4Reg  Not(const UVec4Reg& value);

        //----------------------------------------------------------------------------------------------------
        /// @brief : Component-wise Xor operation.
        //----------------------------------------------------------------------------------------------------
        static NES_INLINE UVec4Reg  Xor(const UVec4Reg& a, const UVec4Reg& b);

        //----------------------------------------------------------------------------------------------------
        /// @brief : Component-wise select. Returns "notSet" component value when highest bit of "mask" == 0 and
        ///     "set" component value when the highest bit of "mask" == 1.
        //----------------------------------------------------------------------------------------------------
        static NES_INLINE UVec4Reg  Select(const UVec4Reg& notSet, const UVec4Reg& set, const UVec4Reg& mask);
        
        //----------------------------------------------------------------------------------------------------
        /// @brief : Sort the elements in "index" so that the values that correspond to trues in "vec" are the
        ///     first elements. The remaining elements are set to "vec.w".
        ///     Ex: if vec = (true, false, true, false) and index = (1, 2, 3, 4), the function returns
        ///     (1, 3, 4, 4).
        ///	@param value : Vector value to check for true/false.
        ///	@param index : The indices that you want to check and sort.
        //----------------------------------------------------------------------------------------------------
        static NES_INLINE UVec4Reg  SortTrue(const UVec4Reg& value, const UVec4Reg& index);

        //----------------------------------------------------------------------------------------------------
        /// @brief : Shift the elements so that the identifiers that correspond with the trues in "value" come first.
        ///	@param value : Values to test for true/false.
        ///	@param identifiers : The identifiers that are shifted, on return they are shifted.
        ///	@returns : The number of trues.
        //----------------------------------------------------------------------------------------------------
        static NES_INLINE int       CountAndSortTrues(const UVec4Reg& value, UVec4Reg& identifiers);
    };
}

#include "UVec4Reg.inl"