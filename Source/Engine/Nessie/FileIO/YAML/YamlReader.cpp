// YamlReader.cpp
#include "YamlReader.h"
#include "Nessie/Debug/Log.h"

namespace nes
{
    YamlReader::YamlReader(const std::filesystem::path& path)
        : m_path(path)
    {
        m_root = YAML::LoadFile(path.string());
        if (!m_root)
        {
            NES_ERROR("Failed to load YAML file at path: {}", path.string());
        }
    }

    bool YamlReader::IsOpen() const
    {
        return m_root.IsDefined() && !m_path.empty();
    }
}
