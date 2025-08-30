// BodyPair.h
#pragma once
#include "BodyID.h"
#include "Nessie/Core/Hash.h"
#include "Nessie/Core/Memory/Memory.h"

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
        /// @brief : Get the hash for this body pair. 
        //----------------------------------------------------------------------------------------------------
        uint64      GetHash() const { return Hash64(*reinterpret_cast<const uint64*>(this)); }
    };

    static_assert(sizeof(BodyPair) == sizeof(uint64_t), "Mismatch in class size");
}
