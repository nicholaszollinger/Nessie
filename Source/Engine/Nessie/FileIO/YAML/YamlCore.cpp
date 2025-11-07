// YamlCore.cpp
#include "YamlCore.h"
#include "Nessie/Debug/Log.h"

namespace nes
{
    YamlInStream::YamlInStream(const std::filesystem::path& path)
        : m_path(path)
    {
        m_root = YAML::LoadFile(path.string());
        if (!m_root)
        {
            NES_ERROR("Failed to load YAML file at path: {}", path.string());
        }
    }

    bool YamlInStream::IsOpen() const
    {
        return !m_path.empty() && m_root.IsDefined();
    }

    YamlOutStream::YamlOutStream(const std::filesystem::path& path, std::ostream& stream)
        : m_path(path)
        , m_emitter(stream)
    {
        if (!m_emitter.good())
        {
            NES_ERROR("Failed to open YamlWriter for file with path: {}\n\t- The stream must be valid!", path.string());
        }
        else
        {
            m_emitter.SetDoublePrecision(4);
            m_emitter.SetFloatPrecision(4);
            m_emitter.SetBoolFormat(YAML::EMITTER_MANIP::TrueFalseBool);
            m_emitter.SetNullFormat(YAML::EMITTER_MANIP::UpperNull);
            
            // Begin the document.
            m_emitter << YAML::BeginMap;
        }
    }

    YamlOutStream::~YamlOutStream()
    {
        // End the document.
        if (m_emitter.good())
            m_emitter << YAML::EndMap;
    }
    
    bool YamlOutStream::IsOpen() const
    {
        return !m_path.empty() && m_emitter.good();
    }

    void YamlOutStream::SetKey(const char* key)
    {
        m_emitter << YAML::Key << key;
    }

    void YamlOutStream::BeginMap(const char* mapName)
    {
        if (mapName != nullptr)
            m_emitter << YAML::Key << mapName;

        m_emitter << YAML::BeginMap;
    }

    void YamlOutStream::EndMap()
    {
        m_emitter << YAML::EndMap;
    }

    void YamlOutStream::BeginSequence(const char* sequenceName, const bool inlineArray)
    {
        if (sequenceName != nullptr)
            m_emitter << YAML::Key << sequenceName;

        if (inlineArray)
            m_emitter << YAML::Flow;

        m_emitter << YAML::BeginSeq;
    }

    void YamlOutStream::EndSequence()
    {
        m_emitter << YAML::EndSeq;
    }
}
