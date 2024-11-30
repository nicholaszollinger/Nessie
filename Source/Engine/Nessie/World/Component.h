#pragma once
// Component.h
#include "Core/Generic/Concepts.h"
#include "Resources/Serializer_Member.h"

namespace nes
{
    template <typename Type>
    concept ComponentType = requires(Type value)
    {
        std::is_default_constructible_v<Type>;
        HasMemberSerializeFunctions<Type>;
    };
}
