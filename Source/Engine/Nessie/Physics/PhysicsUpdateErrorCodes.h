// PhysicsUpdateErrorCodes.h
#pragma once
#include <cstdint>
#include <string>
#include "Core/Generic/Bitmask.h"

namespace nes
{
    enum class PhysicsUpdateErrorCode : uint32_t
    {
        None = 0,                           /// No Errors
        ManifoldCacheFull = 1 << 0,         /// The Manifold Cache is full, total number of contacts between bodies is too high. Some contacts were ignored. Increase maxContactConstraints in PhysicsSystem::Init. 
        BodyPairCacheFull = 2 << 0,         /// The BodyPair cache is full, this means that too many bodies contacted. Some contacts were ignored. Increase maxBodyPairs in PhysicsSystem::Init.
        ContactConstraintsFull = 3 << 0,    /// The ContactConstraints Buffer is full. Some contacts were ignored. Increase maxContactConstraints in PhysicsSystem::Init.
    };

    NES_DEFINE_BIT_OPERTATIONS_FOR_ENUM(PhysicsUpdateErrorCode);

    std::string ToString(const PhysicsUpdateErrorCode errorCodes);
}
