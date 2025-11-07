// YamlSerializers.h
#pragma once
#include "YamlCore.h"

namespace nes
{
    template <EnumType Type>
    struct Serializer<YAML::Emitter, YAML::Node, Type>
    {
        using UnderlyingType = std::underlying_type_t<Type>;
        
        static void Serialize(YAML::Emitter& out, const Type& value)
        {
            if constexpr (std::same_as<UnderlyingType, uint8>)
            {
                // Cast to ensure a scalar value, otherwise it will be output as a char.
                out << static_cast<uint>(static_cast<UnderlyingType>(value));
            }
            else if constexpr (std::same_as<UnderlyingType, char>)
            {
                // Cast to ensure a scalar value, otherwise it will be output as a char.
                out << static_cast<int>(static_cast<UnderlyingType>(value));
            }
            else
            {
                out << static_cast<UnderlyingType>(value);
            }
        }

        static void Deserialize(const YAML::Node& in, Type& value, const Type& defaultValue = {})
        {
            value = static_cast<Type>(in.as<UnderlyingType>(static_cast<UnderlyingType>(defaultValue)));
        }
    };

    // Special string serializer, forcing double-quoted formatting.
    template <>
    struct Serializer<YAML::Emitter, YAML::Node, std::string>
    {
        static void Serialize(YAML::Emitter& out, const std::string& value);
        static void Deserialize(const YAML::Node& in, std::string& value, const std::string& defaultValue = {});
    };
}