// Serialization.h
#pragma once
#include <functional>
#include "Nessie/Core/Concepts.h"

namespace nes
{
    template <typename Type, typename WriterType>
    concept HasValidSerializeMember = requires(const Type& object, WriterType& writer)
    {
        { object.Serialize(writer) } -> std::same_as<void>;
    };

    template <typename Type, typename ReaderType>
    concept HasValidDeserializeMember = requires(Type& object, ReaderType& reader)
    {
        { object.Deserialize(reader) } -> std::same_as<void>;
    };

    template <typename Type, typename WriterType>
    concept HasStaticSerializeMember = requires(const Type& object, WriterType& writer)
    {
        { Type::Serialize(writer, object) } -> std::same_as<void>;
    };

    template <typename Type, typename ReaderType>
    concept HasStaticDeserializeMember = requires(Type& object, ReaderType& reader)
    {
        { Type::Deserialize(reader, object) } -> std::same_as<void>;  
    };
    
    template <typename WriterType, typename ReaderType, typename Type>
    struct Serializer
    {
        // static void Serialize(WriterType& out, const Type& object){}
        // static void Deserialize(const ReaderType& in, Type& object, const Type& defaultValue = {}){}
    };

    template <typename WriterType, typename ReaderType, typename Type>
    concept HasSerializer_Serialize = requires(const Type& object, WriterType& writer)
    {
        { Serializer<WriterType, ReaderType, Type>::Serialize(writer, object) } -> std::same_as<void>;  
    };

    template <typename WriterType, typename ReaderType, typename Type>
    concept HasSerializer_Deserialize = requires(Type& object, ReaderType& reader)
    {
        { Serializer<WriterType, ReaderType, Type>::Deserialize(reader, object, Type()) } -> std::same_as<void>;
    };

    template <typename WriterType, typename ReaderType, typename Type>
    concept HasSerializer = HasSerializer_Serialize<WriterType, ReaderType, Type> && HasSerializer_Deserialize<WriterType, ReaderType, Type>;
    
    template <typename Type, typename WriterType, typename ReaderType>
    concept SerializableTo = HasStaticSerializeMember<Type, WriterType>
        || HasValidSerializeMember<Type, WriterType>
        || HasSerializer<WriterType, ReaderType, Type>;
    
    template <typename Type, typename WriterType, typename ReaderType>
    concept SerializableFrom = HasStaticDeserializeMember<Type, ReaderType>
        || HasValidDeserializeMember<Type, ReaderType>
        || HasSerializer<WriterType, ReaderType, Type>;
}