// YamlCore.h
#pragma once
#include <filesystem>
#include <yaml-cpp/yaml.h>
#include "Nessie/Core/Config.h"
#include "Nessie/FileIO/Serialization.h"

namespace nes
{
    template <typename Type>
    concept HasNativeYamlSerializer_Serialize = requires(Type type, YAML::Emitter& out)
    {
        { Serializer<YAML::Emitter, YAML::Node, Type>::Serialize(out, type) } -> std::same_as<void>; 
    };

    template <typename Type>
    concept HasNativeYamlSerializer_Deserialize = requires(Type type, const YAML::Node& in)
    {
        { Serializer<YAML::Emitter, YAML::Node, Type>::Deserialize(in, type, Type()) } -> std::same_as<void>;   
    };
    
    template <typename Type>
    concept HasNativeYamlSerializer = HasNativeYamlSerializer_Serialize<Type> && HasNativeYamlSerializer_Deserialize<Type>; 
    
    template <typename Type>
    concept IsYamlNativeReadable = HasNativeYamlSerializer || requires (Type type, YAML::Node node)
    {
        { node.as<Type>() } -> std::same_as<Type>;
    };
    
    template <typename Type>
    concept YamlDirectWriteableType = requires (Type type, YAML::Emitter emitter)
    {
        { emitter << type };
    };
    
    template <typename Type>
    concept YamlIndirectWriteableType = requires (Type type)
    {
        { YAML::convert<Type>::encode(type) } -> std::same_as<YAML::Node>;  
    };
    
    template <typename Type>
    concept IsYamlNativeWriteable = HasNativeYamlSerializer || YamlDirectWriteableType<Type> || YamlIndirectWriteableType<Type>;
    
    class YamlNode
    {
    public:
        class Iterator
        {
        public:
            using iterator_category = std::forward_iterator_tag;
            using value_type = YamlNode;
            using reference = YamlNode;
            using pointer = void;
            using difference_type = std::ptrdiff_t;

            explicit    Iterator(YAML::Node::const_iterator it) : m_it(std::move(it)) {}

            YamlNode    Key() const                             { return YamlNode(m_it->first); }
            YamlNode    Value() const                           { return YamlNode(m_it->second); }

            YamlNode    operator*() const                       { return YamlNode(*m_it); }
            Iterator&   operator++()                            { ++m_it; return *this; }
            bool        operator==(const Iterator& other) const { return m_it == other.m_it; }
            bool        operator!=(const Iterator& other) const { return m_it != other.m_it; }

        private:
            YAML::Node::const_iterator m_it{};
        };
        
    public:
        explicit    YamlNode(YAML::Node node) : m_node(std::move(node)) {}
        
        //----------------------------------------------------------------------------------------------------
        /// @brief : Try to read the node's data to the given type. If it fails, or is not present, the
        /// default value will be set.
        //----------------------------------------------------------------------------------------------------
        template <typename Type>
        void        Read(Type& value, const Type& defaultValue = {}) const;

        YamlNode    operator[](const std::string& key) const   { return YamlNode(m_node[key]); }
        YamlNode    operator[](const size_t index) const       { return YamlNode(m_node[index]); }
        explicit    operator bool() const                      { return IsValid(); }
        bool        operator!() const                          { return !IsValid(); }

        bool        IsValid() const                            { return m_node.IsDefined(); }
        bool        IsNull() const                             { return m_node.IsNull(); }
        bool        IsArray() const                            { return m_node.IsSequence(); }
        bool        IsMap() const                              { return m_node.IsMap(); }

        Iterator    begin() const                               { return Iterator(m_node.begin()); }
        Iterator    end() const                                 { return Iterator(m_node.end()); }
        size_t      size() const                                { return m_node.size(); }
        
    private:
        YAML::Node  m_node{};
    };

    // [TODO]: Rename to YamlFile | YamlIStream
    class YamlInStream
    {
    public:
        YamlInStream(const std::filesystem::path& path);
        YamlInStream(const YamlInStream&) = default;
        YamlInStream(YamlInStream&&) noexcept = default;
        YamlInStream& operator=(const YamlInStream&) = default;
        YamlInStream& operator=(YamlInStream&&) noexcept = default;
        ~YamlInStream() = default;
        
        bool                    IsOpen() const;
        YamlNode                GetRoot() const { return YamlNode(m_root); }
    
    private:
        std::filesystem::path   m_path{};
        YAML::Node              m_root{};
    };
    
    class YamlOutStream
    {
    public:
        explicit YamlOutStream(const std::filesystem::path& path, std::ostream& stream);
        ~YamlOutStream();

        bool IsOpen() const;

        template <typename Type>
        void Write(const Type& value);
        
        template <typename Type>
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

    template <typename Type>
    concept HasYamlSerializer = requires (Type type, YamlOutStream& out, const YamlNode& in)
    {
        { Serializer<YamlOutStream, YamlNode, Type>::Serialize(out, type) } -> std::same_as<void>;   
        { Serializer<YamlOutStream, YamlNode, Type>::Deserialize(in, type) } -> std::same_as<void>;   
    };

    template <typename Type>
    void YamlNode::Read(Type& value, const Type& defaultValue) const
    {
        if constexpr (HasYamlSerializer<Type>)
        {
            Serializer<YamlOutStream, YamlNode, Type>::Deserialize(*this, value, defaultValue);            
        }
        else if constexpr (HasNativeYamlSerializer_Deserialize<Type>)
        {
            Serializer<YAML::Emitter, YAML::Node, Type>::Deserialize(m_node, value, defaultValue);
        }
        else if constexpr (IsYamlNativeReadable<Type>)
        {
            value = m_node.as<Type>(defaultValue);
        }
        else
        {
            static_assert(false, "Cannot read type from YamlNode! No valid Yaml Serializer for Type!");
        }
    }

    template <typename Type>
    void YamlOutStream::Write(const Type& value)
    {
        if constexpr (HasYamlSerializer<Type>)
        {
            Serializer<YamlOutStream, YamlNode, Type>::Serialize(*this, value);            
        }
        else if constexpr (HasNativeYamlSerializer_Serialize<Type>)
        {
            Serializer<YAML::Emitter, YAML::Node, Type>::Serialize(m_emitter, value);
        }
        else if constexpr (YamlDirectWriteableType<Type>)
        {
            m_emitter << YAML::Value << value;
        }
        else if constexpr (YamlIndirectWriteableType<Type>)
        {
            m_emitter << YAML::convert<Type>::encode(value);
        }
        else
        {
            static_assert(false, "Cannot write type to YamlOutStream! No valid Yaml Serializer for Type!");
        }
    }
}