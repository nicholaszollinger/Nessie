// YamlWriter.cpp
#include "YamlWriter.h"

#include "Nessie/Debug/Log.h"

namespace nes
{
    YamlWriter::YamlWriter(const std::filesystem::path& path, std::ostream& stream)
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

    YamlWriter::~YamlWriter()
    {
        // End the document.
        if (m_emitter.good())
            m_emitter << YAML::EndMap;
    }
    
    bool YamlWriter::IsOpen() const
    {
        return !m_path.empty() && m_emitter.good();
    }

    void YamlWriter::SetKey(const char* key)
    {
        m_emitter << YAML::Key << key;
    }

    void YamlWriter::BeginMap(const char* mapName)
    {
        if (mapName != nullptr)
            m_emitter << YAML::Key << mapName;

        m_emitter << YAML::BeginMap;
    }

    void YamlWriter::EndMap()
    {
        m_emitter << YAML::EndMap;
    }

    void YamlWriter::BeginSequence(const char* sequenceName, const bool inlineArray)
    {
        if (sequenceName != nullptr)
            m_emitter << YAML::Key << sequenceName;

        if (inlineArray)
            m_emitter << YAML::Flow;

        m_emitter << YAML::BeginSeq;
    }

    void YamlWriter::EndSequence()
    {
        m_emitter << YAML::EndSeq;
    }
}
