// Serialization.h
#pragma once
#include "Nessie/Core/Concepts.h"

namespace nes
{
    template <typename Type, typename WriterType>
    concept WriterCanTriviallyWrite = requires(const Type& object, WriterType& writer)
    {
        { writer.Write(object) } -> std::same_as<void>;
    };

    template <typename Type, typename ReaderType>
    concept ReaderCanTriviallyRead = requires(Type& object, ReaderType& reader)
    {
        { reader.Read(object) } -> std::same_as<void>;
    };
    
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
    
    template <typename Type, typename WriterType>
    concept SerializableTo = HasStaticSerializeMember<Type, WriterType>
        || HasValidSerializeMember<Type, WriterType>
        || WriterCanTriviallyWrite<Type, WriterType>;
    
    template <typename Type, typename ReaderType>
    concept SerializableFrom = HasStaticDeserializeMember<Type, ReaderType>
        || HasValidDeserializeMember<Type, ReaderType>
        || ReaderCanTriviallyRead<Type, ReaderType>;

    //----------------------------------------------------------------------------------------------------
    /// @brief : Serialize an object to a WriterType.
    ///
    /// The Object must have one of the following:
    ///     - Static Serialize Function: static void Type::Serialize(WriterType& writer, const Type& object); 
    ///     - Member Serialize Function: void Type::Serialize(WriterType& writer) const;
    ///     - Can directly call WriterType::Write<Type>(const Type& object).
    ///
    /// Even if the Object can be directly called with the WriterType's Write function, if a specific Serialize
    /// function has been defined, the Serialize function will be called instead. It checks in order of Static,
    /// Member then direct call.
    //----------------------------------------------------------------------------------------------------
    template <typename Type, typename WriterType> requires SerializableTo<Type, WriterType>
    void Serialize(WriterType& writer, const Type& object)
    {
        // Static Serialize Function
        if constexpr (HasStaticSerializeMember<Type, WriterType>)
            Type::Serialize(writer, object);

        // Member Serialize Function.
        if constexpr (HasValidSerializeMember<Type, WriterType>)
            object.Serialize(writer);

        // Trivial to Write.
        else
            writer.template Write<Type>(object);   
    }

    //----------------------------------------------------------------------------------------------------
    /// @brief : Deserialize an object from a ReaderType.
    ///
    /// The Object must have one of the following:
    ///     - Static Deserialize Function: static void Type::Deserialize(ReaderType& reader, Type& outObject); 
    ///     - Member Deserialize Function: void Type::Deserialize(ReaderType& reader);
    ///     - Can directly call ReaderType::Read<Type>(Type& outObject).
    ///
    /// Even if the Object can be directly called with the ReaderType's Read function, if a specific Deserialize
    /// function has been defined, the Deserialize function will be called instead. It checks in order of Static,
    /// Member then direct call.
    //----------------------------------------------------------------------------------------------------
    template <typename Type, typename ReaderType> requires SerializableFrom<Type, ReaderType>
    void Deserialize(ReaderType& reader, Type& object)
    {
        // Static Deserialize Function
        if constexpr (HasStaticDeserializeMember<Type, ReaderType>)
            Type::Deserialize(reader, object);

        // Member Deserialize Function.
        if constexpr (HasValidDeserializeMember<Type, ReaderType>)
            object.Deserialize(reader);

        // Trivial to Read.
        else
            reader.template Read<Type>(object);  
    }
}