// YamlWriter.h
#pragma once
#include "YamlCore.h"

namespace nes
{
    class YamlWriter
    {
    public:
        explicit YamlWriter(const std::filesystem::path& path, std::ostream& stream);
        ~YamlWriter();

        bool IsOpen() const;

        template <YamlWriteableType Type>
        void Write(const Type& value)
        {
            if constexpr (YamlDirectWriteableType<Type>)
            {
                m_emitter << YAML::Value << value;
            }
            else
            {
                m_emitter << YAML::convert<Type>::encode(value);
            }
        }

        template <YamlWriteableType Type>
        void Write(const char* key, const Type& value)
        {
            SetKey(key);
            Write(value);
        }

        void SetKey(const char* key);
        
        void BeginMap(const char* mapName = nullptr);
        void EndMap();

        void BeginSequence(const char* sequenceName = nullptr, const bool inlineArray = false);
        void EndSequence();

    private:
        std::filesystem::path   m_path;
        YAML::Emitter           m_emitter;
    };

    template <>
    inline void YamlWriter::Write<std::string>(const std::string& value)
    {
        m_emitter << YAML::Value << YAML::DoubleQuoted << value;
    }
    
    template <>
    inline void YamlWriter::Write<const char*>(const char* const& value)
    {
        m_emitter << YAML::Value << YAML::DoubleQuoted << value;
    }
}