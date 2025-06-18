// Version.h
#pragma once
#include "Core/Config.h"
#include <string>
#include "Resources/Serializer_Member.h"

namespace nes
{
    //----------------------------------------------------------------------------------------------------
    /// @brief : Struct that represents a version number for the Application, Engine, etc.
    //----------------------------------------------------------------------------------------------------
    struct Version
    {
    public:
        Version() = default;
        Version(uint32 major, uint32 minor = 0, uint32 patch = 0, const uint32 variant = 0);

        bool            operator==(const Version& other) const  { return m_value == other.m_value; }
        bool            operator!=(const Version& other) const  { return m_value != other.m_value; }
                        operator uint32() const                 { return m_value; }

        //----------------------------------------------------------------------------------------------------
        /// @brief : Get the Major value of the version. 
        //----------------------------------------------------------------------------------------------------
        uint32          Major() const;

        //----------------------------------------------------------------------------------------------------
        /// @brief : Get the Minor value of the version.
        //----------------------------------------------------------------------------------------------------
        uint32          Minor() const;

        //----------------------------------------------------------------------------------------------------
        /// @brief : Get the Patch value of the version 
        //----------------------------------------------------------------------------------------------------
        uint32          Patch() const;

        //----------------------------------------------------------------------------------------------------
        /// @brief : Get the string representation of the version value. 
        //----------------------------------------------------------------------------------------------------
        std::string     ToString() const;

        //----------------------------------------------------------------------------------------------------
        /// @brief : Deserialize a Version from YAML.
        //----------------------------------------------------------------------------------------------------
        bool            Deserialize(const YAML::Node& node);

        //----------------------------------------------------------------------------------------------------
        /// @brief : Serialize a Version to YAML
        //----------------------------------------------------------------------------------------------------
        bool            Serialize(YAML::Node& node) const;

    private:
        /// Combined value containing Major, Minor and Patch values.
        uint32          m_value{};
    };

    NES_DEFINE_MEMBER_SERIALIZER(Version);
}