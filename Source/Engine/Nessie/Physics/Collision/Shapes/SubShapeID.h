// SubShapeID.h
#pragma once
#include "Core/Memory/Memory.h"
#include "Debug/Assert.h"

namespace nes
{
    //----------------------------------------------------------------------------------------------------
    //	NOTES:
    //		
    /// @brief : A sub shape ID contains a path to an element (usually a triangle or other primitive type) of
    ///     a compound shape. Each sub shape knows how many bits it needs to encode its ID, so it knows how many
    ///     bits to take from the sub shape ID.
    ///
    ///     For example,
    ///     * We have a CompoundShape A with 5 child shapes (identify sub shape using 3 bits AAA).
    ///     * One of its child shapes is a CompoundShape B which has 3 child shapes (identify sub shape using 2 bits BB).
    ///     * One of its child shapes is MeshShape C which contains enough triangles to need 7 bits to identify a
    ///       triangle (identify sub shape using 7 bits CCCCCCC - note that MeshShape is block based and sorts
    ///       triangles spatially, you can't assume that the first triangle will have a bit pattern 0000000).
    ///
    ///     The bit pattern of the sub shape ID to identify a triangle in MeshShape C will then be "CCCCCCCBBAAA".
    ///
    ///     A sub shape ID will become invalid when the structure of the shape changes. For example, if a
    ///     child shape is removed from the compound shape, the sub shape ID will no longer be valid.
    ///     This can be a problem when caching sub shape IDs from one frame to the next.
    //----------------------------------------------------------------------------------------------------
    class SubShapeID
    {
        friend class SubShapeIDCreator;
        
    public:
        NES_OVERRIDE_NEW_DELETE

        /// Underlying storage type.
        using Type = uint32_t;

        /// Type that is bigger than the underlying storage type for operations that would otherwise overflow.
        using BiggerType = uint64_t;
        static_assert(sizeof(BiggerType) > sizeof(Type));

        // How many bits we can store in this ID.
        static constexpr unsigned kMaxBits = 8 * sizeof(Type);

    private:
        /// An empty sub shape ID has all of its bits set.
        static constexpr Type kEmpty = ~static_cast<Type>(0);
        
        Type m_value = kEmpty;

        explicit SubShapeID(Type value) : m_value(value) {}
        
    public:
        SubShapeID() = default;

        inline bool operator==(const SubShapeID& other) const { return m_value == other.m_value; }
        inline bool operator!=(const SubShapeID& other) const { return m_value != other.m_value; }

        //----------------------------------------------------------------------------------------------------
        /// @brief : Get the next ID in the chain of IDs (pops parents before children). 
        //----------------------------------------------------------------------------------------------------
        inline Type PopID(const unsigned bits, SubShapeID& outRemainder) const
        {
            const Type maskBits = static_cast<Type>((static_cast<BiggerType>(1) << bits) - 1);
            // Fill the left side bits with 1 so that if there's no remainder, all bits will be set.
            // Note that we do this using a BiggerType since on intel 0xffffffff << 32 == 0xffffffff
            const Type fillBits = static_cast<Type>(static_cast<BiggerType>(kEmpty) << (kMaxBits - bits));
            const Type result = m_value & maskBits;
            outRemainder = SubShapeID(static_cast<Type>(static_cast<BiggerType>(m_value) >> bits) | fillBits);
            return result;
        }

        //----------------------------------------------------------------------------------------------------
        /// @brief : Get the value of the path to the sub shape ID. 
        //----------------------------------------------------------------------------------------------------
        inline Type GetValue() const            { return m_value; }

        //----------------------------------------------------------------------------------------------------
        /// @brief : Set the value of the Sub Shape ID. Only use if you know what you are doing! 
        //----------------------------------------------------------------------------------------------------
        inline void SetValue(const Type value)  { m_value = value; }

        //----------------------------------------------------------------------------------------------------
        /// @brief : Check if there are any bits of the sub shape ID left.
        /// @note : There is not a %100 guarantee as the sub shape ID could consist of all 1 bits. Use for
        ///     asserts only.
        //----------------------------------------------------------------------------------------------------
        inline bool IsEmpty() const             { return m_value == kEmpty; }

    private:
        //----------------------------------------------------------------------------------------------------
        /// @brief : Add an id at a particular position in the chain.
        /// @note: This should only be called by the SubShapeIDCreator.
        //----------------------------------------------------------------------------------------------------
        void        PushID(const unsigned value, const unsigned firstBitSet, const unsigned numBits)
        {
            // First clear the bits
            m_value &= ~(static_cast<Type>((static_cast<BiggerType>(1) << numBits) - 1) << firstBitSet);

            // Then set them to the new value.
            m_value |= value << firstBitSet;
        }
    };

    //----------------------------------------------------------------------------------------------------
    /// @brief : A Sub shape ID creator can be used to create a new sub shape ID by recursing through the
    ///     shape hierarchy and pushing the new ID's onto the chain.
    //----------------------------------------------------------------------------------------------------
    class SubShapeIDCreator
    {
        SubShapeID  m_id;
        unsigned    m_currentBit = 0;
        
    public:
        //----------------------------------------------------------------------------------------------------
        /// @brief : Add a new ID to the chain of ID's and return it. 
        //----------------------------------------------------------------------------------------------------
        SubShapeIDCreator PushID(const unsigned value, const unsigned numBits) const
        {
            NES_ASSERT(value < static_cast<SubShapeID::BiggerType>(1) << numBits);

            SubShapeIDCreator result = *this;
            result.m_id.PushID(value, m_currentBit, numBits);
            result.m_currentBit += numBits;
            
            NES_ASSERT(result.m_currentBit <= SubShapeID::kMaxBits);
            return result;
        }
        
        //----------------------------------------------------------------------------------------------------
        /// @brief : Get the resulting Sub Shape ID. 
        //----------------------------------------------------------------------------------------------------
        inline const SubShapeID& GetID() const { return m_id; }

        //----------------------------------------------------------------------------------------------------
        /// @brief : Get the number of bits written to the sub shape ID so far. 
        //----------------------------------------------------------------------------------------------------
        inline unsigned GetNumBitsWritten() const { return m_currentBit; }
    };
}
