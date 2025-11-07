// YamlMathSerializers.h
#pragma once
#include "Nessie/Math/Math.h"
#include "Nessie/Core/Color.h"
#include "Nessie/FileIO/YAML/YamlSerializer.h"

//============================================================================================================================================================================================
#pragma region [ Vectors, Quat, Rotation ]
//============================================================================================================================================================================================

#define DEFINE_YAML_2COMPONENT(type, elementType) \
template <>\
struct nes::Serializer<YAML::Emitter, YAML::Node, type>\
{\
    static void Serialize(YAML::Emitter& out, const type& v)\
    {\
        out << YAML::Flow << YAML::BeginSeq << v.x << v.y << YAML::EndSeq;\
    }\
    \
    static void Deserialize(const YAML::Node& in, type& v, const type& defaultValue = {})\
    {\
        if (!in.IsSequence() || in.size() != 2)\
        {\
            NES_ERROR("Failed to deserialize YamlNode for type: {}! Using default value...", #type);\
            v = defaultValue;\
            return;\
        }\
        \
        v.x = in[0].as<elementType>(defaultValue.x);\
        v.y = in[1].as<elementType>(defaultValue.y);\
    }\
}

DEFINE_YAML_2COMPONENT(nes::Vec2, float);
DEFINE_YAML_2COMPONENT(nes::IVec2, int);
DEFINE_YAML_2COMPONENT(nes::UVec2, uint32);
DEFINE_YAML_2COMPONENT(nes::Float2, float);
DEFINE_YAML_2COMPONENT(nes::Int2, int);
DEFINE_YAML_2COMPONENT(nes::UInt2, uint32);

#undef DEFINE_YAML_2COMPONENT

#define DEFINE_YAML_3COMPONENT(type, elementType) \
template <>\
struct nes::Serializer<YAML::Emitter, YAML::Node, type>\
{\
    static void Serialize(YAML::Emitter& out, const type& v)\
    {\
        out << YAML::Flow << YAML::BeginSeq << v.x << v.y << v.z << YAML::EndSeq;\
    }\
    \
    static void Deserialize(const YAML::Node& in, type& v, const type& defaultValue = {})\
    {\
        if (!in.IsSequence() || in.size() != 3)\
        {\
            NES_ERROR("Failed to deserialize YamlNode for type: {}! Using default value...", #type);\
            v = defaultValue;\
            return;\
        }\
        \
        v.x = in[0].as<elementType>(defaultValue.x);\
        v.y = in[1].as<elementType>(defaultValue.y);\
        v.z = in[2].as<elementType>(defaultValue.z);\
    }\
}

DEFINE_YAML_3COMPONENT(nes::Vec3, float);
DEFINE_YAML_3COMPONENT(nes::IVec3, int);
DEFINE_YAML_3COMPONENT(nes::UVec3, uint32);
DEFINE_YAML_3COMPONENT(nes::Float3, float);
DEFINE_YAML_3COMPONENT(nes::Int3, int);
DEFINE_YAML_3COMPONENT(nes::UInt3, uint32);

#undef DEFINE_YAML_3COMPONENT

#define DEFINE_YAML_4COMPONENT(type, elementType) \
template <>\
struct nes::Serializer<YAML::Emitter, YAML::Node, type>\
{\
    static void Serialize(YAML::Emitter& out, const type& v)\
    {\
        out << YAML::Flow << YAML::BeginSeq << v.x << v.y << v.z << v.w << YAML::EndSeq;\
    }\
    \
    static void Deserialize(const YAML::Node& in, type& v, const type& defaultValue = {})\
    {\
        if (!in.IsSequence() || in.size() != 4)\
        {\
            NES_ERROR("Failed to deserialize YamlNode for type: {}! Using default value...", #type);\
            v = defaultValue;\
            return;\
        }\
        \
        v.x = in[0].as<elementType>(defaultValue.x);\
        v.y = in[1].as<elementType>(defaultValue.y);\
        v.z = in[2].as<elementType>(defaultValue.z);\
        v.z = in[3].as<elementType>(defaultValue.w);\
    }\
}

DEFINE_YAML_4COMPONENT(nes::Vec4, float);
DEFINE_YAML_4COMPONENT(nes::IVec4, int);
DEFINE_YAML_4COMPONENT(nes::UVec4, uint32);
DEFINE_YAML_4COMPONENT(nes::Float4, float);
DEFINE_YAML_4COMPONENT(nes::Int4, int);
DEFINE_YAML_4COMPONENT(nes::UInt4, uint32);

#undef DEFINE_YAML_4COMPONENT

namespace nes
{
    template <>
    struct Serializer<YAML::Emitter, YAML::Node, Rotation>
    {
        static void Serialize(YAML::Emitter& out, const Rotation& rot)
        {
            out << YAML::Flow << YAML::BeginSeq << rot.m_pitch << rot.m_yaw << rot.m_roll << YAML::EndSeq;
        }
       
        static void Deserialize(const YAML::Node& in, Rotation& rot, const Rotation& defaultValue = {})
        {
            if (!in.IsSequence() || in.size() != 3)
            {
                NES_ERROR("Failed to deserialize YamlNode for type: nes::Rotation! Using default value...");
                rot = defaultValue;
                return;
            }

            rot.m_pitch = in[0].as<float>(defaultValue.m_pitch);
            rot.m_yaw = in[1].as<float>(defaultValue.m_yaw);
            rot.m_roll = in[2].as<float>(defaultValue.m_roll);
        }
    };

    template <>
    struct Serializer<YAML::Emitter, YAML::Node, Quat>
    {
        static void Serialize(YAML::Emitter& out, const Quat& rot)
        {
            Serializer<YAML::Emitter, YAML::Node, Vec4>::Serialize(out, rot.m_value);
        }
       
        static void Deserialize(const YAML::Node& in, Quat& rot, const Quat& defaultValue = {})
        {
            Serializer<YAML::Emitter, YAML::Node, Vec4>::Deserialize(in, rot.m_value, defaultValue.m_value);
        }
    };
}

#pragma endregion
//============================================================================================================================================================================================

//============================================================================================================================================================================================
#pragma region [ Colors ]
//============================================================================================================================================================================================

namespace nes
{
    template <>
    struct Serializer<YAML::Emitter, YAML::Node, LinearColor>
    {
        static void Serialize(YAML::Emitter& out, const LinearColor& color)
        {
            out << YAML::Flow << YAML::BeginSeq << color.r << color.g << color.b << color.a << YAML::EndSeq;
        }
        
        static void Deserialize(const YAML::Node& in, LinearColor& v, const LinearColor& defaultValue)
        {
            if (!in.IsSequence() || in.size() != 4)
            {
                NES_ERROR("Failed to deserialize YamlNode for type: nes::LinearColor! Using default value...");
                v = defaultValue;
                return;
            }
            
            v.r = in[0].as<float>(defaultValue.r);
            v.g = in[1].as<float>(defaultValue.g);
            v.b = in[2].as<float>(defaultValue.b);
            v.a = in[3].as<float>(defaultValue.a);
        }
    };

    template <>
    struct Serializer<YAML::Emitter, YAML::Node, Color>
    {
        static void Serialize(YAML::Emitter& out, const Color& color)
        {
            out << YAML::Flow << YAML::BeginSeq << color.r << color.g << color.b << color.a << YAML::EndSeq;
        }
        
        static void Deserialize(const YAML::Node& in, Color& v, const Color& defaultValue)
        {
            if (!in.IsSequence() || in.size() != 4)
            {
                NES_ERROR("Failed to deserialize YamlNode for type: nes::Color! Using default value...");
                v = defaultValue;
                return;
            }

            v.r = in[0].as<uint8>(defaultValue.r);
            v.g = in[1].as<uint8>(defaultValue.g);
            v.b = in[2].as<uint8>(defaultValue.b);
            v.a = in[3].as<uint8>(defaultValue.a);
        }
    };
}

#pragma endregion
//============================================================================================================================================================================================