#pragma once
// Serializer_Enum.h

#include "Serializer.h"

//----------------------------------------------------------------------------------------------------
//		NOTES:
//		
///		@brief : Macro to that defines how to Serialize and Deserialize an Enum.
///		@param EnumType : Enum Type to serialize.
//----------------------------------------------------------------------------------------------------
#define NES_DEFINE_ENUM_SERIALIZER(EnumType) \
template <> struct nes::Serializer<EnumType> : public nes::Serializer_Enum<EnumType> {}

namespace nes
{
    //----------------------------------------------------------------------------------------------------
    //		NOTES:
    //		
    ///		@brief : Serialize implementation for Enum types. You should use the
    ///             NES_DEFINE_ENUM_SERIALIZER macro to specialize this struct for your Enum type.
    ///		@tparam Type : Enum Type to serialize.
    //----------------------------------------------------------------------------------------------------
    template <EnumType Type>
    struct Serializer_Enum
    {
        using UnderlyingType = std::underlying_type_t<Type>;

        static bool Serialize_Impl(const Type& type, YAML::Node& node)
        {
            node = static_cast<UnderlyingType>(type);
            return true;
        }

        static bool Deserialize_Impl(Type* pValue, const YAML::Node& node)
        {
            NES_ASSERT(pValue);
            *pValue = static_cast<Type>(node.as<UnderlyingType>());
            return true;
        }
    };
}