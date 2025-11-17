// YamlSerializers.cpp
#include "YamlSerializer.h"

namespace nes
{
    void Serializer<YAML::Emitter, YAML::Node, std::string>::Serialize(YAML::Emitter& out, const std::string& value)
    {
        out << YAML::Value << YAML::DoubleQuoted << value;
    }

    void Serializer<YAML::Emitter, YAML::Node, std::string>::Deserialize(const YAML::Node& in, std::string& value, const std::string& defaultValue)
    {
        value = in.as<std::string>(defaultValue);
    }
}
