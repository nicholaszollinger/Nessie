// Version.cpp
#include "Version.h"

namespace nes
{
    static constexpr uint32 kVariantOffset = 29U;
    static constexpr uint32 kMajorOffset   = 22U;
    static constexpr uint32 kMinorOffset   = 12U;
    static constexpr uint32 kPatchOffset   = 0U;

    //----------------------------------------------------------------------------------------------------
    // This is taken from Vulkan's MAKE_VERSION macro so that it is compatible.
    // 
    ///	@brief : Creates a version number from the given parameters.
    //----------------------------------------------------------------------------------------------------
    static constexpr uint32 MakeVersion(const uint32 variant, const uint32 major, const uint32 minor, const uint32 patch)
    {
        return (static_cast<uint32>(variant) << kVariantOffset)
            | (static_cast<uint32>(major) << kMajorOffset)
            | (static_cast<uint32>(minor) << kMinorOffset)
            | static_cast<uint32>(patch) << kPatchOffset;
    }


    Version::Version(const uint32 major, const uint32 minor, const uint32 patch, const uint32 variant)
        : m_value(MakeVersion(variant, major, minor, patch))
    {
        //
    }

    uint32 Version::Major() const
    {
        return (m_value >> kMajorOffset) & 0x3FF;
    }

    uint32 Version::Minor() const
    {
        return (m_value >> kMinorOffset) & 0x3FF;
    }

    uint32 Version::Patch() const
    {
        return (m_value >> kPatchOffset) & 0xFFF;
    }

    std::string Version::ToString() const
    {
        std::string versionString = 
            std::to_string(Major()) + "." + 
            std::to_string(Minor()) + "." +
            std::to_string(Patch());           

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