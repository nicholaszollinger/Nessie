// YAMLReader.h
#pragma once
#include "YamlCore.h"

namespace nes
{
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
        /// @brief : Reads the node's data to the given type. Throws an exception if invalid.
        //----------------------------------------------------------------------------------------------------
        template <YamlReadableType Type>
        void        Read(Type& value) const                     { value = m_node.as<Type>(); }

        //----------------------------------------------------------------------------------------------------
        /// @brief : Try to read the node's data to the given type. If it fails, the default value will be set.
        //----------------------------------------------------------------------------------------------------
        template <YamlReadableType Type>
        void        Read(Type& value, const Type& defaultValue) const { value = m_node.as<Type>(defaultValue); }

        YamlNode    operator[](const std::string& key) const   { return YamlNode(m_node[key]); }
        YamlNode    operator[](const char* key) const          { return YamlNode(m_node[key]); }
        YamlNode    operator[](const size_t index) const       { return YamlNode(m_node[index]); }
        explicit    operator bool() const                      { return IsValid(); }
        bool        operator!() const                          { return !IsValid(); }

        bool        IsValid() const                            { return m_node.IsDefined(); }
        bool        IsNull() const                             { return m_node.IsNull(); }
        bool        IsArray() const                            { return m_node.IsSequence(); }
        bool        IsMap() const                              { return m_node.IsMap(); }

        Iterator    begin() const                               { return Iterator(m_node.begin()); }
        Iterator    end() const                                 { return Iterator(m_node.end()); }
        
    private:
        YAML::Node  m_node{};
    };
    
    class YamlReader
    {
    public:
        YamlReader(const std::filesystem::path& path);
        YamlReader(const YamlReader&) = default;
        YamlReader(YamlReader&&) noexcept = default;
        YamlReader& operator=(const YamlReader&) = default;
        YamlReader& operator=(YamlReader&&) noexcept = default;
        ~YamlReader() = default;
        
        bool                    IsOpen() const;
        YamlNode                GetRoot() const { return YamlNode(m_root); }
    
    private:
        std::filesystem::path   m_path{};
        YAML::Node              m_root{};
    };
}