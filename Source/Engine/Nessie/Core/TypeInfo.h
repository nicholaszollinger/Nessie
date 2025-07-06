// TypeInfo.h
#pragma once
#include "Concepts.h"
#include "Hash.h"

//----------------------------------------------------------------------------------------------------
///	@brief : Defines a TypeID based on the Type's name. This should be the first thing in the body
///     of a class or struct. This macro assumes that a base class has defined the public virtual functions:
///         - TypeID GetTypeID(): Returns the TypeID of the object.
///         - const char* GetTypename(): Returns the name of the object.
///	@param _type : Type (class or struct) to define the ID for.
/// @note : This macro finishes with private access, so for structs, make sure to add public after
///     this macro.
//----------------------------------------------------------------------------------------------------
#define NES_DEFINE_TYPE_INFO(_type)                                                                      \
private:                                                                                                 \
    static_assert(std::is_class_v<_type>, "NES_DEFINE_TYPE_INFO(): _type must be a class or a struct");  \
    static constexpr nes::TypeID                kTypeID = HashString64(#_type);                          \
public:                                                                                                  \
    [[nodiscard]] static constexpr nes::TypeID  GetStaticTypeID()               { return kTypeID; }      \
    [[nodiscard]] virtual nes::TypeID           GetTypeID() const override      { return kTypeID; }      \
    [[nodiscard]] static const char*            GetStaticTypename()             { return #_type; }       \
    [[nodiscard]] virtual const char*           GetTypename() const override    { return #_type; }       \
private:

#define NES_DEFINE_TYPE_INFO_NON_VIRTUAL(_type)                                                          \
private:                                                                                                 \
    static_assert(std::is_class_v<_type>, "NES_DEFINE_TYPE_INFO_NON_VIRTUAL(): _type must be a class or a struct");  \
    static constexpr nes::TypeID                kTypeID = HashString64(#_type);                          \
public:                                                                                                  \
    [[nodiscard]] static constexpr nes::TypeID  GetStaticTypeID()               { return kTypeID; }      \
    [[nodiscard]] nes::TypeID                   GetTypeID() const               { return kTypeID; }      \
    [[nodiscard]] static const char*            GetStaticTypename()             { return #_type; }       \
    [[nodiscard]] const char*                   GetTypename() const             { return #_type; }       \
private:

namespace nes
{
    using TypeID = uint64;

    template <typename Type>
    concept HasValidTypeInfo = requires(Type type)
    {
        { Type::GetStaticTypeID() } -> std::same_as<TypeID>;
        { Type::GetStaticTypename() } -> std::same_as<const char*>;
        { type.GetTypeID() } -> std::same_as<TypeID>;
        { type.GetTypename() } -> std::same_as<const char*>;
    };
}