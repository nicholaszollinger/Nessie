// FormatString.h
#pragma once
#include <chrono>
#include <string>
#include "fmt/format.h"

//----------------------------------------------------------------------------------------------------
/// @brief : Define a formatter to create a string version of the type. Following the format string parameter,
///     you need to supply the type's member variables that you are using for arguments in the string.
///     For each argument, use 'val.m_memberVar'.
///
///     Example usage:
///     <code>
///     NES_DEFINE_STRING_FORMATTER(nes::Vector4, "(x={:.3f}, y={:.3f}, z={:.3f}, w={:.3f})", val.x, val.y, val.z, val.w);
///     </code>
///
///	@param type : Type (including namespace) you are creating a formatter for. Ex: nes::Vec4  
///	@param formatStr : Format string to use for the type.
///     Example for nes::Vec4:
///     "(x={:.3f}, y={:.3f}, z={:.3f}, w={:.3f})"
//----------------------------------------------------------------------------------------------------
#define NES_DEFINE_STRING_FORMATTER(type, formatStr, ...)                       \
template <>                                                                     \
struct fmt::formatter<type> : fmt::formatter<std::string>                       \
{                                                                               \
    auto format(type val, format_context& ctx) const -> decltype(ctx.out())     \
    {                                                                           \
        return fmt::format_to(ctx.out(), formatStr, __VA_ARGS__);               \
    }                                                                           \
}

//----------------------------------------------------------------------------------------------------
/// @brief : Begin a format block for an enum. Combined with NES_FORMAT_ENUM_CASE() and NES_END_ENUM_FORMATTER(),
///     this defines a class that will format the enum's values so that it prints out the value name
///     in a format string. ETest::True -> "True".
///
/// Example:
///
/// <code>
/// enum class ETest
/// {
///     False,
///     True,
///     Maybe
/// };
///
/// NES_BEGIN_ENUM_FORMATTER(ETest)
///     NES_FORMAT_ENUM_CASE(False)
///     NES_FORMAT_ENUM_CASE(True)
/// NES_END_ENUM_FORMATTER()
/// </code>
///
/// - nes::ToString(ETest::True) -> "True"
/// - nes::ToString(ETest::False) -> "False"
/// - nes::ToString(ETest::Maybe) -> "Unknown" This is the default case when not specified.
///
///	@param enumType : Enum Type, ex: 
//----------------------------------------------------------------------------------------------------
#define NES_BEGIN_ENUM_FORMATTER(enumType)                                                  \
    template <>                                                                             \
    struct fmt::formatter<enumType> : fmt::formatter<std::string>                           \
    {                                                                                       \
        using Type = enumType;                                                              \
        auto format(enumType value, format_context& ctx) const -> format_context::iterator  \
        {                                                                                   \
            std::string_view name = "Unknown";                                              \
            switch (value)                                                                  \
            { 

//----------------------------------------------------------------------------------------------------
/// @brief : Bookend macro for NES_BEGIN_ENUM_FORMATTER().
///     See NES_BEGIN_ENUM_FORMATTER for example usage.
//----------------------------------------------------------------------------------------------------
#define NES_END_ENUM_FORMATTER()                                                            \
                default: break;                                                             \
            }                                                                               \
                                                                                            \
            return formatter<string_view>::format(name, ctx);                               \
        }                                                                                   \
    };

//----------------------------------------------------------------------------------------------------
/// @brief : Format an individual enum case value.
///     See NES_BEGIN_ENUM_FORMATTER for example usage.
//----------------------------------------------------------------------------------------------------
#define NES_FORMAT_ENUM_CASE(enumValue) case Type::enumValue: name = #enumValue; break;

namespace nes
{
    /// Is valid if NES_DEFINE_STRING_FORMATTER() was set for the type. 
    template <typename Type>
    concept FormattableType = requires(Type type)
    {
        { fmt::formatter<Type>() } -> std::convertible_to<fmt::formatter<std::string>>;
    };

    //----------------------------------------------------------------------------------------------------
    /// @brief : Create a string representation of the type.
    //----------------------------------------------------------------------------------------------------
    template <FormattableType Type>
    std::string ToString(const Type& type)
    {
        fmt::memory_buffer buffer;
        fmt::format_to(std::back_inserter(buffer), "{}", type);
        return fmt::to_string(buffer);
    }
}

namespace nes
{
    inline std::string StripNamespaceFromTypename(const std::string_view fullName)
    {
        const size_t lastColon = fullName.find_last_of(':');
        if (lastColon != std::string_view::npos)
        {
            return std::string(fullName.substr(lastColon + 1));
        }
        return std::string(fullName);
    }

    // [TODO]: This should be the default implementation, and you should get rid of the above.
    // - I just have to update a lot of calls to the former.
    constexpr std::string_view StripNamespaceFromTypename2(const std::string_view fullName)
    {
        auto first = fullName.find_last_of(':');
        auto value = fullName.substr(first + 1);
        return value;
    }
}