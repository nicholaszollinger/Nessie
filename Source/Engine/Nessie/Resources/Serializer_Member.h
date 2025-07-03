// Serializer_Member.h
#pragma once
#include "Serializer.h"

//----------------------------------------------------------------------------------------------------
//		NOTES:
//		
///		@brief : Macro to define how to Serialize and Deserialize a Type that has both
///              Serialize() and Deserialize member functions. If it doesn't have the proper functions
///              implemented, this will fail the HasMemberSerializeFunctions concept.
///		@param Type : Struct or Class Type to serialize.
//----------------------------------------------------------------------------------------------------
#define NES_DEFINE_MEMBER_SERIALIZER(Type) \
template <> struct nes::Serializer<Type> : public nes::Serializer_Member<Type> {}

namespace nes
{
    template <typename Type>
    concept HasMemberSerializeFunctions = requires (Type type, YAML::Node node)
    {
        { type.Serialize(node) } -> std::same_as<bool>;
        { type.Deserialize(node) } -> std::same_as<bool>;
    };

    //----------------------------------------------------------------------------------------------------
    //		NOTES:
    //		
    ///		@brief : Serializer implementation for types that have both Serialize() and Deserialize() member function.
    ///             You should use NES_DEFINE_MEMBER_SERIALIZER to set up the serializer for your Type.
    ///		@tparam Type : Type that must have both Serialize() and Deserialize() member functions.
    //----------------------------------------------------------------------------------------------------
    template <HasMemberSerializeFunctions Type>
    struct Serializer_Member
    {
        static bool Serialize_Impl(const Type& type, YAML::Node& node)
        {
            return type.Serialize(node);
        }

        static bool Deserialize_Impl(Type* pValue, const YAML::Node& node)
        {
            NES_ASSERT(pValue);
            return pValue->Deserialize(node);
        }
    };
}