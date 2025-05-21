// PhysicsUpdateErrorCodes.cpp
#include "PhysicsUpdateErrorCodes.h"

namespace nes
{
    std::string ToString(const PhysicsUpdateErrorCode errorCodes)
    {
        std::string result;

        if (static_cast<uint32_t>(errorCodes & PhysicsUpdateErrorCode::ManifoldCacheFull))
        {
            result += "PhysicsUpdateError: The Manifold Cache is full, total number of contacts between bodies is too high. Some contacts were ignored. Increase maxContactConstraints in PhysicsSystem::Init.\n";
        }

        if (static_cast<uint32_t>(errorCodes & PhysicsUpdateErrorCode::BodyPairCacheFull))
        {
            result += "PhysicsUpdateError: The BodyPair cache is full, this means that too many bodies contacted. Some contacts were ignored. Increase maxBodyPairs in PhysicsSystem::Init.\n";
        }

        if (static_cast<uint32_t>(errorCodes & PhysicsUpdateErrorCode::ContactConstraintsFull))
        {
            result += "PhysicsUpdateError: The ContactConstraints Buffer is full. Some contacts were ignored. Increase maxContactConstraints in PhysicsSystem::Init.\n";
        }

        return result;
    }
}