#pragma once
// Serializer.h
#include <yaml-cpp/yaml.h>
#include "Debug/Assert.h"
#include "Core/Generic/Concepts.h"

namespace nes
{
    //----------------------------------------------------------------------------------------------------
    //		NOTES:
    //		
    ///		@brief : Specialize this struct to enable Serialization for a Type. You must implement
    ///              Serialize_Impl and Deserialize_Impl for the Type.
    ///              - You can check to see if you have implemented the functions by static asserting the SerializableType concept
    ///		@tparam Type : Type that you are enabling Serialization for.
    //----------------------------------------------------------------------------------------------------
    template <typename Type>
    struct Serializer
    {
        static bool Serialize_Impl([[maybe_unused]] const Type& type, [[maybe_unused]] YAML::Node& node) { return true; }
        static bool Deserialize_Impl([[maybe_unused]] Type* pType, [[maybe_unused]] const YAML::Node& node) { return true; }
    };

    template <typename Type>
    concept SerializableType = requires(Type type, YAML::Node node)
    {
        { Serializer<Type>::Serialize_Impl(type, node) } -> std::same_as<bool>;
        { Serializer<Type>::Deserialize_Impl(&type, node) } -> std::same_as<bool>;
    };
}
