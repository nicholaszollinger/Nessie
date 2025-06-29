// BodyID.h
#pragma once
#include "Debug/Assert.h"

namespace nes
{
    //----------------------------------------------------------------------------------------------------
    //	NOTES:
    //  This functions basically the same as my GenerationalID, but it uses reserves a bit within the ID for
    //  use in the BroadPhase layer.
    //  Structure: Lower 23 bits: Index Value. Next 8 bits: Generation Value. Highest Bit: BroadPhase bit.
    //		
    /// @brief : ID of a Body. The underlying value packs in both an Index which maps to the array of Bodies
    ///     that it is located in, and a Generation value. An external BodyID would be considered invalid
    ///     if the Generation value of the Body in the internal array does not match.
    //----------------------------------------------------------------------------------------------------
    class BodyID
    {
    public:
        static constexpr uint32         kInvalidBodyID  = 0xffffffff;   /// The value for an invalid BodyID
        static constexpr uint32         kBroadPhaseBit  = 0x80000000;   /// This bit is used by the Broadphase to determine if a NodeID points to a Body or Node.
        static constexpr uint32         kMaxBodyIndex   = 0x7fffff;     /// Maximum value for a Body Index, also the maximum number of bodies - 1.
        static constexpr uint8          kMaxGeneration  = 0xff;         /// Maximum value for the Generation value.
        static constexpr unsigned int   kGenerationNumberShift = 23;    /// Number of bits to shift to get the Generation value.

        /// Default constructor creates an invalid ID.
        constexpr                       BodyID() : m_id(kInvalidBodyID) {}

        /// Construct from the combined Index and Generation value.
        explicit constexpr              BodyID(const uint32 id)
            : m_id(id)
        {
            // Ensure that the BroadPhaseBit is zero, meaning that this is *not* a Node,
            // or, this is an invalid BodyID.
            NES_ASSERT((id & kBroadPhaseBit) == 0 || id == kInvalidBodyID);
        }
        
        explicit constexpr              BodyID(const uint32 index, const uint8 generation)
            : m_id((static_cast<uint32>(generation) << kGenerationNumberShift) | index)
        {
            // Ensure that the index does not interfere with the broadphase bit and generation value.
            NES_ASSERT(index < kMaxBodyIndex);
        }

        /// Comparison Operators
        constexpr bool                  operator==(const BodyID& right) const   { return m_id == right.m_id; } 
        constexpr bool                  operator!=(const BodyID& right) const   { return m_id != right.m_id; } 
        constexpr bool                  operator<(const BodyID& right) const    { return m_id < right.m_id; } 
        constexpr bool                  operator>(const BodyID& right) const    { return m_id > right.m_id; } 
        
        //----------------------------------------------------------------------------------------------------
        /// @brief : Get the Index in the Body array.
        //----------------------------------------------------------------------------------------------------
        constexpr uint32                GetIndex() const                        { return m_id & kMaxBodyIndex; }
        
        //----------------------------------------------------------------------------------------------------
        /// @brief :  Get the Generation value of a Body.
        /// The Generation value can be used to check if a BodyID with the same Body index has been reused by new Body.
        /// It is mainly used in multithreaded situations where a body is removed and its body index is immediately reused
        /// by a Body created from another thread.
        /// Functions querying the BroadPhase can (after acquiring a BodyLock) detect that the Body has been removed
        /// (we assume that this won't happen more than 128 times in a row).
        //----------------------------------------------------------------------------------------------------
        constexpr uint8                 GetGeneration() const                   { return static_cast<uint8>(m_id >> kGenerationNumberShift); }
        
        //----------------------------------------------------------------------------------------------------
        /// @brief : Returns the Sequence and Generation values packed into a single uint32.
        //----------------------------------------------------------------------------------------------------
        constexpr uint32                GetIndexAndGeneration() const           { return m_id; }

        //----------------------------------------------------------------------------------------------------
        /// @brief : Check if this ID is valid. 
        //----------------------------------------------------------------------------------------------------
        constexpr bool                  IsValid() const                         { return m_id != kInvalidBodyID; }

    private:
        uint32                          m_id;
    };
}
