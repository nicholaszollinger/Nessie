// PhysicsUpdateErrorCodes.h
#pragma once
#include <cstdint>
#include <string>
#include "Nessie/Core/Macro.h"

namespace nes
{
    enum class EPhysicsUpdateErrorCode : uint32_t
    {
        None = 0,                               /// No Errors
        ManifoldCacheFull = NES_BIT(0),         /// The Manifold Cache is full, total number of contacts between bodies is too high. Some contacts were ignored. Increase maxContactConstraints in PhysicsSystem::Init. 
        BodyPairCacheFull = NES_BIT(1),         /// The BodyPair cache is full, this means that too many bodies contacted. Some contacts were ignored. Increase maxBodyPairs in PhysicsSystem::Init.
        ContactConstraintsFull = NES_BIT(2),    /// The ContactConstraints Buffer is full. Some contacts were ignored. Increase maxContactConstraints in PhysicsSystem::Init.
    };
    NES_DEFINE_BIT_OPERATIONS_FOR_ENUM(EPhysicsUpdateErrorCode);

    std::string ToString(const EPhysicsUpdateErrorCode errorCodes);
}
