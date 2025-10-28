// YamlConversions.h
#pragma once
#include "YamlCore.h"

#define NES_YAML_DEFINE_ENUM_CONVERTER(enumType)                                        \
    template <>                                                                         \
    struct YAML::convert<enumType>                                                      \
    {                                                                                   \
        static Node encode(const enumType& value)                                       \
        {                                                                               \
            Node node;                                                                  \
            node = static_cast<std::underlying_type_t<enumType>>(value);                \
            return node;                                                                \
        }                                                                               \
                                                                                        \
        static bool decode(const Node& node, enumType& value)                           \
        {                                                                               \
            if (!node.IsScalar())                                                       \
                return false;                                                           \
                                                                                        \
            value = static_cast<enumType>(node.as<std::underlying_type_t<enumType>>()); \
            return true;                                                                \
        }                                                                               \
    }
