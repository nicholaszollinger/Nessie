#pragma once
// Version.h
#include <cstdint>
#include <string>
#include "Resources/Serializer_Member.h"

namespace nes
{
    //----------------------------------------------------------------------------------------------------
    //		NOTES:
    //		
    ///		@brief : Struct that represents a version number for the Application, Engine, etc.
    //----------------------------------------------------------------------------------------------------
    struct Version
    {
        uint32_t m_value{};

        Version() = default;
        Version(uint32_t major, uint32_t minor = 0, uint32_t patch = 0, const uint32_t variant = 0);

        bool operator==(const Version& other) const { return m_value == other.m_value; }
        bool operator!=(const Version& other) const { return m_value != other.m_value; }

        std::string ToString() const;
        bool Deserialize(const YAML::Node& node);
        bool Serialize(YAML::Node& node) const;
    };

    NES_DEFINE_MEMBER_SERIALIZER(Version);
}