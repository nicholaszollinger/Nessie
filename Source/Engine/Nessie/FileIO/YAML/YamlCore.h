// YamlCore.h
#pragma once
#include <filesystem>
#include <yaml-cpp/yaml.h>
#include "Nessie/FileIO/Serialization.h"

#define NES_YAML_DEFINE_ENUM_CONVERTER(enumType)                                        \
    template <>                                                                         \
    struct YAML::convert<enumType>                                                      \
    {                                                                                   \
        static Node encode(const enumType& value)                                       \
        {                                                                               \
            Node node;                                                                  \
            if constexpr (std::same_as<std::underlying_type_t<enumType>, uint8_t>)      \
                node = static_cast<int>(static_cast<std::underlying_type_t<enumType>>(value)); \
            else                                                                        \
                node = static_cast<std::underlying_type_t<enumType>>(value);            \
            return node;                                                                \
        }                                                                               \
                                                                                        \
        static bool decode(const Node& node, enumType& value)                           \
        {                                                                               \
            if (!node.IsScalar())                                                       \
            return false;                                                               \
                                                                                        \
            value = static_cast<enumType>(node.as<std::underlying_type_t<enumType>>()); \
            return true;                                                                \
        }                                                                               \
    }                                                                                  

namespace nes
{
    template <typename Type>
    concept YamlReadableType = requires (Type type, YAML::Node node)
    {
        { node.as<Type>() } -> std::same_as<Type>;
    };
    
    template <typename Type>
    concept YamlDirectWriteableType = requires (Type type, YAML::Emitter emitter)
    {
        { emitter << type };
    };

    template <typename Type>
    concept YamlIndirectWriteableType = requires (Type type)
    {
        { YAML::convert<Type>::encode(type) } -> std::same_as<YAML::Node>;  
    };
    
    template <typename Type>
    concept YamlWriteableType = YamlDirectWriteableType<Type> || YamlIndirectWriteableType<Type>; 
}

#include "YamlMathConversions.inl"