// YamlAssetSerializers.h
#pragma once
#include "Nessie/FileIO/YAML/YamlSerializer.h"
#include "Nessie/Random/UUID.h"

namespace nes
{
    template <>
    struct Serializer<YAML::Emitter, YAML::Node, UUID>
    {
        static void Serialize(YAML::Emitter& out, const UUID& value)
        {
            out << value.GetValue();
        }

        static void Deserialize(const YAML::Node& node, UUID& value, const UUID& defaultValue = {})
        {
            value = node.as<uint64>(defaultValue.GetValue());
        }
    };
}