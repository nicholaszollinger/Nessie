// Version.cpp

#include "Version.h"

namespace nes
{
    static constexpr uint32_t kVariantOffset = 29U;
    static constexpr uint32_t kMajorOffset   = 22U;
    static constexpr uint32_t kMinorOffset   = 12U;
    static constexpr uint32_t kPatchOffset   = 0U;

    //----------------------------------------------------------------------------------------------------
    //		NOTES:
    //      This is taken from Vulkan's MAKE_VERSION macro, so that it is compatible.
    //
    ///		@brief : Creates a version number from the given parameters.
    //----------------------------------------------------------------------------------------------------
    static constexpr uint32_t MakeVersion(const uint32_t variant, const uint32_t major, const uint32_t minor, const uint32_t patch)
    {
        return (static_cast<uint32_t>(variant) << kVariantOffset)
            | (static_cast<uint32_t>(major) << kMajorOffset)
            | (static_cast<uint32_t>(minor) << kMinorOffset)
            | static_cast<uint32_t>(patch) << kPatchOffset;
    }


    Version::Version(uint32_t major, uint32_t minor, uint32_t patch, const uint32_t variant)
        : m_value(MakeVersion(variant, major, minor, patch))
    {
        //
    }

    std::string Version::ToString() const
    {
        std::string versionString = 
            std::to_string((m_value >> kMajorOffset) & 0x3FF) + "." + // Major
            std::to_string((m_value >> kMinorOffset) & 0x3FF) + "." + // Minor
            std::to_string((m_value >> kPatchOffset) & 0xFFF);        // Patch   

        return versionString;
    }

    bool Version::Serialize(YAML::Node& node) const
    {
        node.push_back((m_value >> kMajorOffset) & 0x3FF);
            node.push_back((m_value >> kMinorOffset) & 0x3FF);
            node.push_back((m_value >> kPatchOffset) & 0xFFF);
        return true;
    }

    bool Version::Deserialize(const YAML::Node& node)
    {
        if (!node.IsSequence() || node.size() != 3)
        {
            return false;
        }

        m_value = MakeVersion(0, node[0].as<uint32_t>(), node[1].as<uint32_t>(), node[2].as<uint32_t>());
        return true;
    }
}