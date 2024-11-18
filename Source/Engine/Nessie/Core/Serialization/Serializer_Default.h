#pragma once
// DefaultSerializer.h

#include "Serializer.h"

//----------------------------------------------------------------------------------------------------
//		NOTES:
//		
///		@brief : Macro to that defines a DefaultSerializer for a Type.
///		@param Type : Type that is directly Serializable to a YAML::Node.
//----------------------------------------------------------------------------------------------------
#define NES_DEFINE_DEFAULT_SERIALIZER(Type) \
template <> struct nes::Serializer<Type> : public nes::Serializer_Default<Type> {}

namespace nes
{
    //----------------------------------------------------------------------------------------------------
    ///		@brief : Default Serializer implementation for types that can be directly serialized and deserialized
    ///              from YAML::Nodes.
    ///		@tparam Type : Type to serialize.
    //----------------------------------------------------------------------------------------------------
    template <typename Type>
    struct Serializer_Default
    {
        static bool Serialize_Impl(const Type& type, YAML::Node& node)
        {
            node = type;
            return true;
        }

        static bool Deserialize_Impl(Type* pValue, const YAML::Node& node)
        {
            NES_ASSERT(pValue);
            *pValue = node.as<Type>();
            return true;
        }
    };

    // Default Specializations
    NES_DEFINE_DEFAULT_SERIALIZER(bool);
    NES_DEFINE_DEFAULT_SERIALIZER(char);
    NES_DEFINE_DEFAULT_SERIALIZER(unsigned char);
    NES_DEFINE_DEFAULT_SERIALIZER(short);
    NES_DEFINE_DEFAULT_SERIALIZER(unsigned short);
    NES_DEFINE_DEFAULT_SERIALIZER(int);
    NES_DEFINE_DEFAULT_SERIALIZER(unsigned int);
    NES_DEFINE_DEFAULT_SERIALIZER(long);
    NES_DEFINE_DEFAULT_SERIALIZER(unsigned long);
    NES_DEFINE_DEFAULT_SERIALIZER(long long);
    NES_DEFINE_DEFAULT_SERIALIZER(unsigned long long);
    NES_DEFINE_DEFAULT_SERIALIZER(float);
    NES_DEFINE_DEFAULT_SERIALIZER(double);
    NES_DEFINE_DEFAULT_SERIALIZER(std::string);
    
    // [TODO]: Add more specializations for Containers
    template <typename ElementType> struct Serializer<std::vector<ElementType>> : Serializer_Default<std::vector<ElementType>> {};
}