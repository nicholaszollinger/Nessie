// BodyPair.h
#pragma once
#include "BodyID.h"
#include "Core/Memory/Memory.h"

namespace nes
{
    //----------------------------------------------------------------------------------------------------
    /// @brief : Struct that holds two Body IDs. 
    //----------------------------------------------------------------------------------------------------
    struct alignas(uint64_t) BodyPair
    {
        NES_OVERRIDE_NEW_DELETE

        BodyID      m_bodyA;
        BodyID      m_bodyB;

        BodyPair() = default;
        BodyPair(const BodyID a, const BodyID b) : m_bodyA(a), m_bodyB(b) {}

        bool        operator==(const BodyPair& other) const { return *reinterpret_cast<const uint64_t*>(this) == *reinterpret_cast<const uint64_t*>(&other); }
        bool        operator< (const BodyPair& other) const { return *reinterpret_cast<const uint64_t*>(this) < *reinterpret_cast<const uint64_t*>(&other); }

        //----------------------------------------------------------------------------------------------------
        /// @brief : [TODO]: Get the hash for this body pair. 
        //----------------------------------------------------------------------------------------------------
        uint64_t    GetHash() const;
    };

    static_assert(sizeof(BodyPair) == sizeof(uint64_t), "Mismatch in class size");
}
